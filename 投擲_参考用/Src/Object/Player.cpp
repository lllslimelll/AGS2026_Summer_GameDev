#include <DxLib.h>
#include <string>
#include "../Utility/AsoUtility.h"
#include "../Application.h"
#include "../Manager/SceneManager.h"
#include "../Manager/InputManager.h"
#include "../Scene/GameScene.h"
#include "Common/AnimationController.h"
#include "Camera.h"
#include "./Weapon/WeaponPunch.h"
#include "./Weapon/WeaponAxe.h"
#include "Player.h"

Player::Player(void)
{
}

Player::~Player(void)
{
}

void Player::Init(GameScene* gameScene)
{
	// ゲームシーンの機能を使えるようにする
	gameScene_ = gameScene;

	// モデルの初期化
	modelId_ = MV1LoadModel("Data/Model/Player/player.mv1");

	// 初期状態
	state_ = STATE::STANDBY;

	MV1SetPosition(modelId_, DEFAULT_POS);

	MV1SetScale(modelId_, SCALES);

	// モデルアニメーション制御の初期化
	animationController_ = new AnimationController(modelId_);
	for (int i = 0; i < static_cast<int>(ANIM_TYPE::MAX); i++)
	{
		animationController_->AddInFbx(i, 30.0f, i);
	}

	// 追加テスト
	//animationController_->Add(0, 30.0f, Application::PATH_MODEL + "Player/Player.fbx");

	// 初期アニメーション再生
	animationController_->Play(static_cast<int>(ANIM_TYPE::IDLE), true);

	// ジャンプ力の初期化
	jumpPow_ = 0.0f;
	// ジャンプフラグ初期化
	isJump_ = false;

	// HPの初期化
	hp_ = MAX_HP;

	//武器の初期化
	weaponPunch_ = new WeaponPunch();
	weaponPunch_->Init(WeaponBase::TYPE::PUNCH);

	weaponAxe_ = new WeaponAxe();
	weaponAxe_->Init(WeaponBase::TYPE::AXE);

	// 初期武器はパンチ
	//useWeapon_ = weaponPunch_;
	useWeapon_ = weaponAxe_;

	// カメラに自分自身を渡す
	SceneManager::GetInstance().GetCamera()->SetFollow(this);
	// 敵に自分自身を渡す
	//gameScene_->GetEnemyBase()->LookPlayer(this);
}

void Player::Update(void)
{
	switch (state_)
	{
	case STATE::STANDBY:
		UpdateStanby();
		break;
	case STATE::KNOCKBACK:  
		UpdateKnockBack(); 
		break;
	case STATE::ATTACK:     
		UpdateAttack();    
		break;
	case STATE::DEAD:       
		UpdateDead();      
		break;
	case STATE::END:        
		UpdateEnd();       
		break;
	case STATE::VICTORY:   
		UpdateVictory();   
		break;
	}

	// 使用武器
	useWeapon_->Update();

	// アニメーションの更新
	animationController_->Update();

	// リスポーン処理
	Respawn();

}

void Player::Draw(void)
{
	switch (state_)
	{
	case STATE::STANDBY:
		DrawStanby();
		break;
	case STATE::KNOCKBACK:
		DrawKnockBack();
		break;
	case STATE::ATTACK:
		DrawAttack();
		break;
	case STATE::DEAD:
		DrawDead();
		break;
	case STATE::END:
		DrawEnd();
		break;
	case STATE::VICTORY:
		DrawVictory();
		break;
	}

	// 使用武器
	useWeapon_->Draw();

	// モデル描画
	MV1DrawModel(modelId_);

	// アニメーションの描画
	animationController_->Draw();

#ifdef _DEBUG
	DrawFormatString(0, 0, 0xffffff, "playerPos:(%.2f, %.2f, %.2f)", pos_.x, pos_.y, pos_.z);

	// フラグの表示
	DrawFormatString(0, 340, 0xffffff, "isJump:%d", isJump_);

	DrawFormatString(0, 180, 0xffffff, "jumpPow:%.2f", jumpPow_);

	DrawFormatString(0, 220, 0xffffff, "state:%d", state_);
#endif _DEBUG
}

void Player::Release(void)
{
	MV1DeleteModel(modelId_);

	// アニメーションコントローラの解放
	animationController_->Release();
	delete animationController_;

	// 武器の解放
	weaponPunch_->Release();
	delete weaponPunch_;

	// 使用武器
	useWeapon_ = nullptr;

}

void Player::ProcessMove(void)
{
	InputManager& ins = InputManager::GetInstance();

	// カメラの移動
	// 移動方向を決める
	VECTOR moveDir = AsoUtility::VECTOR_ZERO;
	if (ins.IsNew(KEY_INPUT_W)) { moveDir = VAdd(moveDir, AsoUtility::DIR_F);}
	if (ins.IsNew(KEY_INPUT_S)) { moveDir = VAdd(moveDir, AsoUtility::DIR_B);}
	if (ins.IsNew(KEY_INPUT_D)) { moveDir = VAdd(moveDir, AsoUtility::DIR_R);}
	if (ins.IsNew(KEY_INPUT_A)) { moveDir = VAdd(moveDir, AsoUtility::DIR_L);}

	// 移動処理
	// 動いていなかったら
	if (AsoUtility::EqualsVZero(moveDir))
	{
		// 待機アニメーション
		animationController_->Play(static_cast<int>(ANIM_TYPE::IDLE), true);
	}
	// 動いていたら
	else 
	{
		// 移動方法を更新
		moveDir_ = moveDir;

		// 移動方向を正規化する
		moveDir = VNorm(moveDir);
		// 移動量を計算する（移動 * スピード）
		VECTOR movePow = VScale(moveDir, SPEED_MOVE);
		// 移動処理（座標＋移動量)
		pos_ = VAdd(pos_, movePow);

		// 方向から角度(ラジアン）に変換する
		angles_.y = atan2(moveDir.x, moveDir.z);
		// モデルの方向が生の不の方向を向いてるので、補正する
		angles_.y += AsoUtility::Deg2RadF(180.0f);

		// アニメーションの再生
		// 歩行アニメーション
		animationController_->Play(static_cast<int>(ANIM_TYPE::WALK), true);
	}

	MV1SetPosition(modelId_, pos_);
	MV1SetRotationXYZ(modelId_, angles_);
}

void Player::CollisionStage(VECTOR pos)
{
	// 衝突判定に指定座標に押し戻す
	pos_ = pos;
	jumpPow_ = 0.0f;
	isJump_ = false;
}

void Player::ProcessJump(void)
{
	InputManager& ins = InputManager::GetInstance();

	// ジャンプ判定
	if (ins.IsTrgDown(KEY_INPUT_SPACE) && !isJump_)
	{
		isJump_ = true;
		jumpPow_ = JUMP_POW;

		// ジャンプアニメーション再生
		animationController_->Play(static_cast<int>(ANIM_TYPE::JUMP), false);
	}

	// 重力(加速度を速度に加算していく)
	jumpPow_ -= GRAVITY_POW;
	pos_.y += jumpPow_;

	// 落下中かジャンプ中なら、空中にいると見なす
	if (jumpPow_ < 0.0f && !isJump_) 
	{
		isJump_ = true;
	}
}

void Player::ProcessAttack(void)
{
	InputManager& ins = InputManager::GetInstance();

	// 重力（加速度）を速度に加算）
	jumpPow_ -= GRAVITY_POW;

	// 攻撃判定
	if (ins.IsTrgDown(KEY_INPUT_K) && !useWeapon_->IsAlive())// && !isJump_) 
	{
		// 攻撃遷移
		ChangeState(STATE::ATTACK);
	}

	// 武器切り替え
	if (ins.IsTrgDown(KEY_INPUT_J) && !useWeapon_->IsAlive())// && !isJump_) 
	{
		// 現在の武器種
		WeaponBase::TYPE type = useWeapon_->GetType();
		int typeInt = static_cast<int>(type);

		// 次の武器へ切り替え
		typeInt++;
		type = static_cast<WeaponBase::TYPE>(typeInt);

		if (type == WeaponBase::TYPE::MAX)
		{
			// 武器種を最初の武器に戻す
			type = WeaponBase::TYPE::PUNCH;
		}

		// 使用武器を切り替える
		switch (type)
		{
		case WeaponBase::TYPE::PUNCH:
			useWeapon_ = weaponPunch_;
			break;
		case WeaponBase::TYPE::AXE:
			useWeapon_ = weaponAxe_;
			break;
		}
	}
}

// 状態遷移
void Player::ChangeState(STATE state)
{
	state_ = state;

	switch (state_)
	{
	case STATE::STANDBY:
		ChangeStanby();
		break;
	case STATE::KNOCKBACK:
		ChangeKnockBack();
		break;
	case STATE::ATTACK:
		ChangeAttack();
		break;
	case STATE::DEAD:
		ChangeDead();
		break;
	case STATE::END:
		ChangeEnd();
		break;
	case STATE::VICTORY:
		ChangeVictory();
		break;
	}
}

void Player::ChangeStanby(void)
{
}

void Player::ChangeKnockBack(void)
{
	// ジャンプ判定にする
	isJump_ = true;

	// ノックバックカウンタリセット
	cntKnockBack_ = 0;
}

void Player::ChangeAttack(void)
{
	// 武器を使用
	useWeapon_->Use(pos_, moveDir_);

	// 攻撃アニメーション
	animationController_->Play(static_cast<int>(ANIM_TYPE::PUNCH), false);
}

void Player::ChangeDead(void)
{
	animationController_->Play(
		static_cast<int>(ANIM_TYPE::DEATH), false);
}

void Player::ChangeEnd(void)
{
}

void Player::ChangeVictory(void)
{
	// モデルにカメラ方向を向かせる
	angles_.y = AsoUtility::Deg2RadF(0.0f);
	MV1SetRotationXYZ(modelId_, angles_);

	// 勝利アニメーション
	animationController_->Play(static_cast<int>(ANIM_TYPE::WAVE), true);
}

void Player::UpdateStanby(void)
{
	// 移動処理
	ProcessMove();

	// ジャンプ
	ProcessJump();

	// 攻撃処理
	ProcessAttack();
}

void Player::UpdateKnockBack(void)
{
	// ノックバックカウントを進める
	cntKnockBack_++;

	// ジャンプする
	jumpPow_ -= GRAVITY_POW;
	pos_.y += jumpPow_;

	// ノックバック方向に移動させる
	VECTOR movePow = VScale(knockBackDir_, SPEED_KNOCKBACK);
	pos_ = VAdd(pos_, movePow);
	// 座標にモデルをセット
	MV1SetPosition(modelId_, pos_);


	// 着地したら通常状態に戻す
	if (!isJump_)
	{
		ChangeState(STATE::STANDBY);
		return;
	}

	// リスポーン判定
	Respawn();
}

void Player::UpdateAttack(void)
{
	// 攻撃アニメーションが終わったら、STANDBY状態にする
	if (animationController_->IsEnd()) {
		ChangeState(STATE::STANDBY);
	}

	// 
}

void Player::UpdateDead(void)
{
	// 撃破アニメーションが終わったら、END状態にする
	if (animationController_->IsEnd()) {
		ChangeState(STATE::END);
	}
}

void Player::UpdateEnd(void)
{
}

void Player::UpdateVictory(void)
{
}

void Player::DrawStanby(void)
{
}

void Player::DrawKnockBack(void)
{
	// 点滅処理
	if ((cntKnockBack_ / TERM_BLINK) % 2 == 0)
	{
		MV1SetMaterialDifColor(modelId_, 0, COLOR_DIF_BLINK);  // 明るくする
	}
	else
	{
		MV1SetMaterialDifColor(modelId_, 0, COLOR_DIF_DEFAULT);  // 通常色
	}

	// 地面についたら
	if (!isJump_)
	{
		MV1SetMaterialDifColor(modelId_, 0, COLOR_DIF_DEFAULT);  // 通常色
	}

}

void Player::DrawAttack(void)
{
}

void Player::DrawDead(void)
{
}

void Player::DrawEnd(void)
{
}

void Player::DrawVictory(void)
{
}

void Player::Respawn()
{
	if (pos_.y < RESPAWN_LEN)
	{
		pos_ = RESPAWN_POS;

		jumpPow_ = 0.0f;

		ChangeState(STATE::STANDBY);
	}
	MV1SetPosition(modelId_, pos_);
}

void Player::KnockBack(VECTOR dirXZ, float jumpPow)
{
	// 引数をメンバ変数に格納
	knockBackDir_ = dirXZ;
	jumpPow_ = jumpPow;

	// ノックバックに状態遷移
	ChangeState(STATE::KNOCKBACK);
}

bool Player::IsInvincible(void)
{
	return state_ == STATE::KNOCKBACK
		|| state_ == STATE::DEAD
		|| state_ == STATE::END;
}

bool Player::IsStateEnd(void)
{
	return state_ == STATE::END;
}

void Player::Damage(int damage)
{
	hp_ -= damage;
	if (hp_ < 0)
	{
		hp_ = 0;
	}

	// hpが０になったら死亡状態に
	if (hp_ == 0) {
		ChangeState(STATE::DEAD);
	}
}

int Player::GetHp(void)
{
	return hp_;
}

VECTOR Player::GetPos(void) const
{
	return pos_;
}

void Player::SetPos(VECTOR pos)
{
	pos_ = pos;
}

WeaponBase* Player::GetUseWeapon(void) const
{
	return useWeapon_;
}

void Player::Victory(void)
{
	ChangeState(STATE::VICTORY);
}



