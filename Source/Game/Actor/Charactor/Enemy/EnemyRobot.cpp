#include "../../../../Utility/AsoUtility.h"
#include "../../../../Manager/ResourceManager.h"
#include "../../../Scene/SceneManager.h"
#include "../../../../Common/AnimationController.h"
#include "../../../../Collision/ColliderLine.h"
#include "../../../../Collision/ColliderCapsule.h"
#include "../../../../Collision/ColliderModel.h"
#include "EnemyRobot.h"

EnemyRobot::EnemyRobot(const EnemyBase::EnemyData& data, Player& player)
	:
	EnemyBase(data, player),
	state_(STATE::NONE),
	step_(0.0f),
	wayPoints_(),
	activeWayPointIndex_(0),
	nextWayPoint_(AsoUtility::VECTOR_ZERO)
{
}

EnemyRobot::~EnemyRobot(void)
{
}

void EnemyRobot::InitLoad(void)
{
	// 基底クラスのリソースロード
	//CharactorBase::InitLoad();

	// モデルのロード
	transform_.SetModel(
		resMng_.LoadModelDuplicate(ResourceManager::SRC::ENEMY_ROBOT));
}

void EnemyRobot::InitTransform(void)
{
	transform_.scl = VScale(AsoUtility::VECTOR_ONE, SCALE);
	transform_.quaRot = Quaternion();
	transform_.quaRotLocal = Quaternion::Euler(DEFAULT_LOCAL_ROT);
	transform_.Update();
}

void EnemyRobot::InitCollider(void)
{
	ColliderLine* colLine = new ColliderLine(
		CollisionProfileType::PAWN,
		this,
		COL_LINE_START_LOCAL_POS,
		COL_LINE_END_LOCAL_POS);
	RegisterCollider(colLine, static_cast<int>(COLLIDER_TYPE::GROUND_LINE));

	ColliderCapsule* colCapsule = new ColliderCapsule(
		CollisionProfileType::PAWN,
		this,
		COL_CAPSULE_TOP_LOCAL_POS,
		COL_CAPSULE_DOWN_LOCAL_POS,
		COL_CAPSULE_RADIUS);
	RegisterCollider(colCapsule, static_cast<int>(COLLIDER_TYPE::CAPSULE));
}

void EnemyRobot::InitAnimation(void)
{
	animCtrl_ = new AnimationController(transform_.modelId);

	// FBX内のアニメーション設定
	int type = 01;

	type = static_cast<int>(ANIM_TYPE::DANCE);
	animCtrl_->AddInFbx(type, 10.0f, type);

	type = static_cast<int>(ANIM_TYPE::IDLE);
	animCtrl_->AddInFbx(type, 20.0f, type);

	type = static_cast<int>(ANIM_TYPE::WALK);
	animCtrl_->AddInFbx(type, 30.0f, type);

	type = static_cast<int>(ANIM_TYPE::RUN);
	animCtrl_->AddInFbx(type, 30.0f, type);

	type = static_cast<int>(ANIM_TYPE::KICK);
	animCtrl_->AddInFbx(type, 45.0f, type);

	type = static_cast<int>(ANIM_TYPE::SHOOT);
	animCtrl_->AddInFbx(type, 30.0f, type);

	// 初期アニメーション再生
	animCtrl_->Play(static_cast<int>(ANIM_TYPE::IDLE), true);
}

void EnemyRobot::InitPost(void)
{
	// 状態遷移初期処理登録
	stateChanges_.emplace(static_cast<int>(STATE::NONE),
		std::bind(&EnemyRobot::ChangeStateNone, this));

	stateChanges_.emplace(static_cast<int>(STATE::THINK),
		std::bind(&EnemyRobot::ChangeStateThink, this));

	stateChanges_.emplace(static_cast<int>(STATE::IDLE),
		std::bind(&EnemyRobot::ChangeStateIdle, this));

	stateChanges_.emplace(static_cast<int>(STATE::PATROL),
		std::bind(&EnemyRobot::ChangeStatePatrol, this));

	stateChanges_.emplace(static_cast<int>(STATE::SURPRISE),
		std::bind(&EnemyRobot::ChangeStateSurprise, this));

	stateChanges_.emplace(static_cast<int>(STATE::ALERT),
		std::bind(&EnemyRobot::ChangeStateAlert, this));

	stateChanges_.emplace(static_cast<int>(STATE::CHASE),
		std::bind(&EnemyRobot::ChangeStateChase, this));

	stateChanges_.emplace(static_cast<int>(STATE::ATTACK_KICK),
		std::bind(&EnemyRobot::ChangeStateAttackKick, this));

	stateChanges_.emplace(static_cast<int>(STATE::ATTACK_SHOOT),
		std::bind(&EnemyRobot::ChangeStateAttackShoot, this));

	stateChanges_.emplace(static_cast<int>(STATE::ESCAPE),
		std::bind(&EnemyRobot::ChangeStateEscape, this));

	stateChanges_.emplace(static_cast<int>(STATE::DEAD),
		std::bind(&EnemyRobot::ChangeStateDead, this));

	stateChanges_.emplace(static_cast<int>(STATE::KNOCKBACK),
		std::bind(&EnemyRobot::ChangeStateKnockBack, this));

	stateChanges_.emplace(static_cast<int>(STATE::END),
		std::bind(&EnemyRobot::ChangeStateEnd, this));

	// 巡回ルート
	wayPoints_.emplace_back(VGet(1926.18f, 151.76f, 618.34f));
	wayPoints_.emplace_back(VGet(1274.85f, 101.35f, 168.02f));
	wayPoints_.emplace_back(VGet(1400.34f, 114.21f, -457.11f));
	wayPoints_.emplace_back(VGet(2553.30f, 141.82f, -593.32f));

	// 初期状態設定
	ChangeState(STATE::THINK);
}

void EnemyRobot::UpdateProcess(void)
{
	// 状態別更新
	stateUpdate_();
}

void EnemyRobot::UpdateProcessPost(void)
{
	EnemyBase::UpdateProcessPost();
}

void EnemyRobot::Draw(void)
{
	// 基底クラスの描画処理
	CharactorBase::Draw();

#ifdef _DEBUG

	// 巡回ルート描画
	for (const auto& point : wayPoints_)
	{
		DrawSphere3D(
			point, 30.0f, 10,
			0x0000ff, 0x0000ff, false);
	}

#endif // _DEBUG

}

void EnemyRobot::ChangeState(STATE state)
{
	state_ = state;
	CharactorBase::ChangeState(static_cast<int>(state_));
}

void EnemyRobot::ChangeStateNone(void)
{
	stateUpdate_ = std::bind(&EnemyRobot::UpdateNone, this);
}

void EnemyRobot::ChangeStateThink(void)
{
	stateUpdate_ = std::bind(&EnemyRobot::UpdateThink, this);

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
		ChangeState(STATE::PATROL);
	}
}

void EnemyRobot::ChangeStateIdle(void)
{
	stateUpdate_ = std::bind(&EnemyRobot::UpdateIdle, this);

	// ランダムな待機時間
	step_ = 3.0f + static_cast<float>(GetRand(3));

	// 移動量ゼロ
	movePow_ = AsoUtility::VECTOR_ZERO;

	// 待機アニメーション再生
	animCtrl_->Play(
		static_cast<int>(ANIM_TYPE::IDLE), true);
}

void EnemyRobot::ChangeStatePatrol(void)
{
	stateUpdate_ = std::bind(&EnemyRobot::UpdatePatrol, this);

	// 移動量ゼロ
	movePow_ = AsoUtility::VECTOR_ZERO;

	if (activeWayPointIndex_ == wayPoints_.size())
	{
		// 巡回終了
		activeWayPointIndex_ = 0;
		ChangeState(STATE::THINK);
		return;
	}

	// 次の巡回ポイント更新
	nextWayPoint_ = wayPoints_[activeWayPointIndex_];
	
	// 巡回ルートの移動方向を設定
	SetMoveDirPatrol();

	// 移動スピード
	moveSpeed_ = 5.0f;

	// 歩きアニメーション再生
	animCtrl_->Play(
		static_cast<int>(ANIM_TYPE::WALK), true);
}

void EnemyRobot::ChangeStateSurprise(void)
{
	stateUpdate_ = std::bind(&EnemyRobot::UpdateSurprise, this);
}

void EnemyRobot::ChangeStateAlert(void)
{
	stateUpdate_ = std::bind(&EnemyRobot::UpdateAlert, this);

	// 移動量ゼロ
	movePow_ = AsoUtility::VECTOR_ZERO;

	// ダンス（足踏み）アニメーション再生
	animCtrl_->Play(
		static_cast<int>(ANIM_TYPE::DANCE), true);
}

void EnemyRobot::ChangeStateChase(void)
{
	stateUpdate_ = std::bind(&EnemyRobot::UpdateChase, this);
}

void EnemyRobot::ChangeStateAttackKick(void)
{
	stateUpdate_ = std::bind(&EnemyRobot::UpdateAttackKick, this);
}

void EnemyRobot::ChangeStateAttackShoot(void)
{
	stateUpdate_ = std::bind(&EnemyRobot::UpdateAttackShoot, this);
}

void EnemyRobot::ChangeStateEscape(void)
{
	stateUpdate_ = std::bind(&EnemyRobot::UpdateEscape, this);
}

void EnemyRobot::ChangeStateDead(void)
{
	stateUpdate_ = std::bind(&EnemyRobot::UpdateDead, this);
}

void EnemyRobot::ChangeStateKnockBack(void)
{
	stateUpdate_ = std::bind(&EnemyRobot::UpdateKnockBack, this);
}

void EnemyRobot::ChangeStateEnd(void)
{
	stateUpdate_ = std::bind(&EnemyRobot::UpdateEnd, this);
}

void EnemyRobot::UpdateNone(void)
{
}

void EnemyRobot::UpdateThink(void)
{
}

void EnemyRobot::UpdateIdle(void)
{
	step_ -= scnMng_.GetDeltaTime();
	if (step_ < 0.0f)
	{
		// 待機終了
		ChangeState(STATE::THINK);
		return;
	}

	// 索敵処理
	if (InSearchCone())
	{
		// プレイヤーを発見
		ChangeState(STATE::ALERT);
	}
}

void EnemyRobot::UpdatePatrol(void)
{
	// 巡回ポイントとの球体衝突判定（半径３０．０ｆくらい）
	if (AsoUtility::IsHitSphere(transform_.pos, nextWayPoint_, 80.0f))
	{
		// 巡回ポイントインデックス更新
		activeWayPointIndex_++;

		// 次の移動地点へか、待機か思考
		ChangeState(STATE::THINK);
		return;
	}


	// 巡回ルートの移動方向を設定
	SetMoveDirPatrol();

	// 移動量の計算
	movePow_ = VScale(moveDir_, moveSpeed_);

	// 索敵処理
	if (InSearchCone())
	{
		// プレイヤーを発見
		ChangeState(STATE::ALERT);
	}
}

void EnemyRobot::UpdateSurprise(void)
{
}

void EnemyRobot::UpdateAlert(void)
{
}

void EnemyRobot::UpdateChase(void)
{
}

void EnemyRobot::UpdateAttackKick(void)
{
}

void EnemyRobot::UpdateAttackShoot(void)
{
}

void EnemyRobot::UpdateEscape(void)
{
}

void EnemyRobot::UpdateDead(void)
{
}

void EnemyRobot::UpdateKnockBack(void)
{
}

void EnemyRobot::UpdateEnd(void)
{
}

void EnemyRobot::SetMoveDirPatrol(void)
{
	// 巡回先座標XZ
	VECTOR tmpPos = nextWayPoint_;
	tmpPos.y = 0.0f;

	// 現在地座標XZ
	VECTOR pos = transform_.pos;
	pos.y = 0.0f;

	// XZ平面上の移動方向を計算
	moveDir_ = VNorm(VSub(tmpPos, pos));
}

bool EnemyRobot::InSearchCone(void)
{
	return EnemyBase::InSearchCone(VIEW_RANGE_PATROL, VIEW_HALF_FOV_PATROL);
}