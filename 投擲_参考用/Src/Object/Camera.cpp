#include <DxLib.h>
#include "../Manager/InputManager.h"
#include "../Utility/AsoUtility.h"
#include "../Object/Player.h"
#include "Camera.h"

Camera::Camera(void)
{
}

Camera::~Camera(void)
{
}

void Camera::Init(void)
{
	// カメラ位置
	pos_ = DEFAULT_POS;
	// カメラ角度
	angles_ = DEFAULT_ANGLES;
}

void Camera::Update(void)
{
	InputManager& ins = InputManager::GetInstance();

	if (player_ != nullptr)
	{
		// プレイヤーの座標を取得
		VECTOR playerPos = player_->GetPos();
		pos_.x = playerPos.x;
		pos_.z = playerPos.z - 1000.0f;
	}

	//// X軸の角度調整
	//float anglePowRad = SPEED_ANGLE_DEG * DX_PI_F / 180;
	//if (ins.IsNew(KEY_INPUT_UP)) { angles_.x -= anglePowRad; }
	//if (ins.IsNew(KEY_INPUT_DOWN)) { angles_.x += anglePowRad; }
	//if (ins.IsNew(KEY_INPUT_LEFT)) { angles_.y -= anglePowRad; }
	//if (ins.IsNew(KEY_INPUT_RIGHT)) { angles_.y += anglePowRad; }

	//// カメラの移動
	//// 移動方向を決める
	//VECTOR moveDir = AsoUtility::VECTOR_ZERO;
	//if (ins.IsNew(KEY_INPUT_T)) { moveDir = AsoUtility::DIR_F; }
	//if (ins.IsNew(KEY_INPUT_G)) { moveDir = AsoUtility::DIR_B; }
	//if (ins.IsNew(KEY_INPUT_H)) { moveDir = AsoUtility::DIR_R; }
	//if (ins.IsNew(KEY_INPUT_F)) { moveDir = AsoUtility::DIR_L; }
	//if (ins.IsNew(KEY_INPUT_P)) { moveDir = AsoUtility::DIR_U; }
	//if (ins.IsNew(KEY_INPUT_L)) { moveDir = AsoUtility::DIR_D; }

	//// 移動量を計算する
	//VECTOR movePow = VScale(moveDir, SPEED_MOVE);

	//// 移動処理（座標＋移動量)
	//pos_ = VAdd(pos_, movePow);
}

void Camera::SetBeforeDraw(void)
{
	// クリップ距離を設定
	// （SetDrawScreenでリセットされる）
	SetCameraNearFar(CAMERA_NEAR, CAMERA_FAR);

	// カメラの設定
	SetCameraPositionAndAngle(pos_, angles_.x, angles_.y, angles_.z);
}

void Camera::DrawDebug(void)
{
	DrawFormatString(0, 0, 0xffffff, "カメラ座標：(%.2f, %.2f, %2f)", pos_.x, pos_.y, pos_.z);

	DrawFormatString(0, 60, 0xffffff, "カメラ角度:(%.2f, %.2f, %.2f)", angles_.x, angles_.y, angles_.z);
}

void Camera::Release(void)
{
}

void Camera::SetFollow(Player* player)
{
	player_ = player;
}
