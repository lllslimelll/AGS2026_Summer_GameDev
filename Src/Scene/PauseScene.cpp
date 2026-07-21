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

	// マウスが動いた時だけホバー判定を行う
	bool mouseMoved = (mouseX != prevMouseX_ || mouseY != prevMouseY_);

	if (mouseMoved)
	{
		const char* labels[MENU_MAX] = { "ゲームに戻る", "設定", "タイトルへ戻る" };

		int baseY = screenH / 2 - 20;
		constexpr int ITEM_SPAN = 110;
		constexpr int MENU_FONT = 70;

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

	switch (static_cast<MENU>(menuIndex_))
	{
	case MENU::RESUME:
		sceMng_.PopOverlay();
		SoundManager::GetInstance().PlayBGMGame();
		break;
	case MENU::OPTION:
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

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 150);
	DrawBox(0, 0, screenW, screenH, 0x000000, TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	int prevSize = GetFontSize();

	constexpr int MENU_MAX = static_cast<int>(MENU::MAX);
	constexpr int MENU_FONT = 70;
	SetFontSize(MENU_FONT);

	const char* labels[MENU_MAX] =
	{
		"ゲームに戻る",
		"設定",
		"タイトルへ戻る",
	};

	constexpr int ITEM_SPAN = 110;
	// 3項目を画面縦中央に配置（中央の項目がscreenH/2に来るよう逆算）
	int totalHeight = ITEM_SPAN * (MENU_MAX - 1);
	int baseY = (screenH - totalHeight) / 2;

	for (int i = 0; i < MENU_MAX; i++)
	{
		bool selected = (i == menuIndex_);
		unsigned int color = selected ? 0xffff60 : 0xaaaaaa;

		int textW = GetDrawStringWidth(labels[i], (int)strlen(labels[i]));
		int x = (screenW - textW) / 2;
		int y = baseY + i * ITEM_SPAN;

		if (selected)
		{
			DrawString(x - 60, y, ">", color);
		}
		DrawString(x, y, labels[i], color);
	}

	SetFontSize(prevSize);
}
