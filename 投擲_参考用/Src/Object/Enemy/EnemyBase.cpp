#include <DxLib.h>
#include "../Player.h"
#include "../../Manager/SceneManager.h"
#include "../../Manager/BlockManager.h"
#include "../Shot/ShotStraight.h"
#include "../Shot/ShotBase.h"
#include "../../Utility/AsoUtility.h"
#include "../Common/AnimationController.h"
#include "EnemyBase.h"

EnemyBase::EnemyBase(void)
{
}

EnemyBase::~EnemyBase(void)
{
}

void EnemyBase::Init(TYPE type,
	int baseModelId, int baseAttackEffectModelId, Player* player)
{
	// 敵種別
	type_ = type;

	// ゲームシーン内のplayerを取得
	player_ = player;

	modelId_ = MV1DuplicateModel(baseModelId);

	// エフェクトモデルID
	baseAttackEffectModelId_ = baseAttackEffectModelId;

	// モデルアニメーション制御の初期化
	animationController_ = new AnimationController(modelId_);
	for (int i = 0; i < static_cast<int>(ANIM_TYPE::MAX); i++)
	{
		animationController_->AddInFbx(i, 30.0f, i);
	}

	// 初期状態
	state_ = STATE::STANDBY;
	// 初期アニメーション再生
	animationController_->Play(static_cast<int>(ANIM_TYPE::IDLE), true);

	pos_ = DEFAULT_POS;
	angle_ = DEFAULT_ANGLE;

	// パラメータ設定
	SetParam();

	// 出現位置設定
	SetSpawnPos();

	MV1SetScale(modelId_, DEFAULT_SCALES);
	MV1SetMaterialEmiColor(modelId_, 0, COLOR_EMI_DEFAULT);
}

// パラメータ設定
void EnemyBase::SetParam()
{
}


void EnemyBase::Update(void)
{
	switch (state_)
	{
	case EnemyBase::STATE::STANDBY:
		UpdateStandby();
		break;
	case EnemyBase::STATE::ATTACK:
		UpdateAttack();
		break;
	case EnemyBase::STATE::HIT_REACT:
		UpdateHitReact();
		break;
	case EnemyBase::STATE::DEAD_REACT:
		UpdateDeadReact();
		break;
	case EnemyBase::STATE::END:
		UpdateEnd();
		break;
	}


	// プルプル防止
	// 近すぎ過ぎたら移動停止
	if (dist_ < MOVE_SPEED)
	{
		return;
	}

	UpdateShot();

	// モデルに角度を反映
	MV1SetRotationXYZ(modelId_, angle_);
	// モデルを座標にセット
	MV1SetPosition(modelId_, pos_);

	// アニメーションの更新
	animationController_->Update();
}

void EnemyBase::Draw(void)
{
	MV1DrawModel(modelId_);

	// アニメーションの描画
	animationController_->Draw();

	switch (state_)
	{
	case EnemyBase::STATE::STANDBY:
		DrawStandby();
		break;
	case EnemyBase::STATE::ATTACK:
		DrawAttack();
		break;
	case EnemyBase::STATE::HIT_REACT:
		DrawHitReact();
		break;
	case EnemyBase::STATE::DEAD_REACT:
		DrawDeadReact();
		break;
	case EnemyBase::STATE::END:
		DrawEnd();
		break;
	}

	DrawShot();


	const int DIV = 64;               // 分割数（精度）
	const float RADIUS1 = 1000.0f;     // 半径
	const float RADIUS2= 800.0f;     // 半径
	const VECTOR CENTER = VGet(0.0f, 0.0f, 0.0f); // 円の中心
	const float Y = 0.1f;             // 地面より少し浮かせる

	for (int i = 0; i < DIV; ++i)
	{
		float angle1 = 2.0f * DX_PI * i / DIV;
		float angle2 = 2.0f * DX_PI * (i + 1) / DIV;

		VECTOR p1 = VGet(cosf(angle1) * RADIUS1, Y, sinf(angle1) * RADIUS1);
		VECTOR p2 = VGet(cosf(angle2) * RADIUS1, Y, sinf(angle2) * RADIUS1);
		
		VECTOR a1 = VGet(cosf(angle1) * RADIUS2, Y, sinf(angle1) * RADIUS2);
		VECTOR a2 = VGet(cosf(angle2) * RADIUS2, Y, sinf(angle2) * RADIUS2);

		// 線で円周を描画
		DrawLine3D(VAdd(CENTER, p1), VAdd(CENTER, p2), 0x00ff00);
		DrawLine3D(VAdd(CENTER, a1), VAdd(CENTER, a2), 0x00ff00);
	}
}

void EnemyBase::Release(void)
{
	MV1DeleteModel(modelId_);

	// アニメーションコントローラの解放
	animationController_->Release();
	delete animationController_;

	for (ShotBase* shot : shots_)
	{
		shot->Release();
	}
}

void EnemyBase::LookPlayer()
{
	// プレイヤーの座標を取得
	VECTOR pPos = player_->GetPos();

	// 移動方向を計算する（プレイヤー座標 - 敵座標）
	moveDir_ = VSub(pPos, pos_);

	// 移動方向のベクトルサイズを取得
	dist_ = VSize(moveDir_);

	// 移動方向を正規化する
	moveDir_ = VNorm(moveDir_);

	// 方向がゼロベクトルでない場合、向きを更新
	if (!AsoUtility::EqualsVZero(moveDir_))
	{
		// プレイヤーの方向を向くように角度を設定
		angle_.y = atan2(moveDir_.x, moveDir_.z);
		angle_.y += AsoUtility::Deg2RadF(180.0f);  // 補正
	}

}

void EnemyBase::Move()
{
	// 移動制限
	if (pos_.x <= -1000.0f || pos_.x >= 1000.0f
		|| pos_.z <= -1000.0f || pos_.z >= 1000.0f)
	{
		return;
	}

	// 移動方向がゼロベクトルでない場合
	if (!AsoUtility::EqualsVZero(moveDir_))
	{
		// 移動量を計算する（向き * スピード）
		VECTOR movePow = VScale(moveDir_, speed_);

		// Y軸固定
		pos_.y = 0.0f;

		// 移動処理（座標＋移動量)
		pos_ = VAdd(pos_, movePow);	// ←移動予定位置

		// アニメーションの再生
		// 歩行アニメーション
		animationController_->Play(static_cast<int>(ANIM_TYPE::WALK), true);

	}

}

// 敵の出現位置
void EnemyBase::SetSpawnPos()
{
	// ステージの中心座標
	VECTOR stageCentorPos = { 0.0f, 0.0f, 0.0f };

	// 0〜360度のランダム角度
	float angleDeg = GetRand(360.0f);  // 0〜360
	float angleRad = AsoUtility::Deg2RadF(angleDeg);	// ラジアンに変換
	
	// 角度から方向を取得
	VECTOR dir = { cosf(angleRad), 0.0f, sinf(angleRad) };

	// 出現位置を計算
	// 出現位置 = ステージ中心座標 + (方向 * 半径)
	pos_ = VAdd(stageCentorPos, VScale(dir, radius_));

	MV1SetPosition(modelId_, pos_);
	MV1SetRotationXYZ(modelId_, angle_);
}

void EnemyBase::ChangeState(STATE state)
{
	state_ = state;

	switch (state_)
	{
	case EnemyBase::STATE::STANDBY:
		ChangeStandby();
		break;
	case EnemyBase::STATE::ATTACK:
		ChangeAttack();
		break;
	case EnemyBase::STATE::HIT_REACT:
		ChangeHitReact();
		break;
	case EnemyBase::STATE::DEAD_REACT:
		ChangeDeadReact();
		break;
	case EnemyBase::STATE::END:
		ChangeEnd();
		break;
	}
}

VECTOR EnemyBase::GetPos(void)
{
	return pos_;
}

void EnemyBase::ChangeStandby(void)
{
	// 拡散光を標準地に戻す
	MV1SetMaterialDifColor(modelId_, 0, COLOR_DIF_DEFAULT);

	cntAttack_ = 0; // 待機状態に入ったらカウンターをリセット
	animationController_->Play(static_cast<int>(ANIM_TYPE::IDLE), true);
}

void EnemyBase::ChangeAttack(void)
{
	animationController_->Play(static_cast<int>(ANIM_TYPE::ATTACK), false);
	cntAttack_ = 0;

	// 有効な弾を取得する
	ShotBase* shot = GetValidShot();
	// 弾を生成
	shot->CreateShot(pos_, moveDir_);
}

void EnemyBase::ChangeHitReact(void)
{
	cntDamaged_ = CNT_HIT_REACT;

	animationController_->Play(static_cast<int>(ANIM_TYPE::HIT_REACT), false);
}

void EnemyBase::ChangeDeadReact(void)
{
	cntDamaged_ = CNT_DEAD_REACT;

	animationController_->Play(static_cast<int>(ANIM_TYPE::DEATH), false);

}

void EnemyBase::ChangeEnd(void)
{
}

void EnemyBase::UpdateStandby(void)
{
	// 追従対象を見る
	LookPlayer();

	// 移動処理
	Move();
}

void EnemyBase::UpdateAttack(void)
{
	cntAttack_++;

	// アニメーションが終わったら
	if (animationController_->IsEnd())
	{
		// 待機
		ChangeState(STATE::STANDBY);
	}
}

void EnemyBase::UpdateHitReact(void)
{
	cntDamaged_--;

	// ダメージカウンタが０未満になったら
	if (cntDamaged_ < 0)
	{
		// 待機状態に戻る
		ChangeState(STATE::STANDBY);
	}
}

void EnemyBase::UpdateDeadReact(void)
{
	cntDamaged_--;

	// ダメージカウンタが０未満になったら
	if (cntDamaged_ < 0 && animationController_->IsEnd())
	{
		// 待機状態に戻る
		ChangeState(STATE::END);
	}
}

void EnemyBase::UpdateEnd(void)
{
}

void EnemyBase::DrawStandby(void)
{
}

void EnemyBase::DrawAttack(void)
{
}

void EnemyBase::DrawHitReact(void)
{
	// 点滅処理
	if (cntDamaged_ % TERM_BLINK == 0)
	{
		MV1SetMaterialDifColor(modelId_, 0, COLOR_DIF_DEFAULT);  // 明るくする
	}
	else
	{
		MV1SetMaterialDifColor(modelId_, 0, COLOR_DIF_BLINK);  // 通常色
	}
	// モデルの描画
	MV1DrawModel(modelId_);
}

void EnemyBase::DrawDeadReact(void)
{
	// 点滅処理
	if (cntDamaged_ % TERM_BLINK == 0)
	{
		MV1SetMaterialDifColor(modelId_, 0, COLOR_DIF_DEFAULT);  
	}
	else
	{
		MV1SetMaterialDifColor(modelId_, 0, COLOR_DIF_BLINK); 
	}
	// モデルの描画
	MV1DrawModel(modelId_);  // 通常色
}

void EnemyBase::DrawEnd(void)
{
	MV1DeleteModel(modelId_);  // モデルを削除
}

void EnemyBase::Damage(int damage)
{
	hp_ -= damage;

	if (hp_ <= 0)
	{
		hp_ = 0;

		// 死亡状態に
		ChangeState(STATE::DEAD_REACT);
	}
	else if (hp_ == 1)
	{
		// 死亡
		ChangeState(STATE::HIT_REACT);
	}
}

bool EnemyBase::IsCollisionState(void)
{
	return state_ == STATE::STANDBY || state_ == STATE::ATTACK;
}

bool EnemyBase::IsAlive(void)
{
	return state_ != STATE::END;
}

float EnemyBase::GetCollisionRadius(void)
{
	return collisionRadius_;
}

const std::vector<ShotBase*>& EnemyBase::GetShots(void)
{
	return shots_;
}


void EnemyBase::UpdateShot(void)
{
	for (ShotBase* shot : shots_)
	{
		shot->Update();
	}
}

void EnemyBase::DrawShot(void)
{
	for (ShotBase* shot : shots_)
	{
		shot->Draw();
	}
}

ShotBase* EnemyBase::GetValidShot(void)
{
	size_t size = shots_.size();
	for (int i = 0; i < size; i++)
	{
		// 未使用(生存していない)
		if (!shots_[i]->IsAlive())
		{
			return shots_[i];
		}
	}
	// 新しい弾のインスタンスを生成する
	ShotBase* shot = new ShotStraight(
		ShotBase::TYPE::STRAIGHT, baseAttackEffectModelId_);

	// 可変長配列に追加
	shots_.push_back(shot);

	return shot;
}
