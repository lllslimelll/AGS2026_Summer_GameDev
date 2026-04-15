#include "../../../Object/Common/AnimationController.h"
#include "../../../Manager/SceneManager.h"
#include "../../../Manager/Camera.h"
#include "../../../Manager/InputManager.h"
#include "../../../Manager/ResourceManager.h"
#include "../../Common/Transform.h"
#include "../../../Utility/AsoUtility.h"
#include "../../Collider/ColliderLine.h"
#include "../../Collider/ColliderCapsule.h"
#include "Player.h"


Player::Player(void)
	:
	CharactorBase()
{
}

Player::~Player(void)
{
}

void Player::ProcessMove(void)
{
	auto& ins = InputManager::GetInstance();

	// 移動量
	movePow_ = AsoUtility::VECTOR_ZERO;

	// 移動方向
	VECTOR dir = AsoUtility::VECTOR_ZERO;

	// ダッシュボタン
	bool isDash = false;

	// ゲームパッドが接続数で処理を分ける
	if (GetJoypadNum() == 0)
	{
		// WASDで移動する
		if (ins.IsNew(KEY_INPUT_W)) { dir = AsoUtility::DIR_F; }
		if (ins.IsNew(KEY_INPUT_A)) { dir = AsoUtility::DIR_L; }
		if (ins.IsNew(KEY_INPUT_S)) { dir = AsoUtility::DIR_B; }
		if (ins.IsNew(KEY_INPUT_D)) { dir = AsoUtility::DIR_R; }
		// 右Shiftでダッシュ
		if (ins.IsNew(KEY_INPUT_LSHIFT)) { isDash = true; }
	}
	else
	{
		// ゲームパッド操作
		// 接続されているゲームパッド１の情報を取得
		InputManager::JOYPAD_IN_STATE padState =
			ins.GetJPadInputState(InputManager::JOYPAD_NO::PAD1);
		// アナログキーの入力値から方向を取得
		dir = ins.GetDirectionXZAKey(padState.AKeyLX, padState.AKeyLY);

		// 右Shiftでダッシュ
		if (ins.IsPadBtnNew(
			InputManager::JOYPAD_NO::PAD1,
			InputManager::JOYPAD_BTN::R_TRIGGER)) {
			isDash = true;
		}
	}

	if (!AsoUtility::EqualsVZero(dir))
	{

		// 移動スピード
		moveSpeed_ = SPEED_MOVE;
		if (isDash)
		{
			moveSpeed_ = SPEED_MOVE;
		}

		// ジャンプ中はアニメーションを変えない
		if (!isJump_)
		{
			if (isDash)
			{
				// ダッシュ速度
				moveSpeed_ = SPEED_DASH;

				// 速く走るアニメーション再生
				animController_->Play(static_cast<int>(ANIM_TYPE::FAST_RUN), true);
			}
			else
			{
				// 走るアニメーション再生
				animController_->Play(static_cast<int>(ANIM_TYPE::RUN), true);
			}
		}

		// Y軸のみのカメラ角度を取得
		Quaternion cameraRot = scnMng_.GetCamera()->GetQuaRotY();
		 
		// 移動方向をカメラに合わせる
		moveDir_ = Quaternion::PosAxis(cameraRot, dir);

		// 移動速度を反映
		movePow_ = VScale(moveDir_, moveSpeed_);
	}
	else
	{
		// ジャンプ中はアニメーションを変えない
		if (!isJump_)
		{
			animController_->Play(static_cast<int>(ANIM_TYPE::IDLE), true);
		}
	}
}

void Player::ProcessJump(void)
{
	auto& ins = InputManager::GetInstance();

	// プレイヤーの上方向を計算（地面の法線方向）
	VECTOR upDir = VNorm(VSub(transform_.pos, MOON_CENTER_POS));

	// ジャンプキーが押されたか
	bool isHitKey = ins.IsTrgDown(KEY_INPUT_BACKSLASH)
		|| ins.IsPadBtnTrgDown(
			InputManager::JOYPAD_NO::PAD1,
			InputManager::JOYPAD_BTN::DOWN);
	// ジャンプキーが押されているか
	bool isHitKeyNew = ins.IsNew(KEY_INPUT_BACKSLASH)
		|| ins.IsPadBtnTrgDown(
			InputManager::JOYPAD_NO::PAD1,
			InputManager::JOYPAD_BTN::DOWN);

	// 初期ジャンプ処理
	if (isHitKey && !isJump_)
	{
		// ジャンプ量の計算
		float jumpSpeed = POW_JUMP_INIT * scnMng_.GetDeltaTime();
		jumpPow_ = VScale(upDir, jumpSpeed);
		stepJump_ = 0.0f;
		isJump_ = true;

		// アニメーション再生
		animController_->Play(
			static_cast<int>(ANIM_TYPE::JUMP), false);
	}

	// 持続ジャンプ処理（長押し）
	if (isHitKeyNew)
	{
		// ジャンプの入力受付時間を減少
		stepJump_ += scnMng_.GetDeltaTime();
		if(stepJump_ < TIME_JUMP_INPUT)
		{
			// ジャンプ量の計算
			float jumpSpeed = POW_JUMP_KEEP * scnMng_.GetDeltaTime();
			jumpPow_ = VAdd(jumpPow_, VScale(upDir, jumpSpeed));
		}
	}
	else
	{
		// ボタンを離したらジャンプ力に加算しない
		stepJump_ = TIME_JUMP_INPUT;
	}
}

void Player::UpdateProcess(void)
{
	// 移動処理
	ProcessMove();

	// ジャンプ処理
	ProcessJump();

	// 衝突判定用の調整
	CollisionReserve();
}

void Player::UpdateProcessPost(void)
{
}

// 衝突判定用の調整
void Player::CollisionReserve(void)
{
	// アニメーションごとの線分調整
	if (animController_->GetPlayType() == static_cast<int>(ANIM_TYPE::JUMP))
	{
		// ジャンプ中は線分を伸ばす
		if (ownColliders_.count(static_cast<int>(COLLIDER_TYPE::GROUND_LINE)) != 0)
		{
			ColliderLine* colLine = dynamic_cast<ColliderLine*>(
				ownColliders_.at(static_cast<int>(COLLIDER_TYPE::GROUND_LINE)));
			colLine->SetLocalPosStart(COL_LINE_JUMP_START_LOCAL_POS);
			colLine->SetLocalPosEnd(COL_LINE_JUMP_END_LOCAL_POS);
		}
	}
	else
	{
		// 通常時の線分に戻す
		if (ownColliders_.count(static_cast<int>(COLLIDER_TYPE::GROUND_LINE)) != 0)
		{
			ColliderLine* colLine = dynamic_cast<ColliderLine*>(
				ownColliders_.at(static_cast<int>(COLLIDER_TYPE::GROUND_LINE)));
			colLine->SetLocalPosStart(COL_LINE_START_LOCAL_POS);
			colLine->SetLocalPosEnd(COL_LINE_END_LOCAL_POS);
		}
	}

	// アニメーションごとのカプセル調整
	if (animController_->GetPlayType() == static_cast<int>(ANIM_TYPE::JUMP))
	{
		// ジャンプ中はカプセルを伸ばす
		if (ownColliders_.count(static_cast<int>(COLLIDER_TYPE::CAPSULE)) != 0)
		{
			ColliderCapsule* colCapsule = dynamic_cast<ColliderCapsule*>(
				ownColliders_.at(static_cast<int>(COLLIDER_TYPE::CAPSULE)));

			colCapsule->SetLocalPosTop(COL_CAPSULE_TOP_JUMP_LOCAL_POS);
			colCapsule->SetLocalPosDown(COL_CAPSULE_DOWN_JUMP_LOCAL_POS);
		}
	}
	else
	{
		// 通常時のカプセルに戻す
		if (ownColliders_.count(static_cast<int>(COLLIDER_TYPE::CAPSULE)) != 0)
		{
			ColliderCapsule* colCapsule = dynamic_cast<ColliderCapsule*>(
				ownColliders_.at(static_cast<int>(COLLIDER_TYPE::CAPSULE)));

			colCapsule->SetLocalPosTop(COL_CAPSULE_TOP_LOCAL_POS);
			colCapsule->SetLocalPosDown(COL_CAPSULE_DOWN_LOCAL_POS);
		}
	}
}

void Player::InitLoad(void)
{
	// 基底クラスのリソースロード
	CharactorBase::InitLoad();

	// モデル読み込み
	transform_.SetModel(resMng_.Load(			// 1個 = Load()  複数 = Depulicate()
		ResourceManager::SRC::PLAYER).handleId_);
}

void Player::InitTransform(void)
{
	// 大きさ
	transform_.scl = AsoUtility::VECTOR_ONE;
	
	transform_.quaRot = Quaternion::Identity();
	// Y軸を180度
	transform_.quaRotLocal = Quaternion::AngleAxis(AsoUtility::Deg2RadF(180.0f), AsoUtility::AXIS_Y);

	// 座標
	transform_.pos = VAdd(AsoUtility::VECTOR_ZERO, VScale(AsoUtility::DIR_U, 2600.0f));//64.0f));

	transform_.Update();

}

void Player::InitCollider(void)
{
	// 主に地面との衝突で使用する線分コライダ
	ColliderLine* colLine = new ColliderLine(
		ColliderBase::TAG::PLAYER, &transform_,
		COL_LINE_START_LOCAL_POS, COL_LINE_END_LOCAL_POS);

	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::GROUND_LINE), colLine);

	// 主に壁や木などの衝突で使用するカプセルコライダ
	ColliderCapsule* colCapsule = new ColliderCapsule(
		ColliderBase::TAG::PLAYER, &transform_,
		COL_CAPSULE_TOP_LOCAL_POS, COL_CAPSULE_DOWN_LOCAL_POS, COL_CAPSULE_RADIUS);

	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::CAPSULE), colCapsule);
}

void Player::InitAnimation(void)
{
	// アニメーション
	animController_ = new AnimationController(transform_.modelId);
	animController_->Add(static_cast<int>(ANIM_TYPE::IDLE), 20.0f, resMng_.Load(ResourceManager::SRC::IDLE).path_);

	animController_->Add(static_cast<int>(ANIM_TYPE::RUN), 20.0f, resMng_.Load(ResourceManager::SRC::RUN).path_);

	animController_->Add(static_cast<int>(ANIM_TYPE::FAST_RUN), 20.0f, resMng_.Load(ResourceManager::SRC::FAST_RUN).path_);

	animController_->Add(static_cast<int>(ANIM_TYPE::JUMP), 20.0f, resMng_.Load(ResourceManager::SRC::JUMP_RISING).path_);
}

void Player::InitPost(void)
{
	transform_.Update();

	animController_->Play(0, true);
}

