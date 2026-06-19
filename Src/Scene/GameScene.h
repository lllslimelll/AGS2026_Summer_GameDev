#pragma once
#include "SceneBase.h"
#include <vector>
class ActorBase;
class SkyDome;
class Stage;
class ItemManager;
class Player;
class EnemyManager;


class GameScene : public SceneBase
{

public:
	
	// コンストラクタ
	GameScene(void);

	// デストラクタ
	~GameScene(void) override;

	// 初期化
	void Init(void) override;

	// 更新
	void Update(void) override;

	// 描画
	void Draw(void) override;

	// 解放
	void Release(void) override;

private:

	// スカイドーム
	SkyDome* skyDome_;
	// ステージ
	Stage* stage_;
	// アイテム
	ItemManager* itemMng_;
	// プレイヤー
	Player* player_;
	// 敵
	EnemyManager* enemyManager_;

	// リアルシャドウ描画
	void DrawShadow(void);

	bool isPaused_;
	int  pauseMenuIndex_;
	// マウス移動検知用（前フレームの座標）
	int prevMouseX_ = -1;
	int prevMouseY_ = -1;

	enum class PAUSE_MENU
	{
		RESUME,
		OPTION,
		TITLE,
		MAX,
	};

	void UpdatePauseMenu(void);
	void DrawPauseMenu(void);
};
