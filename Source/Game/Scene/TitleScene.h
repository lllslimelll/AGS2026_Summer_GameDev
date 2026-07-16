#pragma once
#include <memory>
#include "../../Common/Transform.h"
#include "SceneBase.h"

class Camera;
class SkyDome;

class TitleScene : public SceneBase
{

public:

	// コンストラクタ
	TitleScene(void);

	// デストラクタ
	~TitleScene(void) override;

	// ロード
	void Load(void) override {};

	// 初期化
	void Init(void) override;

	// 更新
	void Update(void) override;

	// 描画
	void Draw(void) override;

private:

	// メニュー項目
	enum class MENU
	{
		GAME_START = 0,
		OPTION,
		QUIT_GAME,
		MAX
	};

	// 定数
	static constexpr int   MENU_COUNT = static_cast<int>(MENU::MAX);
	static constexpr int   MENU_CENTER_X = 430;  // メニューの中心X（タイトル中央あたり）
	static constexpr int   TITLE_X = 100;  // タイトル左X
	static constexpr int   MENU_Y_START = 630;   // 先頭項目のY座標
	static constexpr int   MENU_FONT_SIZE = 55;
	static constexpr int   TITLE_FONT_SIZE = 170;
	static constexpr int   MENU_LINE_HEIGHT = 80;
	static constexpr float PLANET_ROT_SPEED = 1.0f;  // 1フレームあたりの回転角(deg)

	// 選択インデックス
	int selectIndex_ = -1;

	std::unique_ptr<Camera> camera_;
	std::unique_ptr<SkyDome> skyDome_;

	int imgTitle_;
	int imgPushSpace_;

	// 球体惑星
	Transform spherePlanet_;

	// スカイドーム用の空Transform
	Transform empty_;

	// 内部処理
	void UpdateInput(void);
	void DrawTitle(void)  const;
	void DrawMenu(void)   const;

	int prevMouseX_ = -1;
	int prevMouseY_ = -1;
};
