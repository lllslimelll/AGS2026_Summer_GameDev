#include "../../../../Utility/AsoUtility.h"
#include "../../../../Utility/Math.h"
#include "../../../../Manager/ResourceManager.h"
#include "../../../Scene/SceneManager.h"
#include "../../../../Common/AnimationController.h"
#include "../../../../Component/CapsuleComponent.h"
#include "../../../../Component/StaticMeshComponent.h"
#include "../../../../Collision/CollisionManager.h"
#include "../Player.h"
#include "EnemyGiant.h"

EnemyGiant::EnemyGiant(const EnemyBase::EnemyData& data, Player& player)
    : EnemyBase(data, player)
    , state_(STATE::NONE)
    , step_(0.0f)
    , attackHit_(false)
{
}

EnemyGiant::~EnemyGiant(void)
{
}

void EnemyGiant::Init(void)
{
    // モデルロード
    int modelId = resMng_.LoadModelDuplicate(ResourceManager::SRC::ENEMY_GIANT);
    auto* mesh = AddComponent<StaticMeshComponent>(modelId);
    mesh->SetProfile(CollisionProfileType::PAWN);
    animCtrl_ = new AnimationController(modelId);

    // アニメーション
    animCtrl_->AddInFbx(static_cast<int>(ANIM_TYPE::IDLE), 10.0f, static_cast<int>(ANIM_TYPE::IDLE));
    animCtrl_->AddInFbx(static_cast<int>(ANIM_TYPE::WALK), 10.0f, static_cast<int>(ANIM_TYPE::WALK));
    animCtrl_->AddInFbx(static_cast<int>(ANIM_TYPE::RUN), 10.0f, static_cast<int>(ANIM_TYPE::RUN));
    animCtrl_->AddInFbx(static_cast<int>(ANIM_TYPE::ATTACK), 10.0f, static_cast<int>(ANIM_TYPE::ATTACK));

    // トランスフォーム
    SetScl(Vector3(SCALE, SCALE, SCALE));
    SetRot(Quaternion::Euler(DEFAULT_LOCAL_ROT));
    SetPos(defaultPos_ + Vector3::UP * 50.0f);

    // カプセルコライダー
    constexpr float centerY = (COL_CAPSULE_TOP_LOCAL_POS.y + COL_CAPSULE_DOWN_LOCAL_POS.y) * 0.5f;
    constexpr float halfH = (COL_CAPSULE_TOP_LOCAL_POS.y - COL_CAPSULE_DOWN_LOCAL_POS.y) * 0.5f;
    auto* cap = AddComponent<CapsuleComponent>(COL_CAPSULE_RADIUS, halfH);
    cap->SetProfile(CollisionProfileType::PAWN);
    cap->SetLocalPos(Vector3(0.0f, centerY, 0.0f));

    ActorBase::Init();

    // 攻撃フレームを検索
    attackHandFrame_ = MV1SearchFrame(modelId, "mixamorig:LeftHand");

    // 状態遷移バインド
    stateChanges_.emplace(static_cast<int>(STATE::NONE), std::bind(&EnemyGiant::ChangeStateNone, this));
    stateChanges_.emplace(static_cast<int>(STATE::THINK), std::bind(&EnemyGiant::ChangeStateThink, this));
    stateChanges_.emplace(static_cast<int>(STATE::IDLE), std::bind(&EnemyGiant::ChangeStateIdle, this));
    stateChanges_.emplace(static_cast<int>(STATE::PATROL), std::bind(&EnemyGiant::ChangeStatePatrol, this));
    stateChanges_.emplace(static_cast<int>(STATE::CHASE), std::bind(&EnemyGiant::ChangeStateChase, this));
    stateChanges_.emplace(static_cast<int>(STATE::ATTACK), std::bind(&EnemyGiant::ChangeStateAttack, this));
    stateChanges_.emplace(static_cast<int>(STATE::RETURN), std::bind(&EnemyGiant::ChangeStateReturn, this));
    stateChanges_.emplace(static_cast<int>(STATE::END), std::bind(&EnemyGiant::ChangeStateEnd, this));

    ChangeState(STATE::THINK);
}

// ===== ライフサイクル =====

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
        SetPos(prevPos_);
        ChangeState(STATE::THINK);
    }

    PushBackFromPlayer();
}

void EnemyGiant::Draw(void)
{
    CharactorBase::Draw();

#ifdef _DEBUG
    int modelId = GetComponent<StaticMeshComponent>()->GetModelId();
    VECTOR handPos = MV1GetFramePosition(modelId, attackHandFrame_);
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
    ChangeState(rand < 30 ? STATE::IDLE : STATE::PATROL);
}

void EnemyGiant::ChangeStateIdle(void)
{
    stateUpdate_ = std::bind(&EnemyGiant::UpdateIdle, this);
    step_ = 2.0f + static_cast<float>(GetRand(3));
    movePow_ = Vector3::ZERO;
    animCtrl_->Play(static_cast<int>(ANIM_TYPE::IDLE), true);
}

void EnemyGiant::ChangeStatePatrol(void)
{
    stateUpdate_ = std::bind(&EnemyGiant::UpdatePatrol, this);
    movePow_ = Vector3::ZERO;

    // 接平面上でランダム方向を決める
    Vector3 upDir = Vector3::UP;
    Vector3 ref = (fabsf(upDir.y) < 0.99f) ? Vector3::UP : Vector3::RIGHT;
    Vector3 tangent = Vector3::Cross(upDir, ref).Normalized();
    Vector3 binormal = Vector3::Cross(upDir, tangent).Normalized();

    float angle = static_cast<float>(GetRand(360)) * Math::ToRadian(1.0f);
    moveDir_ = tangent * cosf(angle) + binormal * sinf(angle);
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
    movePow_ = Vector3::ZERO;
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
}

// ===== 更新 =====

void EnemyGiant::UpdateNone(void) {}
void EnemyGiant::UpdateThink(void) {}
void EnemyGiant::UpdateEnd(void) {}

void EnemyGiant::UpdateIdle(void)
{
    step_ -= scnMng_.GetDeltaTime();
    if (step_ < 0.0f) { ChangeState(STATE::THINK); return; }
    if (InSearchCone()) { ChangeState(STATE::CHASE); }
}

void EnemyGiant::UpdatePatrol(void)
{
    step_ -= scnMng_.GetDeltaTime();
    if (step_ < 0.0f) { ChangeState(STATE::THINK); return; }

    // 接平面に再投影
    float   d = Vector3::Dot(moveDir_, Vector3::UP);
    moveDir_ = (moveDir_ - Vector3::UP * d).Normalized();
    faceDir_ = moveDir_;
    movePow_ = moveDir_ * moveSpeed_;

    if (InSearchCone()) { ChangeState(STATE::CHASE); }
}

void EnemyGiant::UpdateChase(void)
{
    float dist = DistToPlayer();
    if (dist <= DIST_ATTACK) { ChangeState(STATE::ATTACK); return; }
    if (dist > DIST_CHASE) { ChangeState(STATE::RETURN); return; }

    SetMoveDirToTarget(player_.GetPos());
    movePow_ = moveDir_ * moveSpeed_;
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

    if (attackHit_) return;

    int    modelId = GetComponent<StaticMeshComponent>()->GetModelId();
    VECTOR handPos = MV1GetFramePosition(modelId, attackHandFrame_);
    Vector3 hand = Vector3::FromVECTOR(handPos);

    // CapsuleComponent 経由でプレイヤーとの当たりを判定
    auto* playerCap = player_.GetComponent<CapsuleComponent>();
    if (playerCap == nullptr) return;

    if (Vector3::Distance(hand, player_.GetPos()) < ATTACK_SPHERE_RADIUS + playerCap->GetRadius())
    {
        player_.OnDamaged(static_cast<int>(ATTACK_DAMAGE));
        attackHit_ = true;
    }
}

void EnemyGiant::UpdateReturn(void)
{
    if (Vector3::Distance(GetPos(), spawnPos_) < 120.0f)
    {
        ChangeState(STATE::THINK);
        return;
    }

    SetMoveDirToTarget(spawnPos_);
    movePow_ = moveDir_ * moveSpeed_;

    if (InSearchCone()) { ChangeState(STATE::CHASE); }
}

// ===== ヘルパー =====

void EnemyGiant::SetMoveDirToTarget(const Vector3& target)
{
    Vector3 dir = (target - GetPos()).Normalized();

    // 接平面に投影
    float   d = Vector3::Dot(dir, Vector3::UP);
    Vector3 projected = dir - Vector3::UP * d;

    if (projected.Length() < 0.01f) return;

    moveDir_ = projected.Normalized();
    faceDir_ = moveDir_;
}

bool EnemyGiant::InSearchCone(void) const
{
    return EnemyBase::InSearchCone(VIEW_DIST, VIEW_HALF_FOV);
}

float EnemyGiant::DistToPlayer(void) const
{
    return Vector3::Distance(GetPos(), player_.GetPos());
}

void EnemyGiant::PushBackFromPlayer(void)
{
    // CapsuleComponent 経由で押し戻し
    auto* myCap = GetComponent<CapsuleComponent>();
    auto* playerCap = player_.GetComponent<CapsuleComponent>();
    if (!myCap || !playerCap) return;

    Vector3 myCenter = GetPos();
    Vector3 playerCenter = player_.GetPos();
    float   dist = Vector3::Distance(myCenter, playerCenter);
    float   minDist = myCap->GetRadius() + playerCap->GetRadius();

    if (dist < minDist && dist > 0.01f)
    {
        Vector3 pushDir = (myCenter - playerCenter).Normalized();
        SetPos(GetPos() + pushDir * (minDist - dist) * 0.5f);
    }
}