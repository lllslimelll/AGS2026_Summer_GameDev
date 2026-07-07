#pragma once
#include "SceneBase.h"
#include <vector>
#include <memory>
class ActorBase;
class SkyDome;
class Stage;
class ItemManager;
class Player;
class Camera;
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

private:

	// スカイドーム
	SkyDome* skyDome_;
	// ステージ
	Stage* stage_;
	// アイテム
	ItemManager* itemMng_;
	// プレイヤー
	std::unique_ptr<Player> player_;
	// カメラ
	std::unique_ptr<Camera> camera_;
	// 敵
	EnemyManager* enemyManager_;

	// リアルシャドウ描画
	void DrawShadow(void);
};
