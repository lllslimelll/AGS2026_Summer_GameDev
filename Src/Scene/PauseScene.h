#pragma once
<<<<<<< HEAD
#include "Scene"

class PauseScene : public Scene
{
=======
#include "Scene.h"

class PauseScene : public Scene
{
public:

>>>>>>> c43593c588c266bd4e7c5e84d0a77fe0702bb34e
	PauseScene();
	~PauseScene() override;

	void Init() override;
	void Update() override;
	void Draw() override;
<<<<<<< HEAD
=======

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
>>>>>>> c43593c588c266bd4e7c5e84d0a77fe0702bb34e
};