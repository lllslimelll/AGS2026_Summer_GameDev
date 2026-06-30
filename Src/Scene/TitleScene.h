#pragma once
#include "../Object/Common/Transform.h"
#include "Scene.h"
class SkyDome;

class TitleScene : public SceneBase
{

public:

	// コンストラクタ
	TitleScene(void);

	// デストラクタ
	~TitleScene(void) override;

	// 初期化
	void Init(void) override;

	// 更新
	void Update(void) override;

	// 描画
	void Draw(void) override;

	// 解放
	void Release(void) override;

private:

	// メニュー項目
	enum class MENU
	{
		GAME_START = 0,
		TUTORIAL,
		OPTION,
		RANKING,
		QUIT_GAME,
		MAX
	};

	// 定数
	static constexpr int   MENU_COUNT = static_cast<int>(MENU::MAX);
	static constexpr int   MENU_X = 120;   // 左余白
	static constexpr int   MENU_Y_START = 420;   // 先頭項目のY座標
	static constexpr int   MENU_FONT_SIZE = 48;
	static constexpr int   TITLE_FONT_SIZE = 150;
	static constexpr int   MENU_LINE_HEIGHT = 80;
	static constexpr float PLANET_ROT_SPEED = 1.0f;  // 1フレームあたりの回転角(deg)

	// 選択インデックス
	int selectIndex_ = -1;

	SkyDome* skyDome_;

	int imgTitle_;
	int imgPushSpace_;

	// 球体惑星
	Transform spherePlanet_;

	// プレイヤー
	Transform player_;

	// スカイドーム用の空Transform
	Transform empty_;

	// 内部処理
	void UpdateInput(void);
	void DrawTitle(void)  const;
	void DrawMenu(void)   const;

	int prevMouseX_ = -1;
	int prevMouseY_ = -1;
};
