#pragma once
#include <DxLib.h>
#include "WeaponBase.h"

class WeaponAxe : public WeaponBase
{
public:

	// 使用時のジャンプ力
	static constexpr float JUMP_POW = 30.0f;

	// 重力
	static constexpr float GRAVITY = 2.0f;

	// コンストラクタ
	WeaponAxe(void);
	// デストラクタ
	~WeaponAxe(void);

	void Draw(void) override;
	void Release(void) override;

	// 武器を使用する
	void Use(VECTOR pos, VECTOR dir) override;

protected:
	// 画像やモデルなどのロード
	void Load(void) override;
	// パラメータ設定
	void SetParam(void) override;
private:

	// ジャンプ力
	float jumpPow_;

	// 移動処理
	void Move(void) override;
};
