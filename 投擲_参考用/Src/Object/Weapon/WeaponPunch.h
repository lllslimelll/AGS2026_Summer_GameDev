#pragma once
#include <DxLib.h>
#include "WeaponBase.h"
class WeaponPunch : public WeaponBase
{
public:
	// エフェクト画像の分割数
	static constexpr int NUM_SPRITE_X = 4;
	static constexpr int NUM_SPRITE_Y = 3;
	static constexpr int NUM_SPRITE_ALL = NUM_SPRITE_X * NUM_SPRITE_Y;
	// エフェクト画像の分割された大きさ(X)
	static constexpr int SIZE_XSPRITE_X = 100;
	static constexpr int SIZE_XSPRITE_Y = 80;
	// エフェクト画像の分割された大きさ(Y)
	static constexpr int SIZE_YSPRITE_X = 80;
	static constexpr int SIZE_YSPRITE_Y = 100;
	// 画像の大きさ倍率
	static constexpr float IMG_SCALE = 280.0f;

	// コンストラクタ
	WeaponPunch(void);
	// デストラクタ
	~WeaponPunch(void);
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
	// アニメーションカウンタ
	int cntAnimation_;
	// エフェクト画像(横)
	int imgsX_[NUM_SPRITE_ALL];
	// エフェクト画像(縦)
	int imgsY_[NUM_SPRITE_ALL];
};

