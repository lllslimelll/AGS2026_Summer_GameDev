#pragma once
#include "SceneBase.h"

class PauseScene : public SceneBase
{
public:

	PauseScene();
	~PauseScene() override;

	void Init() override;
	void Update() override;
	void Draw() override;

private:

	// メニュー選択肢のインデックス
	int menuIndex_;

	// マウス移動検知用（前フレームの座標）
	int prevMouseX_ = -1;
	int prevMouseY_ = -1;

	enum class MENU
	{
		RESUME,
		OPTION,
		TITLE,
		MAX,
	};

	void UpdatePauseMenu(void);
	void DrawPauseMenu(void);

	static void DrawDottedLineH(int x1, int x2, int y, unsigned int color,
		int dashLen, int gapLen, int thickness);
};