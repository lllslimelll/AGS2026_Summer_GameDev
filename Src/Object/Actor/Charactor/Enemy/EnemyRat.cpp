#include "../../../../Application.h"
#include "../../../../Utility/AsoUtility.h"
#include "../../../../Manager/ResourceManager.h"
#include "../../../Common/AnimationController.h"
#include "../../../Collider/ColliderLine.h"
#include "../../../Collider/ColliderCapsule.h"
#include "../../../../Scene/SceneManager.h"
#include "EnemyRat.h"


EnemyRat::EnemyRat(const EnemyBase::EnemyData& data, Player* player)
	:
	EnemyBase(data, player),
	step_(0.0f)
{
}

EnemyRat::~EnemyRat(void)
{
}

void EnemyRat::InitLoad(void)
{
	// 基底クラスのリソースロード
	EnemyBase::InitLoad();

	// モデルのロード
	transform_.SetModel(
		resMng_.LoadModelDuplicate(ResourceManager::SRC::ENEMY_RAT));
}

void EnemyRat::InitTransform(void)
{
	// モデルの基本設定
	// 大きさ
	transform_.scl = { SCALE, SCALE, SCALE };

	// モデル本来の向き
	transform_.quaRot = Quaternion::Identity();
	// ローカル回転
	transform_.quaRotLocal = Quaternion::Euler(ROT);

	VECTOR upVec = VNorm((VSub(transform_.pos, { 0,0,0 })));
	transform_.pos = VAdd(transform_.pos, VScale(upVec, 28));

	transform_.Update();
}

void EnemyRat::InitCollider(void)
{
	// 主に地面との衝突で使用する線分コライダ
	ColliderLine* colLine = new ColliderLine(
		ColliderBase::TAG::ENEMY, &transform_,
		COL_LINE_START_LOCAL_POS, COL_LINE_END_LOCAL_POS);

	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::GROUND_LINE), colLine);

	// 主に壁や木などの衝突で使用するカプセルコライダ
	ColliderCapsule* colCapsule = new ColliderCapsule(
		ColliderBase::TAG::ENEMY, &transform_,
		COL_CAPSULE_TOP_LOCAL_POS, COL_CAPSULE_DOWN_LOCAL_POS, COL_CAPSULE_RADIUS);

	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::CAPSULE), colCapsule);
}

void EnemyRat::InitAnimation(void)
{
	// アニメーションコントローラークラス生成
	animCtrl_ = new AnimationController(transform_.modelId);

	int type = -1;

	// 待機
	type = static_cast<int>(ANIM_TYPE::IDLE);
	animCtrl_->AddInFbx(type, 20.0f, type);

	// 歩く
	type = static_cast<int>(ANIM_TYPE::WALK);
	animCtrl_->AddInFbx(type, 30.0, type);

	// アニメ再生
	animCtrl_->Play(static_cast<int>(ANIM_TYPE::IDLE), true);
}

void EnemyRat::InitPost(void)
{
	// 状態遷移処理のバインドを登録
	stateChanges_.emplace(static_cast<int>(STATE::NONE),
		std::bind(&EnemyRat::ChangeStateNone, this));
	stateChanges_.emplace(static_cast<int>(STATE::THINK),
		std::bind(&EnemyRat::ChangeStateThink, this));
	stateChanges_.emplace(static_cast<int>(STATE::IDLE),
		std::bind(&EnemyRat::ChangeStateIdle, this));
	stateChanges_.emplace(static_cast<int>(STATE::WANDER),
		std::bind(&EnemyRat::ChangeStateWander, this));
	stateChanges_.emplace(static_cast<int>(STATE::END),
		std::bind(&EnemyRat::ChangeStateEnd, this));

	// 初期状態設定
	ChangeState(STATE::THINK);
}

void EnemyRat::UpdateProcess(void)
{
	// 状態別更新
	stateUpdate_();
}

void EnemyRat::UpdateProcessPost(void)
{
	EnemyBase::UpdateProcessPost();
	
	if (!InMovableRange())
	{
		// 移動可能範囲外に出たら、移動前座標に戻す
		transform_.pos = prevPos_;
		transform_.Update();

		// 思考状態に戻す
		ChangeState(STATE::THINK);
	}
}

void EnemyRat::ChangeState(STATE state)
{
	state_ = state;

	// 状態遷移の初期処理
	CharactorBase::ChangeState(static_cast<int>(state_));
}

void EnemyRat::ChangeStateNone(void)
{
	stateUpdate_ = std::bind(&EnemyRat::UpdateNone, this);
}

void EnemyRat::ChangeStateThink(void)
{
	stateUpdate_ = std::bind(&EnemyRat::UpdateThink, this);

	// 思考
	// ランダムに次の行動を決定
	// ３０％で待機、７０％で徘徊
	int rand = GetRand(100);
	if (rand < 30)
	{
		ChangeState(STATE::IDLE);
	}
	else
	{
		ChangeState(STATE::WANDER);
	}
}

void EnemyRat::ChangeStateIdle(void)
{
	stateUpdate_ = std::bind(&EnemyRat::UpdateIdle, this);

	// ランダムな待機時間
	step_ = 3.0f + static_cast<float>(GetRand(3));

	// 移動量ゼロ
	movePow_ = AsoUtility::VECTOR_ZERO;

	// 待機アニメーション再生
	animCtrl_->Play(
		static_cast<int>(ANIM_TYPE::IDLE), true);
}

void EnemyRat::ChangeStateWander(void)
{
	//stateUpdate_ = std::bind(&EnemyRat::UpdateWander, this);

	//// ランダムな角度
	//float angle = static_cast<float>(GetRand(360)) * DX_PI_F / 180.0f;

	//// 移動方向
	//moveDir_ = VGet(cosf(angle), 0.0f, sinf(angle));

	//// ランダムな移動時間
	//step_ = 2.0f + static_cast<float>(GetRand(5));

	//// 移動スピード
	//moveSpeed_ = 3.0f;

	//// 歩きアニメーション再生
	//animCtrl_->Play(
	//	static_cast<int>(ANIM_TYPE::WALK), true);

	stateUpdate_ = std::bind(&EnemyRat::UpdateWander, this);

	// 月中心からの上方向（地面の法線）
	VECTOR upDir = VNorm(VSub(transform_.pos, MOON_CENTER_POS));

	// 接平面上の基底ベクトルを2本作る
	// upDir とほぼ平行な参照軸を避けて外積を取る
	VECTOR ref = (fabsf(upDir.y) < 0.99f)
		? AsoUtility::AXIS_Y
		: AsoUtility::AXIS_X;
	VECTOR tangent = VNorm(VCross(upDir, ref));     // 接ベクトル1
	VECTOR binormal = VNorm(VCross(upDir, tangent)); // 接ベクトル2

	// 接平面上でランダム角度の方向を合成
	float angle = static_cast<float>(GetRand(360)) * DX_PI_F / 180.0f;
	moveDir_ = VAdd(
		VScale(tangent, cosf(angle)),
		VScale(binormal, sinf(angle)));

	// ランダムな移動時間
	step_ = 2.0f + static_cast<float>(GetRand(5));

	// 移動スピード
	moveSpeed_ = 3.0f;

	// 歩きアニメーション再生
	animCtrl_->Play(static_cast<int>(ANIM_TYPE::WALK), true);
}

void EnemyRat::ChangeStateEnd(void)
{
	stateUpdate_ = std::bind(&EnemyRat::UpdateEnd, this);
}

void EnemyRat::UpdateNone(void)
{
}

void EnemyRat::UpdateThink(void)
{
}

void EnemyRat::UpdateIdle(void)
{
	step_ -= scnMng_.GetDeltaTime();
	if (step_ < 0.0f)
	{
		// 待機終了
		ChangeState(STATE::THINK);
		return;
	}
}

void EnemyRat::UpdateWander(void)
{
	step_ -= scnMng_.GetDeltaTime();
	if (step_ < 0.0f)
	{
		// 移動終了
		ChangeState(STATE::THINK);
		
		return;
	}

	// 現在位置の法線に対して moveDir_ を接平面に再投影（前回追加した補正）
	VECTOR upDir = VNorm(VSub(transform_.pos, MOON_CENTER_POS));
	float d = VDot(moveDir_, upDir);
	moveDir_ = VNorm(VSub(moveDir_, VScale(upDir, d)));

	// 進行方向を向く
	faceDir_ = moveDir_;

	// 移動量
	movePow_ = VScale(moveDir_, moveSpeed_);
}

void EnemyRat::UpdateEnd(void)
{
}
