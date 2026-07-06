#include <cmath>
#include <DxLib.h>
#include "../Application.h"
#include "../Utility/AsoUtility.h"
#include "../Object/Common/AnimationController.h"
#include "../Object/Actor/SkyDome.h"
#include "../Manager/InputManager.h"
#include "../Camera/Camera.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/SoundManager.h"
#include "SceneManager.h"
#include "TitleScene.h"

// メニューラベル（MENU_ITEM の順に対応）
static const char* MENU_LABELS[] = {
	"GAME START",
	"TUTORIAL",
	"OPTION",
	"RANKING",
	"QUIT GAME",
};
// 選択中 = 赤、非選択 = 白（参考画像準拠）
static constexpr unsigned int COLOR_SELECT = 0xFF2222;
static constexpr unsigned int COLOR_UNSELECT = 0xFFFFFF;
static constexpr unsigned int COLOR_TITLE = 0xFFFFFF;

TitleScene::TitleScene(void)
	:
	imgPushSpace_(-1),
	SceneBase(),
	skyDome_()
{
}

TitleScene::~TitleScene(void)
{
	skyDome_->Release();
}

void TitleScene::Init(void)
{
	// BGM再生
	SoundManager::GetInstance().PlayBgmTitle();

	// 球体惑星
	spherePlanet_.SetModel(resMng_.Load(			// 1個 = Load()  複数 = Depulicate()
		ResourceManager::SRC::MAIN_STAGE).handleId_);
	spherePlanet_.scl = { 0.7f, 0.7f, 0.7f };
	spherePlanet_.quaRot = Quaternion::Identity();

	spherePlanet_.pos = { 600, -300, 550 };
	spherePlanet_.Update();

	// カメラ
	camera_ = std::make_unique<Camera>();
	// 定点モードに設定
	camera_->ChangeMode(Camera::MODE::FIXED_POINT);

	// スカイドーム
	skyDome_ = std::make_unique<SkyDome>(empty_);
	skyDome_->Init();

}

void TitleScene::Update(void)
{
	camera_->Update();
	skyDome_->Update();

	// 惑星にX軸に毎フレーム1°ずつ回転を追加
	spherePlanet_.quaRotLocal = Quaternion::Mult(
		spherePlanet_.quaRotLocal,
		Quaternion::AngleAxis(AsoUtility::Deg2RadF(-0.1f), AsoUtility::AXIS_Z));

	// モデル行列を更新
	spherePlanet_.Update();

	UpdateInput();
}

void TitleScene::UpdateInput(void)
{
	auto const& ins = InputManager::GetInstance();

	// メニュー選択
	if (ins.IsTriggered(InputManager::InputCommand::UI_UP))
	{
		selectIndex_ = (selectIndex_ - 1 + MENU_COUNT) % MENU_COUNT;
	}
	if (ins.IsTriggered(InputManager::InputCommand::UI_DOWN))
	{
		selectIndex_ = (selectIndex_ + 1) % MENU_COUNT;
	}

	// マウス座標取得
	int mouseX, mouseY;
	GetMousePoint(&mouseX, &mouseY);

	// マウスが動いた時だけホバー判定を行う
	bool mouseMoved = (mouseX != prevMouseX_ || mouseY != prevMouseY_);

	if (mouseMoved)
	{
		int mouseHoverIndex = -1;
		for (int i = 0; i < MENU_COUNT; ++i)
		{
			int itemY = MENU_Y_START + i * MENU_LINE_HEIGHT;
			if (mouseX >= MENU_X && mouseX <= MENU_X + 300 &&
				mouseY >= itemY && mouseY <= itemY + MENU_LINE_HEIGHT)
			{
				mouseHoverIndex = i;
				break;
			}
		}
		selectIndex_ = mouseHoverIndex;
	}

	prevMouseX_ = mouseX;
	prevMouseY_ = mouseY;

	// 決定
	bool decide = ins.IsTriggered(InputManager::InputCommand::UI_DECIDE);

	if (decide && selectIndex_ != -1)
	{
		switch (static_cast<MENU>(selectIndex_))
		{
		case MENU::GAME_START:
			sceMng_.ChangeScene(SceneManager::SCENE_ID::GAME);
			SoundManager::GetInstance().StopBGMTitle();
			break;
		case MENU::QUIT_GAME:
			PostQuitMessage(0);
			break;
		default:
			break;
		}
	}
}

// 描画
void TitleScene::Draw(void)
{
	// 描画前処理の適用
	camera_->SetBeforeDraw();

	// スカイドーム
	skyDome_->Draw();
	// 惑星
	MV1DrawModel(spherePlanet_.modelId);

	DrawTitle();
	DrawMenu();
}

void TitleScene::DrawTitle(void) const
{
	SetFontSize(TITLE_FONT_SIZE);
	DrawFormatString(MENU_X, 80, 0xFF8C00, "惑星調査");
}

void TitleScene::DrawMenu(void) const
{
	SetFontSize(MENU_FONT_SIZE);

	for (int i = 0; i < MENU_COUNT; ++i)
	{
		const bool isSelected = (i == selectIndex_);
		const int y = MENU_Y_START + i * MENU_LINE_HEIGHT;

		if (isSelected)
		{
			DrawFormatString(MENU_X - 40, y, 0xFFFFFF, ">");
			DrawFormatString(MENU_X, y, 0xFFFFFF, MENU_LABELS[i]);
		}
		else
		{
			DrawFormatString(MENU_X, y, 0xFF8C00, MENU_LABELS[i]);
		}
	}
}
