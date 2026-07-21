#pragma once
#include <DxLib.h>
#include "../CharactorBase.h"
class Player;

class EnemyBase : public CharactorBase
{
public:

	enum class TYPE
	{
		RAT = 2, // ネズミ
		ROBOT = 1, // ロボット
		GIANT = 0
	};

	// エネミーデータ
	struct EnemyData
	{
		int id;				  // 識別ID
		EnemyBase::TYPE type; // 種類
		int hp;				  // 体力
		Vector3 defaultPos;    // 初期座標
		float movableRange;	  // 移動可能範囲
	};

	// コンストラクタ
	EnemyBase(const EnemyBase::EnemyData& data, Player& player);
	// デストラクタ
	virtual ~EnemyBase(void) override;

	// 描画
	virtual void Draw(void) override;

	// 当たり判定の通知
	// キャラ同士（PAWN）に当たったら敵側が押し出される
	void OnHit(const HitResult& hit) override;

protected:

	// プレイヤー
	Player& player_;

	// 種別
	TYPE type_;
	// HP
	int hp_;
	// 初期位置
	const Vector3 defaultPos_;
	// 移動可能範囲
	float movableRange_;

	// 更新系
	virtual void UpdateProcessPost(void) override {};

	// 計算による視野判定
	// dist    : 視野距離
	// halfFov : 視野角の半分（ラジアン）
	bool InSearchCone(float dist, float halfFovRad) const;

	// 移動可能範囲判定
	bool InMovableRange(void) const;
};

