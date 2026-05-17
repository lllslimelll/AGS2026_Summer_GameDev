#pragma once
#include <DxLib.h>
#include "../ActorBase.h"

class Item : public ActorBase
{
public:

	// 値段のグレード
	enum class GRADE {
		LOW, 
		MID, 
		HIGH
	};

	enum class STATE {
		NONE,
		DROPPED, // 落ちている
		HELD,   // インベントリ内
		FLYING, // 投擲中
	};

	// 拾える距離
	static constexpr float RANGE_PICKUP = 120.0f;

	// ビルボード（金額表示）を表示する距離
	static constexpr float RANGE_BILLBOARD = 500.0f;

	// 投擲時の初速
	static constexpr float THROW_SPEED = 40.0f;

	// グレード別の基本価値
	static constexpr int VALUE_LOW = 100;
	static constexpr int VALUE_MID = 500;
	static constexpr int VALUE_HIGH = 1000;

	// コンストラクタ
	Item();
	~Item(); // デストラクタ

	// 更新
	void Update(void) override;
	// 描画
	void Draw(void) override;

	// 拾われた
	void OnPickedUp(void);
	// 投擲された
	void OnThrow(const VECTOR& throwDir);
	// 敵に命中した
	void OnHitEnemy(void);

	// 取得
	int   GetValue(void) const; // 値段
	GRADE GetGrade(void) const; // グレード
	STATE GetState(void) const; // 状態

	// アイテム取得可能か
	void SetAimed(bool aimed);
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

private:

	// グレード
	GRADE grade_;

	// 現在の価値
	int value_;

	// 状態
	STATE state_;

	// 投擲中の移動量
	VECTOR throwPow_;

	// アイテムを拾える状態か
	bool isAimed_;

	// 状態遷移
	void ChangeState(STATE state);

	// 遷移時の初期化処理
	void ChangeStateDropped(void);
	void ChangeStateHeld(void);
	void ChangeStateFlying(void);

	// 状態別更新
	void UpdateDropped(void);
	void UpdateHeld(void);
	void UpdateFlying(void);

	// 投擲中の弾道計算
	void UpdateThrowMove(void);

	// ビルボード描画（金額表示）
	void DrawBillboard(void) const;
	
};

