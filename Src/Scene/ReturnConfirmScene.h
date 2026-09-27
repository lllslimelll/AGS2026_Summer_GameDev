#pragma once
#include "SceneBase.h"

// -----------------------------------------------------------------------------
// Return-to-title confirmation overlay scene.
//
// - Pushed by Player when the RETURN input is triggered while aiming at the
//   rocket. See Player::UpdateItem().
// - Because SceneManager::Update() only updates scenes_.back(), GameScene and
//   Player are automatically frozen while this scene sits on top of the stack.
//   HP / oxygen do not decrease -- no explicit "pause" code is required here.
// - Menu behavior:
//     Yes         -> Pop self, then Push ResultScene.
//     No          -> Pop self (returns to GameScene).
//     UI_CANCEL   -> Same as No.
// - When the quota has not been met, an extra warning line is shown above the
//   Yes/No prompt. When the quota has been met, only the prompt is shown.
//
// Encoding note:
//   All comments and identifiers in this header are ASCII, so the file compiles
//   regardless of MSVC's source-charset assumption (SJIS / UTF-8 / etc.).
// -----------------------------------------------------------------------------
class ReturnConfirmScene : public SceneBase
{
public:

	ReturnConfirmScene(void);
	~ReturnConfirmScene(void) override;

	void Init(void)   override;
	void Update(void) override;
	void Draw(void)   override;

private:

	// Menu items.
	enum class MENU
	{
		YES,
		NO,
		MAX,
	};

	// Currently focused menu item. May be -1 while the mouse is off both
	// labels; keyboard / pad input immediately snaps it back to 0..MAX-1.
	int menuIndex_;

	// Snapshot captured in Init(); used only for display.
	bool cleared_;     // true when the delivery quota is met
	int  totalScore_;  // current total delivered value

	// Previous-frame input state (for edge detection).
	int  prevMouseX_;
	int  prevMouseY_;
	bool prevMouseDown_;

	// Previous mouse-cursor visibility, restored in the destructor so the game
	// scene keeps its cursor hidden after "No" is selected.
	int  prevMouseDispFlag_;

	void UpdateMenu(void);
	void DrawMenu(void);

	void OnConfirmYes(void);
	void OnConfirmNo(void);
};