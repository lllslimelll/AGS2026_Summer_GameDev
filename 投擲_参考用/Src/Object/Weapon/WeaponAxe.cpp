#include "../../Application.h"
#include "../../Utility/AsoUtility.h"
#include "WeaponAxe.h"

WeaponAxe::WeaponAxe(void)
{
}

WeaponAxe::~WeaponAxe(void)
{
}

void WeaponAxe::Draw(void)
{
	if (!isAlive_)
	{
		return;
	}
	// モデル描画
	MV1DrawModel(modelId_);

#ifdef _DEBUG
	// デバッグ用衝突判定
	//DrawSphere3D(pos_, collisionRadius_, 10, 0x0000ff, 0x0000ff, false);
#endif // _DEBUG
}

void WeaponAxe::Release(void)
{
	MV1DeleteModel(modelId_);
}

void WeaponAxe::Use(VECTOR pos, VECTOR dir)
{
	// 武器の高さ調整
	pos_ = VAdd(pos, localPos_);

	// 移動方向
	moveDir_ = dir;

	// 武器使用 = 生存
	isAlive_ = true;

	// 初期のジャンプ力を設定
	jumpPow_ = JUMP_POW;

	// 斧の角度調整
	// 方向から角度(ラジアン）に変換する
	angles_.y = atan2(moveDir_.x, moveDir_.z);

	// 補正する
	angles_.x = 0.0f;
	angles_.y += AsoUtility::Deg2RadF(90.0f);
	angles_.z = 0.0f;

	MV1SetRotationXYZ(modelId_, angles_);
}

void WeaponAxe::Load(void)
{
	modelId_ = MV1LoadModel(
		"Data/Model/Weapon/Axe_Wood.mv1");
}

void WeaponAxe::SetParam(void)
{
	// モデルの大きさ
	scales_ = { 0.5f, 0.5f, 0.5f };
	// 移動スピード
	speed_ = 15.0f;
	// 衝突判定用半径
	collisionRadius_ = 60.0f;
	// 使用時の位置調整
	localPos_ = { 0.0f, 90.0f, 0.0f };	
}

void WeaponAxe::Move(void)
{
	// 移動処理(一方方向)
	pos_ = VAdd(pos_, VScale(moveDir_, speed_));

	// ジャンプ・重力処理
	jumpPow_ -= GRAVITY;
	pos_.y += jumpPow_;

	// 地面に落下したら武器をなくす
	if (pos_.y < 0.0f)
	{
		isAlive_ = false;
	}

	// モデルをセット
	MV1SetPosition(modelId_, pos_);
}
