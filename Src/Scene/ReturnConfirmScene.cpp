#include <DxLib.h>
#include <string.h>
#include "../Manager/InputManager.h"
#include "../Manager/SoundManager.h"
#include "../Object/Actor/Stage/Rocket.h"
#include "SceneManager.h"
#include "ReturnConfirmScene.h"

// -----------------------------------------------------------------------------
// ENCODING POLICY
//
// This file is written in pure ASCII so it compiles regardless of whether the
// project's source charset is Shift-JIS, UTF-8, or anything else. That means:
//
//   * All comments are English.
//   * On-screen Japanese text is emitted as raw Shift-JIS byte sequences via
//     \x hex escapes, one Japanese glyph per pair of escapes. A trailing
//     comment shows the intended glyph. DxLib's DrawString expects SJIS bytes
//     by default on Japanese Windows, so the strings render correctly at
//     runtime even though the source contains no non-ASCII bytes.
//
// If the project is ever migrated to DX_CHARCODEFORMAT_UTF8, these string
// tables must be replaced with UTF-8 byte sequences.
// -----------------------------------------------------------------------------

namespace
{
	// ---- Layout constants ----
	constexpr int PANEL_W = 900;
	constexpr int PANEL_H = 380;

	constexpr int TITLE_FONT = 56;
	constexpr int WARN_FONT = 30;
	constexpr int MENU_FONT = 52;

	// Half-distance between the two menu labels, measured from the panel
	// centerline (px).
	constexpr int HALF_SPAN = 160;

	// Extra padding around each label to enlarge the mouse hit-box (px).
	constexpr int HIT_PAD_X = 60;
	constexpr int HIT_PAD_Y = 20;

	// ------------------------------------------------------------------------
	// On-screen strings, encoded as Shift-JIS byte sequences.
	// Each \x pair is one SJIS byte; two escapes form one full-width glyph.
	// Adjacent string literals are concatenated by the compiler.
	// ------------------------------------------------------------------------

	// "hai"
	const char STR_YES[] =
		"\x82\xCD"   // ha
		"\x82\xA2";  // i

	// "iie"
	const char STR_NO[] =
		"\x82\xA2"   // i
		"\x82\xA2"   // i
		"\x82\xA6";  // e

	// "kikan shimasu ka?"
	const char STR_TITLE[] =
		"\x8B\x41"   // ki
		"\x8A\xD2"   // kan
		"\x82\xB5"   // shi
		"\x82\xDC"   // ma
		"\x82\xB7"   // su
		"\x82\xA9"   // ka
		"\x81\x48";  // ?

	// "* noruma kingaku ni toutatsu shite imasen. hontou ni kikan shimasu ka?"
	const char STR_WARN[] =
		"\x81\xA6"   // *
		"\x83\x6D"   // no
		"\x83\x8B"   // ru
		"\x83\x7D"   // ma
		"\x8B\xE0"   // kin
		"\x8A\x7A"   // gaku
		"\x82\xC9"   // ni
		"\x93\x9E"   // tou
		"\x92\x42"   // tatsu
		"\x82\xB5"   // shi
		"\x82\xC4"   // te
		"\x82\xA2"   // i
		"\x82\xDC"   // ma
		"\x82\xB9"   // se
		"\x82\xF1"   // n
		"\x81\x42"   // .
		"\x96\x7B"   // hon
		"\x93\x96"   // tou
		"\x82\xC9"   // ni
		"\x8B\x41"   // ki
		"\x8A\xD2"   // kan
		"\x82\xB5"   // shi
		"\x82\xDC"   // ma
		"\x82\xB7"   // su
		"\x82\xA9"   // ka
		"\x81\x48";  // ?
}

// -----------------------------------------------------------------------------
// Construction / destruction
// -----------------------------------------------------------------------------
ReturnConfirmScene::ReturnConfirmScene(void)
	:
	SceneBase(),
	menuIndex_(static_cast<int>(MENU::YES)),   // default focus: Yes
	cleared_(false),
	totalScore_(0),
	prevMouseX_(-1),
	prevMouseY_(-1),
	prevMouseDown_(false),
	prevMouseDispFlag_(0)
{
}

ReturnConfirmScene::~ReturnConfirmScene(void)
{
	// Restore whatever the game scene had before we took over.
	SetMouseDispFlag(prevMouseDispFlag_);
}

void ReturnConfirmScene::Init(void)
{
	// Player has already stored the current score via SetResultScore() before
	// pushing this overlay, so we just read it back.
	totalScore_ = sceMng_.GetResultScore();
	cleared_ = (totalScore_ >= Rocket::QUOTA);

	// Save-and-show the mouse cursor. Restored in the destructor.
	prevMouseDispFlag_ = GetMouseDispFlag();
	SetMouseDispFlag(TRUE);

	// Prime edge-detection state so we don't false-trigger on the first frame
	// if the mouse happened to be held down when the scene opened.
	prevMouseDown_ = (GetMouseInput() & MOUSE_INPUT_LEFT) != 0;
	GetMousePoint(&prevMouseX_, &prevMouseY_);
}

void ReturnConfirmScene::Update(void)
{
	UpdateMenu();
}

void ReturnConfirmScene::Draw(void)
{
	DrawMenu();
}

// -----------------------------------------------------------------------------
// Input
// -----------------------------------------------------------------------------
void ReturnConfirmScene::UpdateMenu(void)
{
	auto& ins = InputManager::GetInstance();

	// UI_CANCEL is a fast path for "No".
	if (ins.IsTriggered(InputManager::InputCommand::UI_CANCEL))
	{
		OnConfirmNo();
		return;
	}

	// Keyboard / pad: left-right cycles the selection.
	constexpr int MENU_MAX = static_cast<int>(MENU::MAX);
	if (ins.IsTriggered(InputManager::InputCommand::UI_LEFT))
	{
		menuIndex_ = (menuIndex_ - 1 + MENU_MAX) % MENU_MAX;
	}
	if (ins.IsTriggered(InputManager::InputCommand::UI_RIGHT))
	{
		menuIndex_ = (menuIndex_ + 1) % MENU_MAX;
	}

	// ---- Mouse handling ----
	int screenW, screenH;
	GetScreenState(&screenW, &screenH, nullptr);

	int mouseX, mouseY;
	GetMousePoint(&mouseX, &mouseY);

	const bool mouseMoved =
		(mouseX != prevMouseX_ || mouseY != prevMouseY_);
	prevMouseX_ = mouseX;
	prevMouseY_ = mouseY;

	// Label positions must exactly match those used in DrawMenu().
	const int centerX = screenW / 2;
	const int menuY = (screenH / 2) + PANEL_H / 2 - 90;
	const int labelX[MENU_MAX] =
	{
		centerX - HALF_SPAN,
		centerX + HALF_SPAN,
	};
	const char* labels[MENU_MAX] = { STR_YES, STR_NO };

	// Only react to hover on frames where the mouse actually moved. This lets
	// pad / keyboard users keep their selection without the mouse "stealing"
	// it every frame.
	if (mouseMoved)
	{
		int prevSize = GetFontSize();
		SetFontSize(MENU_FONT);

		int hoverIndex = -1;
		for (int i = 0; i < MENU_MAX; i++)
		{
			int textW = GetDrawStringWidth(labels[i], (int)strlen(labels[i]));
			int left = labelX[i] - textW / 2 - HIT_PAD_X;
			int right = labelX[i] + textW / 2 + HIT_PAD_X;
			int top = menuY - HIT_PAD_Y;
			int bottom = menuY + MENU_FONT + HIT_PAD_Y;

			if (mouseX >= left && mouseX <= right &&
				mouseY >= top && mouseY <= bottom)
			{
				hoverIndex = i;
				break;
			}
		}

		SetFontSize(prevSize);

		if (hoverIndex >= 0) menuIndex_ = hoverIndex;
	}

	// Left-click edge detection.
	const bool mouseDown = (GetMouseInput() & MOUSE_INPUT_LEFT) != 0;
	const bool mouseClicked = (mouseDown && !prevMouseDown_);
	prevMouseDown_ = mouseDown;

	const bool decide =
		ins.IsTriggered(InputManager::InputCommand::UI_DECIDE) || mouseClicked;

	if (!decide)         return;
	if (menuIndex_ < 0)  return; // cursor is off both labels

	SoundManager::GetInstance().PlaySelect();

	switch (static_cast<MENU>(menuIndex_))
	{
	case MENU::YES: OnConfirmYes(); return;
	case MENU::NO:  OnConfirmNo();  return;
	default: break;
	}
}

// -----------------------------------------------------------------------------
// Drawing
// -----------------------------------------------------------------------------
void ReturnConfirmScene::DrawMenu(void)
{
	int screenW, screenH;
	GetScreenState(&screenW, &screenH, nullptr);

	const int panelL = (screenW - PANEL_W) / 2;
	const int panelT = (screenH - PANEL_H) / 2;
	const int panelR = panelL + PANEL_W;
	const int panelB = panelT + PANEL_H;

	// Full-screen dim behind the panel.
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);
	DrawBox(0, 0, screenW, screenH, 0x000000, TRUE);

	// Panel body and border.
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 220);
	DrawBox(panelL, panelT, panelR, panelB, 0x1a1a1a, TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	DrawBox(panelL, panelT, panelR, panelB, 0xffffff, FALSE);

	int prevSize = GetFontSize();

	// ---- Title ----
	SetFontSize(TITLE_FONT);
	int titleW = GetDrawStringWidth(STR_TITLE, (int)strlen(STR_TITLE));
	int titleY = panelT + 60;
	DrawString((screenW - titleW) / 2, titleY, STR_TITLE, 0xffffff);

	// ---- Warning line (only when quota is not met) ----
	if (!cleared_)
	{
		SetFontSize(WARN_FONT);
		int warnW = GetDrawStringWidth(STR_WARN, (int)strlen(STR_WARN));
		int warnY = titleY + TITLE_FONT + 20;
		DrawString((screenW - warnW) / 2, warnY, STR_WARN, 0xff6060);
	}

	// ---- Yes / No labels ----
	constexpr int MENU_MAX = static_cast<int>(MENU::MAX);
	const char* labels[MENU_MAX] = { STR_YES, STR_NO };
	const int centerX = screenW / 2;
	const int menuY = (screenH / 2) + PANEL_H / 2 - 90;
	const int labelX[MENU_MAX] =
	{
		centerX - HALF_SPAN,
		centerX + HALF_SPAN,
	};

	SetFontSize(MENU_FONT);
	for (int i = 0; i < MENU_MAX; i++)
	{
		bool selected = (i == menuIndex_);
		unsigned int color = selected ? 0xffff80 : 0x888888;

		int textW = GetDrawStringWidth(labels[i], (int)strlen(labels[i]));
		int x = labelX[i] - textW / 2;

		DrawString(x, menuY, labels[i], color);

		// Underline the currently focused label.
		if (selected)
		{
			DrawLine(
				x, menuY + MENU_FONT + 4,
				x + textW, menuY + MENU_FONT + 4,
				0xffff80, 2);
		}
	}

	SetFontSize(prevSize);
}

// -----------------------------------------------------------------------------
// Confirmation handlers
//
// IMPORTANT: PopOverlay() destroys `this`. After calling it, do not touch any
// member of ReturnConfirmScene. Both helpers below return immediately from
// their caller.
// -----------------------------------------------------------------------------
void ReturnConfirmScene::OnConfirmYes(void)
{
	auto& sm = SceneManager::GetInstance();
	sm.PopOverlay();  // <-- `this` is destroyed here
	sm.PushOverlay(SceneManager::SCENE_ID::RESULT);
}

void ReturnConfirmScene::OnConfirmNo(void)
{
	// Pop self -> back to GameScene. `this` is destroyed by this call.
	SceneManager::GetInstance().PopOverlay();
}