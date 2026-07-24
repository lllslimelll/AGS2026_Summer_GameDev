#include <DxLib.h>
#include "../Manager/InputManager.h"
#include "../Manager/SoundManager.h"
#include "SceneManager.h"
#include "PauseScene.h"

PauseScene::PauseScene()
	:
	SceneBase()
{
}

PauseScene::~PauseScene()
{
}

void PauseScene::Init()
{
	SetMouseDispFlag(true);
}

void PauseScene::Update()
{
	// ポーズメニューの更新
	UpdatePauseMenu();
}

void PauseScene::Draw()
{
	DrawPauseMenu();
}

void PauseScene::UpdatePauseMenu(void)
{
	auto& ins = InputManager::GetInstance();

	int prevIndex = menuIndex_;

	bool up = ins.IsTriggered(InputManager::InputCommand::UI_UP);
	bool down = ins.IsTriggered(InputManager::InputCommand::UI_DOWN);

	constexpr int MENU_MAX = static_cast<int>(MENU::MAX);

	if (up)
	{
		menuIndex_ = (menuIndex_ <= 0) ? MENU_MAX - 1 : menuIndex_ - 1;
	}
	if (down)
	{
		menuIndex_ = (menuIndex_ < 0 || menuIndex_ >= MENU_MAX - 1) ? 0 : menuIndex_ + 1;
	}

	// ===== マウスでの選択 =====
	int screenW, screenH;
	GetScreenState(&screenW, &screenH, nullptr);

	int mouseX, mouseY;
	GetMousePoint(&mouseX, &mouseY);

	bool mouseMoved = (mouseX != prevMouseX_ || mouseY != prevMouseY_);

	if (mouseMoved)
	{
		const char* labels[MENU_MAX] = { "ゲームに戻る", "設定", "タイトルへ戻る" };

		constexpr int ITEM_SPAN = 110;
		constexpr int MENU_FONT = 70;

		int totalHeight = ITEM_SPAN * (MENU_MAX - 1);
		int baseY = (screenH - totalHeight) / 2;

		int prevSize = GetFontSize();
		SetFontSize(MENU_FONT);

		int mouseHoverIndex = -1;
		for (int i = 0; i < MENU_MAX; i++)
		{
			int textW = GetDrawStringWidth(labels[i], (int)strlen(labels[i]));
			int x = (screenW - textW) / 2;
			int y = baseY + i * ITEM_SPAN;

			if (mouseX >= x - 60 && mouseX <= x + textW &&
				mouseY >= y && mouseY <= y + ITEM_SPAN)
			{
				mouseHoverIndex = i;
				break;
			}
		}

		SetFontSize(prevSize);
		menuIndex_ = mouseHoverIndex;
	}

	prevMouseX_ = mouseX;
	prevMouseY_ = mouseY;

	// 決定
	bool decide = ins.IsTriggered(InputManager::InputCommand::UI_DECIDE);
	if (!decide || menuIndex_ < 0) return;

	SoundManager::GetInstance().PlaySelect();
	switch (static_cast<MENU>(menuIndex_))
	{
	case MENU::RESUME:
		sceMng_.PopOverlay();
		SoundManager::GetInstance().PlayBGMGame();
		break;
	case MENU::OPTION:
		sceMng_.PushOverlay(SceneManager::SCENE_ID::OPTION);
		break;
	case MENU::TITLE:
		sceMng_.ChangeScene(SceneManager::SCENE_ID::TITLE);
		SoundManager::GetInstance().StopBGMGame();
		SoundManager::GetInstance().StopWalk();
		break;
	default: break;
	}
}

void PauseScene::DrawPauseMenu(void)
{
	int screenW, screenH;
	GetScreenState(&screenW, &screenH, nullptr);

	// 半透明オーバーレイ
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 150);
	DrawBox(0, 0, screenW, screenH, 0x000000, TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	int prevSize = GetFontSize();
	constexpr int MENU_MAX = static_cast<int>(MENU::MAX);

	// ===== タイトル文字 =====
	constexpr int TITLE_FONT = 100;
	SetFontSize(TITLE_FONT);
	const char* title = "ポーズ";
	int titleW = GetDrawStringWidth(title, (int)strlen(title));
	DrawString((screenW - titleW) / 2, screenH / 6, title, 0xFF8C00);

	// ===== メニュー =====
	constexpr int MENU_FONT = 70;
	constexpr int ITEM_SPAN = 110;
	constexpr int LINE_WIDTH = 500;
	constexpr int LINE_PAD_Y = 60;

	SetFontSize(MENU_FONT);

	const char* labels[MENU_MAX] =
	{
		"ゲームに戻る",
		"設定",
		"タイトルへ戻る",
	};

	int totalHeight = ITEM_SPAN * (MENU_MAX - 1);
	int baseY = (screenH - totalHeight) / 2;
	int centerX = screenW / 2;

	// 上下の点線
	const int frameTop = baseY - LINE_PAD_Y;
	const int lastY = baseY + (MENU_MAX - 1) * ITEM_SPAN;
	const int frameBottom = lastY + MENU_FONT + LINE_PAD_Y;
	DrawDottedLineH(centerX - LINE_WIDTH / 2, centerX + LINE_WIDTH / 2,
		frameTop, 0xFFFFFF, 2, 3, 1);
	DrawDottedLineH(centerX - LINE_WIDTH / 2, centerX + LINE_WIDTH / 2,
		frameBottom, 0xFFFFFF, 2, 3, 1);

	// 三角マーカーのサイズ
	const int triH = MENU_FONT;
	const int triW = MENU_FONT / 2;
	const int gap = MENU_FONT / 3;

	for (int i = 0; i < MENU_MAX; i++)
	{
		bool selected = (i == menuIndex_);
		unsigned int color = selected ? 0xFFFFFF : 0xFF8C00;

		int textW = GetDrawStringWidth(labels[i], (int)strlen(labels[i]));
		int x = centerX - textW / 2;
		int y = baseY + i * ITEM_SPAN;

		DrawString(x, y, labels[i], color);

		if (!selected) continue;

		// 左右の三角マーカー
		const int cy = y + triH / 2;

		const int lTip = x - gap;
		const int lBase = lTip - triW;
		DrawTriangle(lBase, cy - triH / 2, lBase, cy + triH / 2, lTip, cy, 0xFFFFFF, TRUE);

		const int rTip = x + textW + gap;
		const int rBase = rTip + triW;
		DrawTriangle(rBase, cy - triH / 2, rBase, cy + triH / 2, rTip, cy, 0xFFFFFF, TRUE);
	}

	SetFontSize(prevSize);
}

void PauseScene::DrawDottedLineH(int x1, int x2, int y, unsigned int color, int dashLen, int gapLen, int thickness)
{
	for (int x = x1; x < x2; x += dashLen + gapLen)
	{
		const int ex = (x + dashLen < x2) ? x + dashLen : x2;
		DrawBox(x, y, ex, y + thickness, color, TRUE);
	}
}
