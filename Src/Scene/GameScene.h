#pragma once
#include "SceneBase.h"
#include <vector>
class ActorBase;
class SkyDome;
class Stage;
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
	// 全てのアクター
	std::vector<ActorBase*> allActor_;

	// スカイドーム
	SkyDome* skyDome_;
	// ステージ
	Stage* stage_;
	// プレイヤー
	Player* player_;
	// 敵
	EnemyManager* enemyManager_;

	// リアルシャドウ描画
	void DrawShadow(void);
};
