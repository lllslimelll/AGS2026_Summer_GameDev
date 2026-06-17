#include <cmath>
#include <DxLib.h>
#include "../Application.h"
#include "../Utility/AsoUtility.h"
#include "../Object/Common/AnimationController.h"
#include "../Object/Actor/SkyDome.h"
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Manager/Camera.h"
#include "../Manager/ResourceManager.h"
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
	player_(),
	SceneBase(),
	skyDome_()
{
}

TitleScene::~TitleScene(void)
{
}

void TitleScene::Init(void)
{

	// PushSpace画像読み込み
	imgPushSpace_ = resMng_.Load(ResourceManager::SRC::PUSH_SPACE).handleId_;

	// 定点カメラ
	sceMng_.GetCamera()->ChangeMode(Camera::MODE::FIXED_POINT);

	// 球体惑星
	spherePlanet_.SetModel(resMng_.Load(			// 1個 = Load()  複数 = Depulicate()
		ResourceManager::SRC::MAIN_STAGE).handleId_);
	spherePlanet_.scl = { 0.7f, 0.7f, 0.7f };
	spherePlanet_.quaRot = Quaternion::Identity();

	spherePlanet_.pos = { 600, -300, 550 };
	spherePlanet_.Update();

	// スカイドーム
	skyDome_ = new SkyDome(empty_);
	skyDome_->Init();
}

void TitleScene::Update(void)
{
	skyDome_->Update();

	// 惑星にX軸に毎フレーム1°ずつ回転を追加
	spherePlanet_.quaRotLocal = Quaternion::Mult(
		spherePlanet_.quaRotLocal,
		Quaternion::AngleAxis(AsoUtility::Deg2RadF(-1.0f), AsoUtility::AXIS_Z));

	// モデル行列を更新
	spherePlanet_.Update();

	UpdateInput();
}

void TitleScene::UpdateInput(void)
{
	auto const& ins = InputManager::GetInstance();

	// ↑↓ でカーソル移動
	if (ins.IsTrgDown(KEY_INPUT_UP) ||
		ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1,
			InputManager::JOYPAD_BTN::TOP))
	{
		selectIndex_ = (selectIndex_ - 1 + MENU_COUNT) % MENU_COUNT;
	}
	if (ins.IsTrgDown(KEY_INPUT_DOWN) ||
		ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1,
			InputManager::JOYPAD_BTN::DOWN))
	{
		selectIndex_ = (selectIndex_ + 1) % MENU_COUNT;
	}

	// 決定
	bool decide = ins.IsTrgDown(KEY_INPUT_RETURN)
		|| ins.IsTrgDown(KEY_INPUT_SPACE)
		|| ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1,
			InputManager::JOYPAD_BTN::LEFT);
	if (decide)
	{
		switch (static_cast<MENU>(selectIndex_))
		{
		case MENU::GAME_START:
			sceMng_.ChangeScene(SceneManager::SCENE_ID::GAME);
			break;
		case MENU::QUIT_GAME:
			// DXライブラリにウィンドウ終了を要求
			PostQuitMessage(0);
			break;
		default:
			// TUTORIAL / OPTION / RANKING は未実装スタブ
			break;
		}
	}
}

void TitleScene::Draw(void)
{
	skyDome_->Draw();
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

void TitleScene::Release(void)
{
	skyDome_->Release();
	delete skyDome_;
}
