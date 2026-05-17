#pragma once
#include <DxLib.h>
class WeaponBase
{
public:
	// 武器種別
	enum class TYPE
	{
		PUNCH = 0,
		AXE,
		MAX
	};
	// コンストラクタ
	WeaponBase(void);
	// デストラクタ
	~WeaponBase(void);
	void Init(TYPE type);
	void Update(void);
	virtual void Draw(void) = 0;
	virtual void Release(void) = 0;

	// 座標の取得
	VECTOR GetPos(void);

	// 衝突判定用半径
	float GetCollisionRadius(void);

	// 武器を使用する
	virtual void Use(VECTOR pos, VECTOR dir) = 0;

	// 生存判定
	bool IsAlive(void);

	// 武器種別の取得
	TYPE GetType(void);
protected:
	// 武器種別
	TYPE type_;
	int modelId_;
	VECTOR pos_;
	VECTOR angles_;
	VECTOR scales_;

	// 衝突判定用半径
	float collisionRadius_;

	// 移動スピード
	float speed_;
	// 移動方向
	VECTOR moveDir_;

	// 生存判定
	bool isAlive_;
	// 使用時の位置調整(Y)
	VECTOR localPos_;

	// 画像やモデルなどのロード(純粋仮想関数)
	virtual void Load(void) = 0;
	// パラメータ設定(純粋仮想関数)
	virtual void SetParam(void) = 0;
	// 移動処理
	virtual void Move(void);
};

