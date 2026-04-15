#pragma once
#include <DxLib.h>
#include "EnemyBase.h"
class Player;

class EnemyRat : public EnemyBase
{
public:

	// アニメーション種別
	enum class ANIM_TYPE
	{
		IDLE = 8,
		WALK = 13,
	};

	// 状態
	enum class STATE
	{
		NONE,
		THINK,
		IDLE,
		WANDER,
		END,
	};

	// コンストラクタ
	EnemyRat(const EnemyBase::EnemyData& data, Player* player);
	// デストラクタ
	~EnemyRat(void) override;

protected:

	// リソースロード
	virtual void InitLoad(void) override;
	// 大きさ、回転、座標の初期化
	virtual void InitTransform(void) override;
	// 衝突判定の初期化
	virtual void InitCollider(void) override;
	// アニメーションの初期化
	virtual void InitAnimation(void) override;
	// 初期化後の個別処理
	virtual void InitPost(void) override;

	// 更新系
	void UpdateProcess(void) override;
	void UpdateProcessPost(void) override;

private:

	// 状態
	STATE state_;

	// 更新ステップ
	float step_;

	// モデルの大きさ
	static constexpr float SCALE = 0.5f;

	// モデルのローカル回転
	static constexpr VECTOR ROT = { 0.0f, 180 * DX_PI_F / 180.0f, 0.0f };

	// 衝突判定用線分開始
	static constexpr VECTOR COL_LINE_START_LOCAL_POS = { 0.0f, 80.0f, 0.0f };
	// 衝突判定用線分終了
	static constexpr VECTOR COL_LINE_END_LOCAL_POS = { 0.0f, -10.0f, 0.0f };

	// 衝突判定用カプセル上部球体
	static constexpr VECTOR COL_CAPSULE_TOP_LOCAL_POS = { 0.0f, 110.0f, 0.0f };
	// 衝突判定用カプセル下部球体
	static constexpr VECTOR COL_CAPSULE_DOWN_LOCAL_POS = { 0.0f, 30.0f, 0.0f };

	// 衝突判定用カプセル球体半径
	static constexpr float COL_CAPSULE_RADIUS = 20.0f;

	// 状態遷移
	void ChangeState(STATE state);
	void ChangeStateNone(void);
	void ChangeStateThink(void);
	void ChangeStateIdle(void);
	void ChangeStateWander(void);
	void ChangeStateEnd(void);

	// 更新系
	void UpdateNone(void);
	void UpdateThink(void);
	void UpdateIdle(void);
	void UpdateWander(void);
	void UpdateEnd(void);
};

