#pragma once
#include "SceneBase.h"
#include <vector>
#include <memory>
class ActorBase;
class SkyDome;
class StageManager;
class ItemManager;
class Player;
class EnemyManager;
class GameUI;

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

private:

	// スカイドーム
	SkyDome* skyDome_;
	// ステージ
	std::unique_ptr<StageManager> stageMng_;
	// アイテム
	ItemManager* itemMng_;
	// プレイヤー
	std::unique_ptr<Player> player_;
	// 敵
	EnemyManager* enemyManager_;
	// ゲームUI
	std::vector<std::unique_ptr<GameUI>> gameUIs_;

	int shadowMapHandle_ = -1;

	// シャドウマップ作成
	int CreateShadowMap(void);

	bool NeedsCamera(void) const override { return true; }
};
