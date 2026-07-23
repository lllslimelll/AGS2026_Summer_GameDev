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
static const char* MENU_LABELS[] =
{
	"探査を開始",
	"オプション",
	"終了"
};

// 水平の点線（DxLibに点線APIは無いので短い破線を並べる）
static void DrawDottedLineH(int x1, int x2, int y, unsigned int color,
	int dashLen, int gapLen, int thickness)
{
	for (int x = x1; x < x2; x += dashLen + gapLen)
	{
		const int ex = (x + dashLen < x2) ? x + dashLen : x2;
		DrawBox(x, y, ex, y + thickness, color, TRUE);
	}
}
// 選択中 = 赤、非選択 = 白（参考画像準拠）
static constexpr unsigned int COLOR_SELECT = 0xFF2222;
static constexpr unsigned int COLOR_UNSELECT = 0xFFFFFF;
static constexpr unsigned int COLOR_TITLE = 0xFFFFFF;


static constexpr int LINE_WIDTH = 500;               // 点線の長さ
static constexpr int LINE_PAD_Y = 130;                // メニュー上下と点線の間隔
static constexpr unsigned int COLOR_LINE = 0xFFFFFF; // 点線色（文字色とは無関係）

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
		ResourceManager::SRC::STAGE).handleId_);
	spherePlanet_.scl = { 0.7f, 0.7f, 0.7f };
	spherePlanet_.quaRot = Quaternion::Identity();

	spherePlanet_.pos = { 900, -600, 300 };
	spherePlanet_.quaRotLocal = Quaternion::Mult(
		spherePlanet_.quaRotLocal,
		Quaternion::AngleAxis(AsoUtility::Deg2RadF(50.0f), AsoUtility::AXIS_Y));
	spherePlanet_.quaRotLocal = Quaternion::Mult(
		spherePlanet_.quaRotLocal,
		Quaternion::AngleAxis(AsoUtility::Deg2RadF(-20.0f), AsoUtility::AXIS_X));
	spherePlanet_.quaRotLocal = Quaternion::Mult(
		spherePlanet_.quaRotLocal,
		Quaternion::AngleAxis(AsoUtility::Deg2RadF(30.0f), AsoUtility::AXIS_Z));
	spherePlanet_.Update();

	// 定点カメラ
	SceneManager::GetInstance().GetCamera().ChangeMode(Camera::MODE::FIXED_POINT);

	// スカイドーム
	skyDome_ = std::make_unique<SkyDome>(empty_);
	skyDome_->Init();

}

void TitleScene::Update(void)
{
	skyDome_->Update();

	// 惑星にX軸に毎フレーム1°ずつ回転を追加
	/*spherePlanet_.quaRotLocal = Quaternion::Mult(
		spherePlanet_.quaRotLocal,
		Quaternion::AngleAxis(AsoUtility::Deg2RadF(-0.1f), AsoUtility::AXIS_Z));*/

	spherePlanet_.quaRotLocal = Quaternion::Mult(
		spherePlanet_.quaRotLocal,
		Quaternion::AngleAxis(AsoUtility::Deg2RadF(-0.05f), AsoUtility::AXIS_Y));
	//spherePlanet_.quaRotLocal = Quaternion::Mult(
	//	spherePlanet_.quaRotLocal,
	//	Quaternion::AngleAxis(AsoUtility::Deg2RadF(-0.00f), AsoUtility::AXIS_Z));
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
		const int halfW = LINE_WIDTH / 2;   // 判定幅は点線幅に合わせる（好みで調整）
		for (int i = 0; i < MENU_COUNT; ++i)
		{
			int itemY = MENU_Y_START + i * MENU_LINE_HEIGHT;
			if (mouseX >= MENU_CENTER_X - halfW && mouseX <= MENU_CENTER_X + halfW &&
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
	DrawFormatString(TITLE_X, 190, 0xFF8C00, "惑星探査");
}

void TitleScene::DrawMenu(void) const
{
	// --- メニューを挟む上下の点線（中心基準で左右対称に） ---
	const int lastItemY = MENU_Y_START + (MENU_COUNT - 1) * MENU_LINE_HEIGHT;
	const int frameTop = MENU_Y_START - LINE_PAD_Y;
	const int frameBottom = lastItemY + MENU_FONT_SIZE + LINE_PAD_Y;

	DrawDottedLineH(MENU_CENTER_X - LINE_WIDTH / 2, MENU_CENTER_X + LINE_WIDTH / 2,
		frameTop, COLOR_LINE, 2, 3, 1);
	DrawDottedLineH(MENU_CENTER_X - LINE_WIDTH / 2, MENU_CENTER_X + LINE_WIDTH / 2,
		frameBottom, COLOR_LINE, 2, 3, 1);

	SetFontSize(MENU_FONT_SIZE);

	const int triH = MENU_FONT_SIZE;
	const int triW = MENU_FONT_SIZE / 2;
	const int gap = MENU_FONT_SIZE / 3;

	for (int i = 0; i < MENU_COUNT; ++i)
	{
		const bool isSelected = (i == selectIndex_);
		const int  y = MENU_Y_START + i * MENU_LINE_HEIGHT;

		// 項目ごとに幅を測り、中心から左右均等に配置する
		const int len = static_cast<int>(std::strlen(MENU_LABELS[i]));
		const int textW = GetDrawStringWidth(MENU_LABELS[i], len);
		const int textX = MENU_CENTER_X - textW / 2;   // この項目の開始X

		const unsigned int color = isSelected ? 0xFFFFFF : 0xFF8C00;
		DrawFormatString(textX, y, color, MENU_LABELS[i]);

		if (!isSelected) { continue; }

		const int cy = y + triH / 2;

		// 左：右向き三角（テキスト左端の手前）
		const int lTip = textX - gap;
		const int lBase = lTip - triW;
		DrawTriangle(lBase, cy - triH / 2, lBase, cy + triH / 2, lTip, cy, 0xFFFFFF, TRUE);

		// 右：左向き三角（テキスト右端の先）
		const int rTip = textX + textW + gap;
		const int rBase = rTip + triW;
		DrawTriangle(rBase, cy - triH / 2, rBase, cy + triH / 2, rTip, cy, 0xFFFFFF, TRUE);
	}
}
