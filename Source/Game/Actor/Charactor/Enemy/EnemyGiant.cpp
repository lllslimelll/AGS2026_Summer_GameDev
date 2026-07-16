#include "../../../../Utility/AsoUtility.h"
#include "../../../../Manager/ResourceManager.h"
#include "../../../Scene/SceneManager.h"
#include "../../../../Common/AnimationController.h"
#include "../../../../Collision/ColliderLine.h"
#include "../../../../Collision/ColliderCapsule.h"
#include "../../../../Collision/ColliderSphere.h"
#include "../../../../Collision/ColliderModel.h"
#include "../../../../Collision/CollisionManager.h"
#include "../Player.h"
#include "EnemyGiant.h"

EnemyGiant::EnemyGiant(const EnemyBase::EnemyData& data, Player& player)
    :
    EnemyBase(data, player),
    state_(STATE::NONE),
    step_(0.0f),
    attackHit_(false)
{
}

EnemyGiant::~EnemyGiant(void)
{
}

void EnemyGiant::InitLoad(void)
{
    transform_.SetModel(
        resMng_.LoadModelDuplicate(ResourceManager::SRC::ENEMY_GIANT));
}

void EnemyGiant::InitTransform(void)
{
    transform_.scl = VScale(AsoUtility::VECTOR_ONE, SCALE);
    transform_.quaRot = Quaternion();
    transform_.quaRotLocal = Quaternion::Euler(DEFAULT_LOCAL_ROT);
    transform_.pos = defaultPos_;
    // 月面の法線方向に少し浮かせる（地面にめり込まないように）
    VECTOR upVec = VNorm(VSub(transform_.pos, MOON_CENTER_POS));
    transform_.pos = VAdd(transform_.pos, VScale(upVec, 50.0f));  // 値は調整が必要

    transform_.Update();
}

void EnemyGiant::InitCollider(void)
{
    // 地面衝突用線分
    ColliderLine* colLine = new ColliderLine(
        CollisionProfileType::PAWN,
        this,
        COL_LINE_START_LOCAL_POS,
        COL_LINE_END_LOCAL_POS);
    RegisterCollider(colLine, static_cast<int>(COLLIDER_TYPE::GROUND_LINE));

    // 本体カプセル
    ColliderCapsule* colCapsule = new ColliderCapsule(
        CollisionProfileType::PAWN,
        this,
        COL_CAPSULE_TOP_LOCAL_POS,
        COL_CAPSULE_DOWN_LOCAL_POS,
        COL_CAPSULE_RADIUS);
    RegisterCollider(colCapsule, static_cast<int>(COLLIDER_TYPE::CAPSULE));
}

void EnemyGiant::InitAnimation(void)
{
    animCtrl_ = new AnimationController(transform_.modelId);
    animCtrl_->SetBlendTime(0.2f);

    animCtrl_->Add(static_cast<int>(ANIM_TYPE::IDLE), 20.0f,
        resMng_.Load(ResourceManager::SRC::IDLE).path_);
    animCtrl_->Add(static_cast<int>(ANIM_TYPE::WALK), 25.0f,
        resMng_.Load(ResourceManager::SRC::RUN).path_);
    animCtrl_->Add(static_cast<int>(ANIM_TYPE::RUN), 30.0f,
        resMng_.Load(ResourceManager::SRC::FAST_RUN).path_);
    animCtrl_->Add(static_cast<int>(ANIM_TYPE::ATTACK), 200.0f,
        resMng_.Load(ResourceManager::SRC::ATTACK).path_);

    animCtrl_->Play(static_cast<int>(ANIM_TYPE::IDLE), true);
}

void EnemyGiant::InitPost(void)
{
    stateChanges_.emplace(static_cast<int>(STATE::NONE),
        std::bind(&EnemyGiant::ChangeStateNone, this));
    stateChanges_.emplace(static_cast<int>(STATE::THINK),
        std::bind(&EnemyGiant::ChangeStateThink, this));
    stateChanges_.emplace(static_cast<int>(STATE::IDLE),
        std::bind(&EnemyGiant::ChangeStateIdle, this));
    stateChanges_.emplace(static_cast<int>(STATE::PATROL),
        std::bind(&EnemyGiant::ChangeStatePatrol, this));
    stateChanges_.emplace(static_cast<int>(STATE::CHASE),
        std::bind(&EnemyGiant::ChangeStateChase, this));
    stateChanges_.emplace(static_cast<int>(STATE::ATTACK),
        std::bind(&EnemyGiant::ChangeStateAttack, this));
    stateChanges_.emplace(static_cast<int>(STATE::RETURN),
        std::bind(&EnemyGiant::ChangeStateReturn, this));
    stateChanges_.emplace(static_cast<int>(STATE::END),
        std::bind(&EnemyGiant::ChangeStateEnd, this));

    attackHandFrame_ = MV1SearchFrame(transform_.modelId, "mixamorig:LeftHand");
    ChangeState(STATE::THINK);
}

void EnemyGiant::UpdateProcess(void)
{
    stateUpdate_();
}

void EnemyGiant::UpdateProcessPost(void)
{
    EnemyBase::UpdateProcessPost();

    // 移動可能範囲外に出たら戻す
    if (state_ == STATE::PATROL && !InMovableRange())
    {
        transform_.pos = prevPos_;
        transform_.Update();
        ChangeState(STATE::THINK);
    }

    // プレイヤーとの押し戻し
    PushBackFromPlayer();
}

void EnemyGiant::Draw(void)
{
    CharactorBase::Draw();

#ifdef _DEBUG
    // 攻撃コライダの可視化
    VECTOR handPos = MV1GetFramePosition(transform_.modelId, attackHandFrame_);
    DrawSphere3D(handPos, ATTACK_SPHERE_RADIUS, 16, 0xff0000, 0xff0000, false);
#endif
}

// ===== 状態遷移 =====

void EnemyGiant::ChangeState(STATE state)
{
    state_ = state;
    CharactorBase::ChangeState(static_cast<int>(state_));
}

void EnemyGiant::ChangeStateNone(void)
{
    stateUpdate_ = std::bind(&EnemyGiant::UpdateNone, this);
}

void EnemyGiant::ChangeStateThink(void)
{
    stateUpdate_ = std::bind(&EnemyGiant::UpdateThink, this);

    int rand = GetRand(100);
    if (rand < 30) { ChangeState(STATE::IDLE); }
    else { ChangeState(STATE::PATROL); }
}

void EnemyGiant::ChangeStateIdle(void)
{
    stateUpdate_ = std::bind(&EnemyGiant::UpdateIdle, this);
    step_ = 2.0f + static_cast<float>(GetRand(3));
    movePow_ = AsoUtility::VECTOR_ZERO;
    animCtrl_->Play(static_cast<int>(ANIM_TYPE::IDLE), true);
}

void EnemyGiant::ChangeStatePatrol(void)
{
    stateUpdate_ = std::bind(&EnemyGiant::UpdatePatrol, this);
    movePow_ = AsoUtility::VECTOR_ZERO;

    // 接平面上でランダム方向
    VECTOR upDir = VNorm(VSub(transform_.pos, MOON_CENTER_POS));
    VECTOR ref = (fabsf(upDir.y) < 0.99f) ? AsoUtility::AXIS_Y : AsoUtility::AXIS_X;
    VECTOR tangent = VNorm(VCross(upDir, ref));
    VECTOR binormal = VNorm(VCross(upDir, tangent));

    float angle = static_cast<float>(GetRand(360)) * DX_PI_F / 180.0f;
    moveDir_ = VAdd(VScale(tangent, cosf(angle)), VScale(binormal, sinf(angle)));
    faceDir_ = moveDir_;

    step_ = 3.0f + static_cast<float>(GetRand(4));
    moveSpeed_ = SPEED_PATROL;
    animCtrl_->Play(static_cast<int>(ANIM_TYPE::WALK), true);
}

void EnemyGiant::ChangeStateChase(void)
{
    stateUpdate_ = std::bind(&EnemyGiant::UpdateChase, this);
    moveSpeed_ = SPEED_CHASE;
    animCtrl_->Play(static_cast<int>(ANIM_TYPE::RUN), true);
}

void EnemyGiant::ChangeStateAttack(void)
{
    stateUpdate_ = std::bind(&EnemyGiant::UpdateAttack, this);
    movePow_ = AsoUtility::VECTOR_ZERO;
    attackHit_ = false;
    animCtrl_->Play(static_cast<int>(ANIM_TYPE::ATTACK), false);
}

void EnemyGiant::ChangeStateReturn(void)
{
    stateUpdate_ = std::bind(&EnemyGiant::UpdateReturn, this);
    moveSpeed_ = SPEED_PATROL;
    animCtrl_->Play(static_cast<int>(ANIM_TYPE::WALK), true);
}

void EnemyGiant::ChangeStateEnd(void)
{
    stateUpdate_ = std::bind(&EnemyGiant::UpdateEnd, this);
    //isEnd_ = true;
}

// ===== 更新系 =====

void EnemyGiant::UpdateNone(void)
{
}

void EnemyGiant::UpdateThink(void)
{
}

void EnemyGiant::UpdateIdle(void)
{
    step_ -= scnMng_.GetDeltaTime();
    if (step_ < 0.0f)
    {
        ChangeState(STATE::THINK);
        return;
    }

    if (InSearchCone())
    {
        ChangeState(STATE::CHASE);
    }
}

void EnemyGiant::UpdatePatrol(void)
{
    step_ -= scnMng_.GetDeltaTime();
    if (step_ < 0.0f)
    {
        ChangeState(STATE::THINK);
        return;
    }

    // 接平面に再投影
    VECTOR upDir = VNorm(VSub(transform_.pos, MOON_CENTER_POS));
    float d = VDot(moveDir_, upDir);
    moveDir_ = VNorm(VSub(moveDir_, VScale(upDir, d)));
    faceDir_ = moveDir_;
    movePow_ = VScale(moveDir_, moveSpeed_);

    if (InSearchCone()) { ChangeState(STATE::CHASE); }
}

void EnemyGiant::UpdateChase(void)
{
    float dist = DistToPlayer();

    // 攻撃距離内に入ったら攻撃
    if (dist <= DIST_ATTACK)
    {
        ChangeState(STATE::ATTACK);
        return;
    }

    // 追跡解除距離を超えたら帰還
    if (dist > DIST_CHASE)
    {
        ChangeState(STATE::RETURN);
        return;
    }

    // プレイヤーに向かって移動
    SetMoveDirToTarget(player_.GetTransform().pos);
    movePow_ = VScale(moveDir_, moveSpeed_);
}

void EnemyGiant::UpdateAttack(void)
{
    if (animCtrl_->IsEnd())
    {
        float dist = DistToPlayer();
        if (dist <= DIST_ATTACK) { ChangeState(STATE::ATTACK); }
        else if (dist > DIST_CHASE) { ChangeState(STATE::RETURN); }
        else { ChangeState(STATE::CHASE); }
        return;
    }

    if (!attackHit_)
    {
        VECTOR handPos = MV1GetFramePosition(
            transform_.modelId, attackHandFrame_);

        // OVERLAP の結果からプレイヤーを探す
        auto overlapHits = CollisionManager::GetInstance().GetOverlapHits(this);
        for (const auto& hit : overlapHits)
        {
            if (hit.otherCollider->GetChannel() != CollisionChannel::PAWN) continue;

            // プレイヤーかどうか確認
            Player* player = dynamic_cast<Player*>(hit.otherActor);
            if (player == nullptr) continue;

            const ColliderCapsule* playerCapsule =
                dynamic_cast<const ColliderCapsule*>(hit.otherCollider);
            if (playerCapsule == nullptr) continue;

            if (AsoUtility::IsHitSphereCapsule(
                handPos, ATTACK_SPHERE_RADIUS,
                playerCapsule->GetPosTop(),
                playerCapsule->GetPosDown(),
                playerCapsule->GetRadius()))
            {
                player_.OnDamaged(static_cast<int>(ATTACK_DAMAGE));
                attackHit_ = true;
                break;
            }
        }
    }
}

void EnemyGiant::UpdateReturn(void)
{
    if (AsoUtility::IsHitSphere(transform_.pos, spawnPos_, 120.0f))
    {
        ChangeState(STATE::THINK);
        return;
    }

    movePow_ = VScale(moveDir_, moveSpeed_);

    // 帰還中もプレイヤーが近づいてきたら追いかける
    if (InSearchCone())
    {
        ChangeState(STATE::CHASE);
    }
}

void EnemyGiant::UpdateEnd(void)
{
}

// ===== ヘルパー =====

void EnemyGiant::SetMoveDirToTarget(const VECTOR& target)
{
    // 月面の上方向
    VECTOR upDir = VNorm(VSub(transform_.pos, MOON_CENTER_POS));

    // ターゲット方向を計算
    VECTOR dir = VNorm(VSub(target, transform_.pos));

    // 接平面に投影（上方向成分を除去）
    float d = VDot(dir, upDir);
    VECTOR projected = VSub(dir, VScale(upDir, d));

    // 投影後のベクトルが極端に小さい場合（目標がほぼ真上/真下）は無視
    if (VSize(projected) < 0.01f) return;

    moveDir_ = VNorm(projected);
    faceDir_ = moveDir_;
}

bool EnemyGiant::InSearchCone(void) const
{
    return EnemyBase::InSearchCone(VIEW_DIST, VIEW_HALF_FOV);
}

float EnemyGiant::DistToPlayer(void) const
{
    return VSize(VSub(player_.GetTransform().pos, transform_.pos));
}

void EnemyGiant::PushBackFromPlayer(void)
{
    int capsuleType = static_cast<int>(COLLIDER_TYPE::CAPSULE);
    if (ownColliders_.count(capsuleType) == 0) return;

    ColliderCapsule* myCapsule =
        dynamic_cast<ColliderCapsule*>(ownColliders_.at(capsuleType));
    if (myCapsule == nullptr) return;

    // BLOCK の結果からプレイヤーを探す
    auto blockHits = CollisionManager::GetInstance().GetBlockHits(this);
    for (const auto& hit : blockHits)
    {
        if (hit.otherCollider->GetChannel() != CollisionChannel::PAWN) continue;

        Player* player = dynamic_cast<Player*>(hit.otherActor);
        if (player == nullptr) continue;

        const ColliderCapsule* playerCapsule =
            dynamic_cast<const ColliderCapsule*>(hit.otherCollider);
        if (playerCapsule == nullptr) continue;

        VECTOR myCenter = VScale(
            VAdd(myCapsule->GetPosTop(), myCapsule->GetPosDown()), 0.5f);
        VECTOR plCenter = VScale(
            VAdd(playerCapsule->GetPosTop(), playerCapsule->GetPosDown()), 0.5f);

        float dist = VSize(VSub(myCenter, plCenter));
        float minDist = myCapsule->GetRadius() + playerCapsule->GetRadius();

        if (dist < minDist && dist > 0.01f)
        {
            VECTOR pushDir = VNorm(VSub(myCenter, plCenter));
            float  overlap = minDist - dist;
            transform_.pos = VAdd(transform_.pos,
                VScale(pushDir, overlap * 0.5f));
            transform_.Update();
        }
        break;
    }
}