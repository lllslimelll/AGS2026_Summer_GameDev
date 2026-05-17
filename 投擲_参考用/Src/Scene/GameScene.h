#pragma once
#include "SceneBase.h"

class SceneManager;
class HpManager;
class Camera;
class Grid;
class BlockManager;
class Player;
class EnemyBase;
class EnemyManager;

class GameScene : public SceneBase
{
public:

	// 使用武器アイコンの描画座標
	static constexpr int USE_WEAPON_ICON_POS_X = 61;
	static constexpr int USE_WEAPON_ICON_POS_Y = 150;

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;
	void Release(void) override;

	// 敵ベースを取得
	EnemyBase* GetEnemyBase(void) const;

private:

	Camera* camera_;
	Grid* grid_;
	// ブロック管理
	BlockManager* blockManager_;
	Player* player_;
	EnemyBase* enemyBase_;
	EnemyManager* enemyManager_;
	HpManager* hpManager_;

	// ゲームオーバーフラグ
	bool isGameOver_;
	// ゲームオーバー画像
	int imgGameOver_;

	// ゲームクリアフラグ
	bool isGameClear_;
	// ゲームクリア画像
	int imgGameClear_;

	// パンチアイコン
	int imgWeaponIconPunch_;
	// 斧アイコン
	int imgWeaponIconAxe_;

	// 地面との衝突用線分
	VECTOR lineTopPos_;
	VECTOR lineDownPos_;

	// 衝突判定
	void Collision(void);

	// ステージブロックとプレイヤーの衝突
	void CollisionStage(void);

	// 敵の当たり判定
	void CollisionEnemy(void);

	void CollisionEShot(void);

	// 敵とプレイヤーの武器の衝突
	void CollisionWeapon(void);

};