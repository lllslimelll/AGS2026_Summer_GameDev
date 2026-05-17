#include "../../Application.h"
#include "../../Utility/AsoUtility.h"
#include "WeaponPunch.h"
WeaponPunch::WeaponPunch(void)
{
}
WeaponPunch::~WeaponPunch(void)
{
}
void WeaponPunch::Draw(void)
{
	if (!isAlive_)
	{
		return;
	}

	bool isXRev = false;
	bool isYRev = false;
	int img = -1;

	// 横画像
	if (moveDir_.x != 0.0f) {
		if (moveDir_.x > 0.0f) {
			isXRev = true;
		}
		img = imgsX_[cntAnimation_];
	}
	// 縦画像
	if (moveDir_.y != 0.0f) {
		if (moveDir_.y > 0.0f) {
			isYRev = true;
		}
		img = imgsY_[cntAnimation_];
	}
	// 画像の描画
	DrawBillboard3D(
			pos_, 0.5f, 0.5f, IMG_SCALE, 0.0f, img, true, isXRev, isYRev);
	
	// スプライトアニメーションカウンタ
	cntAnimation_++;
	if (cntAnimation_ >= NUM_SPRITE_ALL)
	{
		isAlive_ = false;
	}

#ifdef _DEBUG
	// デバッグ用衝突判定
	//DrawSphere3D(pos_, collisionRadius_, 10, 0x0000ff, 0x0000ff, false);
#endif // _DEBUG
}

void WeaponPunch::Release(void)
{
	// エフェクト画像のメモリの解放
	for (int i = 0; i < NUM_SPRITE_ALL; i++)
	{
		DeleteGraph(imgsX_[i]);
		DeleteGraph(imgsY_[i]);
	}
}

void WeaponPunch::Use(VECTOR pos, VECTOR dir)
{
	// 武器の高さ調整
	pos_ = VAdd(pos, localPos_);
	moveDir_ = dir;
	isAlive_ = true;
	cntAnimation_ = 0;

}

void WeaponPunch::Load(void)
{
	// エフェクト画像(横)のロード
	LoadDivGraph(
		(Application::PATH_IMAGE + "DrillX.png").c_str(),
		NUM_SPRITE_ALL,
		NUM_SPRITE_X, NUM_SPRITE_Y,
		SIZE_XSPRITE_X, SIZE_XSPRITE_Y,
		imgsX_);

	// エフェクト画像(横)のロード
	LoadDivGraph(
			(Application::PATH_IMAGE + "DrillY.png").c_str(),
			NUM_SPRITE_ALL,
			NUM_SPRITE_X, NUM_SPRITE_Y,
			SIZE_YSPRITE_X, SIZE_YSPRITE_Y,
			imgsY_);
}

void WeaponPunch::SetParam(void)
{
	// モデルの大きさ
	scales_ = { 1.0f, 1.0f, 1.0f };
	// 移動スピード
	speed_ = 10.0f;
	// 衝突判定用半径
	collisionRadius_ = 100.0f;
	// 使用時の位置調整
	localPos_ = { 0.0f, 90.0f, 0.0f };
	// アニメーショカウンタ初期化
	cntAnimation_ = 0;


	// モデルをセット
	MV1SetPosition(modelId_, pos_);
}
