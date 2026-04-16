#include <DxLib.h>
#include <EffekseerForDXLib.h>
#include "../Utility/AsoUtility.h"
#include "../Manager/InputManager.h"
#include "../Object/Common/Transform.h"
#include "../Object/Collider/ColliderBase.h"
#include "../Object/Collider/ColliderModel.h"
#include "../Object/Collider/ColliderSphere.h"
#include "Camera.h"

Camera::Camera(void)
	:
	followTransform_(nullptr),
	mode_(MODE::NONE),
	angles_(AsoUtility::VECTOR_ZERO),
	rotY_(Quaternion::Identity()),
	targetPos_(AsoUtility::VECTOR_ZERO)
{
	// DxLibの初期設定では、
	// カメラの位置が x = 320.0f, y = 240.0f, z = (画面のサイズによって変化)、
	// 注視点の位置は x = 320.0f, y = 240.0f, z = 1.0f
	// カメラの上方向は x = 0.0f, y = 1.0f, z = 0.0f
	// 右上位置からZ軸のプラス方向を見るようなカメラ
}

Camera::~Camera(void)
{
}

void Camera::Update(void)
{
}

void Camera::SetBeforeDraw(void)
{

	// クリップ距離を設定する(SetDrawScreenでリセットされる)
	SetCameraNearFar(VIEW_NEAR, VIEW_FAR);

	// 更新前情報
	prePos_ = transform_.pos;

	switch (mode_)
	{
	case Camera::MODE::FIXED_POINT:
		SetBeforeDrawFixedPoint();
		break;
	case Camera::MODE::FREE:
		SetBeforeDrawFree();
		break;
	case Camera::MODE::FOLLOW:
		SetBeforeDrawFollow();
		break;
	}

	// カメラの設定(位置と注視点による制御)
	SetCameraPositionAndTargetAndUpVec(
		transform_.pos, 
		targetPos_, 
		transform_.quaRot.GetUp()
	);

	// DXライブラリのカメラとEffekseerのカメラを同期する。
	Effekseer_Sync3DSetting();

}

void Camera::DrawDebug(void)
{
}

void Camera::Release(void)
{
}

void Camera::SetFollow(const Transform* follow)
{
	followTransform_ = follow;
}

void Camera::InitCollider(void)
{
	// 主にステージとの衝突で使用する球体コライダ
	ColliderSphere* colliderSphere = new ColliderSphere(
		ColliderBase::TAG::CAMERA,
		&transform_,
		AsoUtility::VECTOR_ZERO,
		COL_CAPSULE_SPHERE
	);
	// 自分自身の衝突情報に登録
	ownColliders_.emplace(
		static_cast<int>(COLLIDER_TYPE::SPHERE), colliderSphere);
}

void Camera::InitPost(void)
{
	ChangeMode(MODE::FIXED_POINT);
}

const VECTOR& Camera::GetPos(void) const
{
	return transform_.pos;
}

const VECTOR& Camera::GetAngles(void) const
{
	return angles_;
}

const VECTOR& Camera::GetTargetPos(void) const
{
	return targetPos_;
}

const Quaternion& Camera::GetQuaRot(void) const
{
	return transform_.quaRot;
}

const Quaternion& Camera::GetQuaRotY(void) const
{
	return rotY_;
}

VECTOR Camera::GetForward(void) const
{
	return VNorm(VSub(targetPos_, transform_.pos));
}

void Camera::ChangeMode(MODE mode)
{

	// カメラの初期設定
	SetDefault();

	// カメラモードの変更
	mode_ = mode;

	// 変更時の初期化処理
	switch (mode_)
	{
	case Camera::MODE::FIXED_POINT:
		break;
	case Camera::MODE::FREE:
		break;
	case Camera::MODE::FOLLOW:
		break;
	}

}

void Camera::SetDefault(void)
{

	// カメラの初期設定
	transform_.pos = DERFAULT_POS;

	// カメラ角
	angles_ = DERFAULT_ANGLES;
	transform_.quaRot = Quaternion::Identity();

	// 注視点
	targetPos_ = AsoUtility::VECTOR_ZERO;

}

void Camera::SyncFollow(void)
{

	//// 同期先の位置
	//VECTOR pos = followTransform_->pos;

	//// Y軸
	//rotY_ = Quaternion::AngleAxis(angles_.y, AsoUtility::AXIS_Y);

	//// Y軸 + X軸
	//transform_.quaRot = rotY_.Mult(Quaternion::AngleAxis(angles_.x, AsoUtility::AXIS_X));

	//VECTOR localPos;

	//// 注視点
	//localPos = transform_.quaRot.PosAxis(FOLLOW_TARGET_LOCAL_POS);
	//targetPos_ = VAdd(pos, localPos);

	//// カメラ位置
	//localPos = transform_.quaRot.PosAxis(FOLLOW_CAMERA_LOCAL_POS);
	//transform_.pos = VAdd(pos, localPos);


	// 1. プレイヤーの現在の座標を取得
	VECTOR playerPos = followTransform_->pos;

	// 2. 星の中心からプレイヤーへの方向を、カメラにとっての「真上(Up)」とする
	// ※MOON_CENTER_POS が定義されていればそちらを使ってください
	VECTOR upDir = VNorm(VSub(playerPos, MOON_CENTER_POS));

	// 3. プレイヤーの「前」と「真上」から、カメラの基準となる回転を作る
	// ★修正ポイント：playerRot を直接掛け算するのをやめました！（無限ループ防止）
	Quaternion baseRot = Quaternion::LookRotation(followTransform_->GetForward(), upDir);

	// 4. マウスやスティックの操作分(angles_)の回転を作る
	Quaternion rotY = Quaternion::AngleAxis(angles_.y, AsoUtility::AXIS_Y); // 左右
	Quaternion rotX = Quaternion::AngleAxis(angles_.x, AsoUtility::AXIS_X); // 上下

	// 5. 基準の回転に、操作分の回転を重ねて最終的な向きを決定
	transform_.quaRot = baseRot.Mult(rotY).Mult(rotX);

	// 6. 「後ろ・上」のオフセット位置を計算
	VECTOR cameraOffset = transform_.quaRot.PosAxis(FOLLOW_CAMERA_LOCAL_POS);
	transform_.pos = VAdd(playerPos, cameraOffset);

	// 7. 注視点（どこを見るか）を計算
	VECTOR targetOffset = transform_.quaRot.PosAxis(FOLLOW_TARGET_LOCAL_POS);
	targetPos_ = VAdd(playerPos, targetOffset);

	// ==========================================
	// ★ 超重要：勝手に回り続けるのを防ぐ「魔法の1行」
	// 左右の回転はプレイヤーの体が回ることで吸収されるため、
	// カメラ側のオフセットはここで 0 にリセットします。
	// ==========================================
	angles_.y = 0.0f;
}

void Camera::ProcessRot(bool isLimit)
{

	if (GetJoypadNum() == 0)
	{
		// 方向回転によるXYZの移動(マウス)
		RotMouse(isLimit);
	}
	else
	{
		// 方向回転によるXYZの移動(ゲームパッド)
		RotGamePad(isLimit);
	}

}

void Camera::ProcessMove(void)
{

	auto& ins = InputManager::GetInstance();

	VECTOR moveDir = AsoUtility::VECTOR_ZERO;

	if (GetJoypadNum() == 0)
	{
		if (ins.IsNew(KEY_INPUT_W)) { moveDir = AsoUtility::DIR_F; }
		if (ins.IsNew(KEY_INPUT_S)) { moveDir = AsoUtility::DIR_B; }
		if (ins.IsNew(KEY_INPUT_A)) { moveDir = AsoUtility::DIR_L; }
		if (ins.IsNew(KEY_INPUT_D)) { moveDir = AsoUtility::DIR_R; }
	}
	else
	{

		InputManager::JOYPAD_IN_STATE padState =
			ins.GetJPadInputState(InputManager::JOYPAD_NO::PAD1);

		// 左スティックの傾き
		moveDir = ins.GetDirectionXZAKey(padState.AKeyLX, padState.AKeyLY);

	}

	// 移動処理
	if (!AsoUtility::EqualsVZero(moveDir))
	{

		// 移動させたい方向(ベクトル)に変換

		// 現在の向きからの進行方向を取得
		VECTOR direction = VNorm(transform_.quaRot.PosAxis(moveDir));

		// 移動させたい方向に移動量をかける(=移動量)
		VECTOR movePow = VScale(direction, SPEED);

		// カメラ位置も注視点も移動させる
		transform_.pos = VAdd(transform_.pos, movePow);
		targetPos_ = VAdd(targetPos_, movePow);

	}

}

void Camera::SetBeforeDrawFixedPoint(void)
{
	// 何もしない
}

void Camera::SetBeforeDrawFree(void)
{

	// カメラ操作(回転)
	ProcessRot(false);
	
	// カメラ操作(移動)
	ProcessMove();

	// Y軸
	rotY_ = Quaternion::AngleAxis(angles_.y, AsoUtility::AXIS_Y);

	// Y軸 + X軸
	transform_.quaRot = rotY_.Mult(Quaternion::AngleAxis(angles_.x, AsoUtility::AXIS_X));

	// 注視点更新
	targetPos_ = VAdd(transform_.pos, transform_.quaRot.PosAxis(FOLLOW_TARGET_LOCAL_POS));

}

void Camera::SetBeforeDrawFollow(void)
{

	// カメラ操作(回転)
	ProcessRot(true);

	// 追従対象との相対位置を同期
	SyncFollow();

	// 衝突判定
	Collision();

	// カメラ位置の線形補完
	/*transform_.pos =
		AsoUtility::Lerp(prePos_, transform_.pos, 0.25f);*/

}

void Camera::Collision(void)
{
	// プレイヤーのルートフレーム
	VECTOR start = MV1GetFramePosition(followTransform_->modelId, 1);

	for (const auto& hitCol : hitColliders_)
	{
		// モデル以外は処理を飛ばす
		if (hitCol->GetShape() != ColliderBase::SHAPE::MODEL) continue;

		// 派生クラスへキャスト
		const ColliderModel* colliderModel =
			dynamic_cast<const ColliderModel*>(hitCol);

		if (colliderModel == nullptr) continue;

		// 線分とモデルの最近接(startに近い)衝突ポリゴンを取得
		auto hitPoly = colliderModel->GetNearestHitPolyLine(
			start, transform_.pos,
			false, //ブラックリストを使うか否か
			true   // ホワイトリストを使うか否か
		);

		// 線分が衝突していなければ次のコライダへ
		if (!hitPoly.HitFlag) continue;

		// カメラ位置から注視点への方向
		VECTOR dirToTarget = VNorm(VSub(targetPos_, transform_.pos));

		// 衝突点の少し手前にカメラを置く
		transform_.pos =
			VAdd(hitPoly.HitPosition, VScale(dirToTarget, COLLISION_BACK_DIS));

#pragma region 球体の衝突で線分衝突の補助として押し戻す

		// カメラ位置の球体コライダ
		int typeSphere = static_cast<int>(COLLIDER_TYPE::SPHERE);
		// 球体コライダがなければ処理を抜ける
		if (ownColliders_.count(typeSphere) == 0) continue;

		//// 球体コライダ情報
		//ColliderSphere* colliderSpehre =
		//	dynamic_cast<ColliderSphere*>(ownColliders_.at(typeSphere));
		//if (colliderSpehre == nullptr) return;

		// 指定された回数と距離で三角形の法線方向に押し戻す
		transform_.pos =
			ownColliders_.at(typeSphere)->GetPosPushBackAlongNormal(
				hitPoly, CNT_TRY_COLLISION_CAMERA, COLLISION_BACK_DIS);

#pragma endregion
	}
}

void Camera::ResetOpacityFrame(void)
{
	/*for (const auto& opacy : opacityFrames_)
	{
		MV1SetOpacityRate(opacy.modelId, )
	}*/
}

void Camera::RotKeyboard(bool isLimit)
{

	const auto& ins = InputManager::GetInstance();

	// カメラ回転
	if (ins.IsNew(KEY_INPUT_RIGHT))
	{
		// 右回転
		angles_.y += ROT_POW_RAD;
	}
	if (ins.IsNew(KEY_INPUT_LEFT))
	{
		// 左回転
		angles_.y -= ROT_POW_RAD;
	}

	// 上回転
	if (ins.IsNew(KEY_INPUT_UP))
	{
		angles_.x -= ROT_POW_RAD;
		if (isLimit && angles_.x < -LIMIT_X_DW_RAD)
		{
			angles_.x = -LIMIT_X_DW_RAD;
		}
	}

	// 下回転
	if (ins.IsNew(KEY_INPUT_DOWN))
	{
		angles_.x += ROT_POW_RAD;
		if (isLimit && angles_.x > LIMIT_X_UP_RAD)
		{
			angles_.x = LIMIT_X_UP_RAD;
		}
	}

}

void Camera::RotMouse(bool isLimit)
{
	// マウスの感度（この数値をいじってカメラの回転速度を調整します）
	const float SENSITIVITY = 0.00005f;

	// 画面のサイズを取得して、中心の座標を計算
	int screenW, screenH;
	GetDrawScreenSize(&screenW, &screenH);
	int centerX = screenW / 2;
	int centerY = screenH / 2;

	// 現在のマウス座標を取得
	int mouseX, mouseY;
	GetMousePoint(&mouseX, &mouseY);

	// 画面中心からの移動量を計算
	int deltaX = mouseX - centerX;
	int deltaY = mouseY - centerY;

	// 移動量に感度を掛けて、カメラの角度に足し込む
	angles_.y += deltaX * SENSITIVITY; // 左右回転
	angles_.x += deltaY * SENSITIVITY; // 上下回転

	// 角度制限（上下を見すぎないようにする）
	if (isLimit)
	{
		if (angles_.x < -LIMIT_X_DW_RAD) angles_.x = -LIMIT_X_DW_RAD;
		if (angles_.x > LIMIT_X_UP_RAD) angles_.x = LIMIT_X_UP_RAD;
	}

	// マウスカーソルを画面の中心に戻す（端にぶつからないようにするため）
	SetMousePoint(centerX, centerY);
}

void Camera::RotGamePad(bool isLimit)
{

	auto& ins = InputManager::GetInstance();

	// 接続されているゲームパッド１の情報を取得
	InputManager::JOYPAD_IN_STATE padState =
		ins.GetJPadInputState(InputManager::JOYPAD_NO::PAD1);

	// 右スティックの傾き
	VECTOR dir = ins.GetDirectionXZAKey(padState.AKeyRX, padState.AKeyRY);

	// 右スティック左右の傾き
	angles_.y += dir.x * ROT_POW_RAD;

	// 右スティック上下の傾き
	angles_.x -= dir.z * ROT_POW_RAD;
	
	// 角度制限
	if (isLimit && angles_.x < -LIMIT_X_DW_RAD)
	{
		angles_.x = -LIMIT_X_DW_RAD;
	}
	if (isLimit && angles_.x > LIMIT_X_UP_RAD)
	{
		angles_.x = LIMIT_X_UP_RAD;
	}

}
