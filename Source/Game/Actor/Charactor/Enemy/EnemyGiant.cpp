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
    // 見た目用メッシュは当たり判定に参加させない（判定はカプセルが担う）
    mesh->SetProfile(CollisionProfileType::NO_COLLISION);
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

    // 手のフレームに追従する攻撃判定カプセル
    // NO_COLLISION なので CollisionManager の総当たりには参加せず、
    // 攻撃中のみ手動でプレイヤーカプセルと判定する
    attackCapsule_ = AddComponent<CapsuleComponent>(
        ATTACK_CAPSULE_RADIUS, ATTACK_CAPSULE_HALF_HEIGHT);
    attackCapsule_->SetProfile(CollisionProfileType::NO_COLLISION);

    ActorBase::Init();

    // 攻撃フレームを検索
    attackHandFrame_ = MV1SearchFrame(modelId, "mixamorig:LeftHand");

    // 帰還先を初期位置に設定
    spawnPos_ = defaultPos_;

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

    // 攻撃カプセルを手のフレームへ追従させる
    UpdateAttackCapsule();

    // 攻撃中ならプレイヤーとの当たり判定
    if (state_ == STATE::ATTACK)
    {
        CheckAttackHit();
    }
}

void EnemyGiant::Draw(void)
{
    // カプセルのデバッグ描画は各 Component の Draw が行う
    CharactorBase::Draw();
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

// 攻撃カプセルを手のフレーム位置へ追従させる
void EnemyGiant::UpdateAttackCapsule(void)
{
    if (attackCapsule_ == nullptr || attackHandFrame_ < 0) return;

    auto* mesh = GetComponent<StaticMeshComponent>();
    if (mesh == nullptr) return;

    // アニメーション適用後の手のフレーム位置（ワールド座標）
    VECTOR handPos = MV1GetFramePosition(mesh->GetModelId(), attackHandFrame_);
    attackCapsule_->SetWorldPos(Vector3::FromVECTOR(handPos));
}

// 攻撃カプセルとプレイヤーカプセルの当たり判定
// 当たったらプレイヤーの HP を 20 減らす（1 回の攻撃につき 1 回のみ）
void EnemyGiant::CheckAttackHit(void)
{
    if (attackHit_) return;
    if (attackCapsule_ == nullptr) return;

    auto* playerCap = player_.GetComponent<CapsuleComponent>();
    if (playerCap == nullptr) return;

    bool isHit = HitCheck_Capsule_Capsule(
        attackCapsule_->GetTopPos().ToVECTOR(),
        attackCapsule_->GetBottomPos().ToVECTOR(),
        attackCapsule_->GetRadius(),
        playerCap->GetTopPos().ToVECTOR(),
        playerCap->GetBottomPos().ToVECTOR(),
        playerCap->GetRadius());

    if (isHit)
    {
        player_.OnDamaged(ATTACK_DAMAGE);
        attackHit_ = true;
    }
}