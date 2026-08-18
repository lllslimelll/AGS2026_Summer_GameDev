#include <cmath>
#include <random>
#include "../../../../Utility/AsoUtility.h"
#include "../../../../Manager/ResourceManager.h"
#include "../../../../Scene/SceneManager.h"
#include "../../../../Manager/SoundManager.h"
#include "../../../Common/AnimationController.h"
#include "../../../Collider/ColliderLine.h"
#include "../../../Collider/ColliderCapsule.h"
#include "../../../Collider/ColliderSphere.h"
#include "../../../Collider/ColliderModel.h"
#include "../Player.h"
#include "EnemyGiant.h"

EnemyGiant::EnemyGiant(const EnemyBase::EnemyData& data, Player& player)
    :
    EnemyBase(data, player),
    state_(STATE::NONE),
    idleTimer_(0.0f),
    attackHit_(false),
    attackHandFrame_(-1),
    wanderCenter_(data.defaultPos),
    wanderTarget_(data.defaultPos),
    lastWanderTheta_(0.0f),
    // this のアドレスを XOR してシードに使う。
    // 複数体が同フレームに生成されても同一シードにならない。
    rng_(std::random_device{}() ^ static_cast<unsigned>(
        reinterpret_cast<uintptr_t>(this))),
    wallNormalXZ_(AsoUtility::VECTOR_ZERO),
    prevMoveDir_(AsoUtility::DIR_F),
    lastAvoidChoice_(0),
    avoidCommitTimer_(0.0f),
    stuckTimer_(0.0f),
    stuckLastPos_(AsoUtility::VECTOR_ZERO)
{
}

EnemyGiant::~EnemyGiant(void) {}

// ---------------------------------------------------------------
// 初期化
// ---------------------------------------------------------------
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
    transform_.Update();
}

void EnemyGiant::InitCollider(void)
{
    ColliderLine* colLine = new ColliderLine(
        ColliderBase::TAG::ENEMY, &transform_,
        COL_LINE_START_LOCAL_POS, COL_LINE_END_LOCAL_POS);
    ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::GROUND_LINE), colLine);

    ColliderCapsule* colCapsule = new ColliderCapsule(
        ColliderBase::TAG::ENEMY, &transform_,
        COL_CAPSULE_TOP_LOCAL_POS, COL_CAPSULE_DOWN_LOCAL_POS,
        COL_CAPSULE_RADIUS);
    ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::CAPSULE), colCapsule);
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
    stateChanges_.emplace(static_cast<int>(STATE::IDLE),
        std::bind(&EnemyGiant::ChangeStateIdle, this));
    stateChanges_.emplace(static_cast<int>(STATE::WANDER),
        std::bind(&EnemyGiant::ChangeStateWander, this));
    stateChanges_.emplace(static_cast<int>(STATE::CHASE),
        std::bind(&EnemyGiant::ChangeStateChase, this));
    stateChanges_.emplace(static_cast<int>(STATE::ATTACK),
        std::bind(&EnemyGiant::ChangeStateAttack, this));
    stateChanges_.emplace(static_cast<int>(STATE::END),
        std::bind(&EnemyGiant::ChangeStateEnd, this));

    attackHandFrame_ = MV1SearchFrame(transform_.modelId, "mixamorig:LeftHand");

    wanderCenter_ = defaultPos_;
    PickNextWanderTarget();

    ChangeState(STATE::WANDER);

    InitCurvatureShaderSkinned();
}

// ---------------------------------------------------------------
// 更新
// ---------------------------------------------------------------
void EnemyGiant::UpdateProcess(void)
{
    stateUpdate_();
}

void EnemyGiant::UpdateProcessPost(void)
{
    EnemyBase::UpdateProcessPost();

    // 押し戻し後の位置から壁法線を推定して保存。
    // 次フレームの ApplyWallSlide が読む。
    UpdateWallNormal();

    PushBackFromPlayer();
}

void EnemyGiant::Draw(void)
{
    EnemyBase::Draw();

#ifdef _DEBUG
    if (attackHandFrame_ >= 0)
    {
        DrawSphere3D(GetAttackSpherePos(), ATTACK_SPHERE_RADIUS,
            16, 0xff0000, 0xff0000, false);
    }
    if (debugDrawView_) DrawDebugAI();
#endif
}

// ===============================================================
// 状態遷移
// ===============================================================
void EnemyGiant::ChangeState(STATE state)
{
    state_ = state;
    CharactorBase::ChangeState(static_cast<int>(state_));
}

void EnemyGiant::ChangeStateNone(void)
{
    stateUpdate_ = std::bind(&EnemyGiant::UpdateNone, this);
}

void EnemyGiant::ChangeStateIdle(void)
{
    stateUpdate_ = std::bind(&EnemyGiant::UpdateIdle, this);

    idleTimer_ = RandFloat(IDLE_TIME_MIN, IDLE_TIME_MAX);

    movePow_ = AsoUtility::VECTOR_ZERO;
    animCtrl_->Play(static_cast<int>(ANIM_TYPE::IDLE), true);
    ResetSteering();
}

void EnemyGiant::ChangeStateWander(void)
{
    stateUpdate_ = std::bind(&EnemyGiant::UpdateWander, this);

    SetMoveDirToTarget(wanderTarget_);

    moveSpeed_ = SPEED_WANDER;
    animCtrl_->Play(static_cast<int>(ANIM_TYPE::WALK), true);
    ResetSteering();
}

void EnemyGiant::ChangeStateChase(void)
{
    stateUpdate_ = std::bind(&EnemyGiant::UpdateChase, this);

    moveSpeed_ = SPEED_CHASE;
    animCtrl_->Play(static_cast<int>(ANIM_TYPE::RUN), true);
    ResetSteering();
}

void EnemyGiant::ChangeStateAttack(void)
{
    stateUpdate_ = std::bind(&EnemyGiant::UpdateAttack, this);
    movePow_ = AsoUtility::VECTOR_ZERO;
    attackHit_ = false;

    SetMoveDirToTarget(player_.GetTransform().pos);

    if (animCtrl_->GetPlayType() == static_cast<int>(ANIM_TYPE::ATTACK))
        animCtrl_->Replay();
    else
        animCtrl_->Play(static_cast<int>(ANIM_TYPE::ATTACK), false);
}

void EnemyGiant::ChangeStateEnd(void)
{
    stateUpdate_ = std::bind(&EnemyGiant::UpdateEnd, this);
}

// ===============================================================
// 各ステート更新
// ===============================================================
void EnemyGiant::UpdateNone(void) {}
void EnemyGiant::UpdateEnd(void) {}

void EnemyGiant::UpdateIdle(void)
{
    movePow_ = AsoUtility::VECTOR_ZERO;

    if (IsPlayerInSight())
    {
        ChangeState(STATE::CHASE);
        return;
    }

    idleTimer_ -= scnMng_.GetDeltaTime();
    if (idleTimer_ <= 0.0f)
    {
        PickNextWanderTarget();
        ChangeState(STATE::WANDER);
    }
}

void EnemyGiant::UpdateWander(void)
{
    if (IsPlayerInSight())
    {
        ChangeState(STATE::CHASE);
        return;
    }

    VECTOR delta = VSub(wanderTarget_, transform_.pos);
    delta.y = 0.0f;

    if (VSize(delta) <= DIST_ARRIVE)
    {
        ChangeState(STATE::IDLE);
        return;
    }

    if (IsStuck())
    {
        PickNextWanderTarget();
        ChangeState(STATE::WANDER);
        return;
    }

    // 毎フレームレイキャストで前方を確認しながら目標へ向かう。
    // 以前の SetMoveDirToTarget（直線代入）と異なり、障害物を手前で検知して曲がる。
    VECTOR dir = ComputeWanderDir();
    moveDir_ = dir;
    faceDir_ = dir;

    ApplyWallSlide();
    LimitTurnRate();
    movePow_ = VScale(moveDir_, moveSpeed_);
}

void EnemyGiant::UpdateChase(void)
{
    float dist = DistToPlayer();

    if (dist > DIST_LOSE_CHASE)
    {
        // 見失った：現在位置を新しい徘徊中心にして WANDER へ
        SetWanderCenter(transform_.pos);
        PickNextWanderTarget();
        ChangeState(STATE::WANDER);
        return;
    }

    if (dist <= DIST_ATTACK)
    {
        ChangeState(STATE::ATTACK);
        return;
    }

    VECTOR dir = ComputeChaseDir();
    moveDir_ = dir;
    faceDir_ = dir;

    ApplyWallSlide();
    LimitTurnRate();
    movePow_ = VScale(moveDir_, moveSpeed_);
}

void EnemyGiant::UpdateAttack(void)
{
    const auto& anim = animCtrl_->GetPlayAnim();
    float progress = (anim.totalTime > 0.0f) ? (anim.step / anim.totalTime) : 0.0f;

    bool hitboxActive =
        !attackHit_ &&
        progress >= ATTACK_HIT_START &&
        progress <= ATTACK_HIT_END;

    if (hitboxActive)
    {
        VECTOR spherePos = GetAttackSpherePos();

        for (const auto& hitCol : hitColliders_)
        {
            if (hitCol->GetTag() != ColliderBase::TAG::PLAYER) continue;

            const ColliderCapsule* playerCapsule =
                dynamic_cast<const ColliderCapsule*>(hitCol);
            if (playerCapsule == nullptr) continue;

            if (AsoUtility::IsHitSphereCapsule(
                spherePos, ATTACK_SPHERE_RADIUS,
                playerCapsule->GetPosTop(),
                playerCapsule->GetPosDown(),
                playerCapsule->GetRadius()))
            {
                player_.OnDamagedByEnemy(ATTACK_DAMAGE);
                SoundManager::GetInstance().PlayDamaged();
                attackHit_ = true;
                break;
            }
        }
    }

    if (animCtrl_->IsEnd())
    {
        float dist = DistToPlayer();
        if (dist > DIST_LOSE_CHASE)
        {
            SetWanderCenter(transform_.pos);
            PickNextWanderTarget();
            ChangeState(STATE::WANDER);
        }
        else if (dist <= DIST_ATTACK) ChangeState(STATE::ATTACK);
        else                          ChangeState(STATE::CHASE);
    }
}

// ===============================================================
// ヘルパー：徘徊
// ===============================================================

// wanderCenter_ 周辺からランダムな目標を抽選する。
//
// 選出戦略：
//   1. 最大 WANDER_MAX_TRIES 回ランダム抽選し、3本レイが全部通れば即採用。
//   2. 全滅したら「前回角度 +π」の逆方向を試す（囲われた空間でも反対側は開けやすい）。
//   3. それでも駄目なら wanderCenter_ 自体を目標にしてスタック検知に任せる。
void EnemyGiant::PickNextWanderTarget(void)
{
    // ---- 通常試行 ----
    for (int i = 0; i < WANDER_MAX_TRIES; ++i)
    {
        float  theta = RandAngle();
        float  r = RandFloat(WANDER_MIN_DIST, WANDER_RADIUS);

        VECTOR cand = wanderCenter_;
        cand.x += r * cosf(theta);
        cand.z += r * sinf(theta);

        if (IsReachable(cand))
        {
            wanderTarget_ = cand;
            lastWanderTheta_ = theta;
            return;
        }
    }

    // ---- フォールバック：逆方向 ----
    {
        float  theta = lastWanderTheta_ + DX_PI_F;
        float  r = RandFloat(WANDER_MIN_DIST, WANDER_RADIUS * 0.5f);

        VECTOR cand = wanderCenter_;
        cand.x += r * cosf(theta);
        cand.z += r * sinf(theta);

        if (IsReachable(cand))
        {
            wanderTarget_ = cand;
            lastWanderTheta_ = theta;
            return;
        }
    }

    // ---- 最終手段：中心を目標にしてスタック検知に任せる ----
    wanderTarget_ = wanderCenter_;
    lastWanderTheta_ = lastWanderTheta_ + DX_PI_F; // 次回は逆側から始める
}

// カプセル幅を考慮した3本レイ到達可能チェック。
//
// 中心1本だけでは幅120のカプセルが通れない狭所を見逃す。
// 目標方向に垂直なオフセット2本も飛ばし、3本全部通った時だけ true を返す。
bool EnemyGiant::IsReachable(const VECTOR& cand) const
{
    VECTOR from = transform_.pos; from.y += EYE_HEIGHT;
    VECTOR to = cand;          to.y += EYE_HEIGHT;

    // 中心レイ
    if (!IsPathClear(from, to)) return false;

    // 目標方向の単位ベクトル（XZ）
    VECTOR dir = VSub(to, from);
    dir.y = 0.0f;
    float len = VSize(dir);
    if (len < 0.001f) return true; // ほぼ同一点なら通過扱い
    dir = VScale(dir, 1.0f / len);

    // 進行方向に垂直な XZ 法線：(dx, dz) を 90 度回転 → (-dz, dx)
    VECTOR perp = { -dir.z, 0.0f, dir.x };
    VECTOR offset = VScale(perp, COL_CAPSULE_RADIUS * 0.8f); // 少し内側で判定

    if (!IsPathClear(VAdd(from, offset), VAdd(to, offset))) return false;
    if (!IsPathClear(VSub(from, offset), VSub(to, offset))) return false;

    return true;
}

bool EnemyGiant::IsPathClear(const VECTOR& from, const VECTOR& to) const
{
    // IsOccludedByColliders は非 const なので const_cast で回避
    return !const_cast<EnemyGiant*>(this)->IsOccludedByColliders(from, to);
}

void EnemyGiant::SetWanderCenter(const VECTOR& center)
{
    wanderCenter_ = center;
    wanderCenter_.y = defaultPos_.y; // 極端な高さを防ぐため Y は足元基準
}

// ===============================================================
// ヘルパー：乱数
// ===============================================================

// std::mt19937（メルセンヌ・ツイスタ）を使用。
// インスタンス固有シードなのでどの個体も独立した乱数列を持つ。
float EnemyGiant::RandFloat(float minVal, float maxVal)
{
    std::uniform_real_distribution<float> dist(minVal, maxVal);
    return dist(rng_);
}

float EnemyGiant::RandAngle(void)
{
    return RandFloat(0.0f, 2.0f * DX_PI_F);
}

// ===============================================================
// ヘルパー：移動と方向
// ===============================================================
void EnemyGiant::SetMoveDirToTarget(const VECTOR& target)
{
    VECTOR dir = VSub(target, transform_.pos);
    dir.y = 0.0f;
    if (VSize(dir) < 0.01f) return;

    moveDir_ = VNorm(dir);
    faceDir_ = moveDir_;
}

float EnemyGiant::DistToPlayer(void) const
{
    return VSize(VSub(player_.GetTransform().pos, transform_.pos));
}

float EnemyGiant::DistToPlayerXZ(void) const
{
    VECTOR d = VSub(player_.GetTransform().pos, transform_.pos);
    d.y = 0.0f;
    return VSize(d);
}

bool EnemyGiant::IsPlayerInSight(void)
{
    VECTOR myPos = transform_.pos;
    VECTOR plPos = player_.GetTransform().pos;

    VECTOR toPl = VSub(plPos, myPos);
    toPl.y = 0.0f;
    float dist = VSize(toPl);

    if (dist > VIEW_RANGE) return false;
    if (dist < 0.0001f)    return true;

    VECTOR toPlDir = VScale(toPl, 1.0f / dist);

    VECTOR fwd = faceDir_;
    fwd.y = 0.0f;
    if (VSize(fwd) < 0.0001f) return false;
    fwd = VNorm(fwd);

    float cosAngle = VDot(fwd, toPlDir);
    float cosHalfFov = cosf(VIEW_HALF_FOV_RAD);
    if (cosAngle < cosHalfFov) return false;

    VECTOR eye = myPos; eye.y += EYE_HEIGHT;
    VECTOR chest = plPos; chest.y += EYE_HEIGHT;
    if (IsOccludedByColliders(eye, chest)) return false;

    return true;
}

// ---------------------------------------------------------------
// 追跡方向の決定（レイキャスト＋ヒステリシス）
//
// 前方が塞がれていたら ±45度 → ±90度 の順で候補を探す。
// 一度選んだ方向は AVOID_COMMIT_SEC の間保持し、
// フレーム毎に左右の判定が反転するのを防ぐ。
// ---------------------------------------------------------------
VECTOR EnemyGiant::ComputeChaseDir(void)
{
    VECTOR myPos = transform_.pos;
    VECTOR plPos = player_.GetTransform().pos;

    VECTOR base = VSub(plPos, myPos);
    base.y = 0.0f;
    float baseLen = VSize(base);
    if (baseLen < 0.001f) return faceDir_;
    base = VScale(base, 1.0f / baseLen);

    VECTOR probeFrom = myPos; probeFrom.y += EYE_HEIGHT;

    auto probe = [&](const VECTOR& dir) -> bool {
        VECTOR to = VAdd(probeFrom, VScale(dir, AVOID_PROBE_DIST));
        return IsOccludedByColliders(probeFrom, to);
        };

    Quaternion rotL = Quaternion::AngleAxis(+AVOID_ANGLE_RAD, AsoUtility::AXIS_Y);
    Quaternion rotR = Quaternion::AngleAxis(-AVOID_ANGLE_RAD, AsoUtility::AXIS_Y);
    VECTOR dirL = rotL.PosAxis(base);
    VECTOR dirR = rotR.PosAxis(base);

    bool blockedFwd = probe(base);

    // ヒステリシス：前回の選択がまだ有効かつ通れるならそれを使う
    if (avoidCommitTimer_ > 0.0f)
    {
        avoidCommitTimer_ -= scnMng_.GetDeltaTime();

        if (lastAvoidChoice_ == 0 && !blockedFwd)  return base;
        if (lastAvoidChoice_ == -1 && !probe(dirL)) return dirL;
        if (lastAvoidChoice_ == +1 && !probe(dirR)) return dirR;
        // 前回選択が塞がったら下の再判定へ
    }

    auto commit = [&](int choice, const VECTOR& dir) -> VECTOR {
        lastAvoidChoice_ = choice;
        avoidCommitTimer_ = AVOID_COMMIT_SEC;
        return dir;
        };

    if (!blockedFwd) return commit(0, base);

    bool blockedL = probe(dirL);
    bool blockedR = probe(dirR);

    if (!blockedL && !blockedR)
    {
        // 両方空いていたらプレイヤーに近い方を選ぶ
        VECTOR endL = VAdd(myPos, VScale(dirL, AVOID_PROBE_DIST));
        VECTOR endR = VAdd(myPos, VScale(dirR, AVOID_PROBE_DIST));
        float  dL = VSize(VSub(plPos, endL));
        float  dR = VSize(VSub(plPos, endR));
        return (dL <= dR) ? commit(-1, dirL) : commit(+1, dirR);
    }
    if (!blockedL) return commit(-1, dirL);
    if (!blockedR) return commit(+1, dirR);

    // ±90度フォールバック
    VECTOR dirWL = Quaternion::AngleAxis(+AVOID_ANGLE_WIDE_RAD, AsoUtility::AXIS_Y).PosAxis(base);
    VECTOR dirWR = Quaternion::AngleAxis(-AVOID_ANGLE_WIDE_RAD, AsoUtility::AXIS_Y).PosAxis(base);

    if (!probe(dirWL)) return commit(-1, dirWL);
    if (!probe(dirWR)) return commit(+1, dirWR);

    // 完全に詰まっている場合は基準方向のまま。
    // ApplyWallSlide が壁に沿わせてくれる。
    return commit(0, base);
}

// ---------------------------------------------------------------
// 徘徊目標方向の決定（ComputeChaseDir の徘徊版）
//
// ComputeChaseDir と全く同じロジックで、
// 「プレイヤー方向」の代わりに「wanderTarget_ 方向」を基準にする。
// WANDER 中も毎フレーム前方を確認して障害物を手前で避ける。
// ---------------------------------------------------------------
VECTOR EnemyGiant::ComputeWanderDir(void)
{
    VECTOR myPos = transform_.pos;
    VECTOR target = wanderTarget_;

    VECTOR base = VSub(target, myPos);
    base.y = 0.0f;
    float baseLen = VSize(base);
    if (baseLen < 0.001f) return faceDir_;
    base = VScale(base, 1.0f / baseLen);

    VECTOR probeFrom = myPos; probeFrom.y += EYE_HEIGHT;

    auto probe = [&](const VECTOR& dir) -> bool {
        VECTOR to = VAdd(probeFrom, VScale(dir, AVOID_PROBE_DIST));
        return IsOccludedByColliders(probeFrom, to);
        };

    Quaternion rotL = Quaternion::AngleAxis(+AVOID_ANGLE_RAD, AsoUtility::AXIS_Y);
    Quaternion rotR = Quaternion::AngleAxis(-AVOID_ANGLE_RAD, AsoUtility::AXIS_Y);
    VECTOR dirL = rotL.PosAxis(base);
    VECTOR dirR = rotR.PosAxis(base);

    bool blockedFwd = probe(base);

    if (avoidCommitTimer_ > 0.0f)
    {
        avoidCommitTimer_ -= scnMng_.GetDeltaTime();

        if (lastAvoidChoice_ == 0 && !blockedFwd)  return base;
        if (lastAvoidChoice_ == -1 && !probe(dirL)) return dirL;
        if (lastAvoidChoice_ == +1 && !probe(dirR)) return dirR;
    }

    auto commit = [&](int choice, const VECTOR& dir) -> VECTOR {
        lastAvoidChoice_ = choice;
        avoidCommitTimer_ = AVOID_COMMIT_SEC;
        return dir;
        };

    if (!blockedFwd) return commit(0, base);

    bool blockedL = probe(dirL);
    bool blockedR = probe(dirR);

    if (!blockedL && !blockedR)
    {
        // 両方空いていたら目標に近い方を選ぶ
        VECTOR endL = VAdd(myPos, VScale(dirL, AVOID_PROBE_DIST));
        VECTOR endR = VAdd(myPos, VScale(dirR, AVOID_PROBE_DIST));
        float  dL = VSize(VSub(target, endL));
        float  dR = VSize(VSub(target, endR));
        return (dL <= dR) ? commit(-1, dirL) : commit(+1, dirR);
    }
    if (!blockedL) return commit(-1, dirL);
    if (!blockedR) return commit(+1, dirR);

    VECTOR dirWL = Quaternion::AngleAxis(+AVOID_ANGLE_WIDE_RAD, AsoUtility::AXIS_Y).PosAxis(base);
    VECTOR dirWR = Quaternion::AngleAxis(-AVOID_ANGLE_WIDE_RAD, AsoUtility::AXIS_Y).PosAxis(base);

    if (!probe(dirWL)) return commit(-1, dirWL);
    if (!probe(dirWR)) return commit(+1, dirWR);

    return commit(0, base);
}

void EnemyGiant::PushBackFromPlayer(void)
{
    int capsuleType = static_cast<int>(COLLIDER_TYPE::CAPSULE);
    if (ownColliders_.count(capsuleType) == 0) return;

    ColliderCapsule* myCapsule =
        dynamic_cast<ColliderCapsule*>(ownColliders_.at(capsuleType));
    if (myCapsule == nullptr) return;

    for (const auto& hitCol : hitColliders_)
    {
        if (hitCol->GetTag() != ColliderBase::TAG::PLAYER) continue;

        const ColliderCapsule* playerCapsule =
            dynamic_cast<const ColliderCapsule*>(hitCol);
        if (playerCapsule == nullptr) continue;

        VECTOR myC = myCapsule->GetCenter();
        VECTOR plC = playerCapsule->GetCenter();

        float dx = myC.x - plC.x;
        float dz = myC.z - plC.z;
        float distSq = dx * dx + dz * dz;

        float minDist = myCapsule->GetRadius() + playerCapsule->GetRadius();
        if (distSq >= minDist * minDist) return;

        float dist = sqrtf(distSq);
        if (dist < 0.0001f) { dx = 1.0f; dz = 0.0f; dist = 1.0f; }

        float k = (minDist - dist) * 0.5f / dist;
        transform_.pos.x += dx * k;
        transform_.pos.z += dz * k;
        transform_.Update();
        return;
    }
}

VECTOR EnemyGiant::GetAttackSpherePos(void) const
{
    VECTOR framePos = MV1GetFramePosition(transform_.modelId, attackHandFrame_);
    VECTOR worldOffset = transform_.quaRot.PosAxis(ATTACK_SPHERE_OFFSET_LOCAL);
    return VAdd(framePos, worldOffset);
}

// ===============================================================
// ステアリングフィルタ
// ===============================================================

// 「動かすつもりだった位置（prevPos_ + movePow_）」と
// 「押し戻し後の実位置」の差分から壁の外向き法線を推定する。
//
// CollisionCapsule が三角形法線方向に押し戻しているので、
//     push = 実位置 - expected ≒ 壁の外向き法線 × 押し戻し量
// が成立する。
void EnemyGiant::UpdateWallNormal(void)
{
    VECTOR expected = VAdd(prevPos_, movePow_);

    float pdx = transform_.pos.x - expected.x;
    float pdz = transform_.pos.z - expected.z;
    float pushLen = sqrtf(pdx * pdx + pdz * pdz);

    if (pushLen < WALL_PUSH_EPSILON)
    {
        wallNormalXZ_ = AsoUtility::VECTOR_ZERO;
        return;
    }

    float inv = 1.0f / pushLen;
    wallNormalXZ_.x = pdx * inv;
    wallNormalXZ_.y = 0.0f;
    wallNormalXZ_.z = pdz * inv;
}

// v_slide = v - (v・n)n
// moveDir_ から壁法線方向の成分を引き、壁の接線成分だけを残す（平面射影）。
void EnemyGiant::ApplyWallSlide(void)
{
    if (VSize(wallNormalXZ_) < 0.0001f) return;

    float vn = VDot(moveDir_, wallNormalXZ_);
    if (vn >= 0.0f) return; // 壁から離れる向きならスライド不要

    moveDir_ = VSub(moveDir_, VScale(wallNormalXZ_, vn));

    float len = VSize(moveDir_);
    if (len > 0.0001f)
    {
        moveDir_ = VScale(moveDir_, 1.0f / len);
    }
    else
    {
        // 壁に真正面から突っ込んで方向が消えた場合。
        // XZ 平面での法線 90 度回転 (nx, nz) → (-nz, nx) を接線として採用。
        moveDir_.x = -wallNormalXZ_.z;
        moveDir_.y = 0.0f;
        moveDir_.z = wallNormalXZ_.x;
    }

    faceDir_ = moveDir_;
}

// moveDir_ の 1 フレームあたり回転量を MAX_TURN_RAD_PER_SEC で制限する。
//
// θ = arccos(a・b) で前フレーム方向 a と目標方向 b のなす角を求め、
// 上限 maxAngle = ω × Δt を超えていたら (a × b).y の符号で回転方向を決めて
// Y 軸周りに maxAngle だけ a を回した結果を新しい moveDir_ にする。
void EnemyGiant::LimitTurnRate(void)
{
    if (VSize(prevMoveDir_) < 0.0001f || VSize(moveDir_) < 0.0001f)
    {
        prevMoveDir_ = moveDir_;
        return;
    }

    VECTOR a = prevMoveDir_; a.y = 0.0f;
    VECTOR b = moveDir_;     b.y = 0.0f;
    float la = VSize(a), lb = VSize(b);
    if (la < 0.0001f || lb < 0.0001f) { prevMoveDir_ = moveDir_; return; }
    a = VScale(a, 1.0f / la);
    b = VScale(b, 1.0f / lb);

    float cosA = VDot(a, b);
    if (cosA > 1.0f) cosA = 1.0f;
    if (cosA < -1.0f) cosA = -1.0f;
    float angle = acosf(cosA);
    float maxAngle = MAX_TURN_RAD_PER_SEC * scnMng_.GetDeltaTime();

    if (angle <= maxAngle)
    {
        prevMoveDir_ = moveDir_;
        faceDir_ = moveDir_;
        return;
    }

    VECTOR cross = VCross(a, b);
    float  sign = (cross.y >= 0.0f) ? +1.0f : -1.0f;

    Quaternion rot = Quaternion::AngleAxis(sign * maxAngle, AsoUtility::AXIS_Y);
    moveDir_ = rot.PosAxis(a);
    faceDir_ = moveDir_;
    prevMoveDir_ = moveDir_;
}

// ===============================================================
// スタック検知
// ===============================================================

// 1フレームの実移動が STUCK_MOVE_MIN 未満なら stuckTimer_ を加算。
// STUCK_TIMEOUT_SEC を超えたら true を返す。
bool EnemyGiant::IsStuck(void)
{
    float dx = transform_.pos.x - stuckLastPos_.x;
    float dz = transform_.pos.z - stuckLastPos_.z;

    if (dx * dx + dz * dz < STUCK_MOVE_MIN * STUCK_MOVE_MIN)
        stuckTimer_ += scnMng_.GetDeltaTime();
    else
        stuckTimer_ = 0.0f;

    stuckLastPos_ = transform_.pos;

    return stuckTimer_ >= STUCK_TIMEOUT_SEC;
}

// ステート遷移時にステアリング系の状態をまとめてリセット
void EnemyGiant::ResetSteering(void)
{
    stuckTimer_ = 0.0f;
    stuckLastPos_ = transform_.pos;
    lastAvoidChoice_ = 0;
    avoidCommitTimer_ = 0.0f;
    prevMoveDir_ = moveDir_;
}

// ===============================================================
// デバッグ描画
// ===============================================================
#ifdef _DEBUG
void EnemyGiant::DrawDebugAI(void)
{
    VECTOR pos = transform_.pos;
    pos.y += EYE_HEIGHT;

    VECTOR fwd = faceDir_;
    fwd.y = 0.0f;
    if (VSize(fwd) < 0.0001f) fwd = AsoUtility::DIR_F;
    fwd = VNorm(fwd);

    // 視野円錐（検知中：赤、非検知：緑）
    unsigned int viewCol = IsPlayerInSight()
        ? GetColor(255, 40, 40)
        : GetColor(40, 255, 40);

    Quaternion rotL = Quaternion::AngleAxis(+VIEW_HALF_FOV_RAD, AsoUtility::AXIS_Y);
    Quaternion rotR = Quaternion::AngleAxis(-VIEW_HALF_FOV_RAD, AsoUtility::AXIS_Y);
    VECTOR dirL = rotL.PosAxis(fwd);
    VECTOR dirR = rotR.PosAxis(fwd);

    DrawLine3D(pos, VAdd(pos, VScale(fwd, VIEW_RANGE)), viewCol);
    DrawLine3D(pos, VAdd(pos, VScale(dirL, VIEW_RANGE)), viewCol);
    DrawLine3D(pos, VAdd(pos, VScale(dirR, VIEW_RANGE)), viewCol);

    const int SEG = 24;
    VECTOR prev = VAdd(pos, VScale(dirL, VIEW_RANGE));
    for (int i = 1; i <= SEG; ++i)
    {
        float t = static_cast<float>(i) / SEG;
        float ang = -VIEW_HALF_FOV_RAD + 2.0f * VIEW_HALF_FOV_RAD * t;
        VECTOR e = VAdd(pos, VScale(
            Quaternion::AngleAxis(ang, AsoUtility::AXIS_Y).PosAxis(fwd), VIEW_RANGE));
        DrawLine3D(prev, e, viewCol);
        prev = e;
    }

    // 徘徊範囲（水色の円 + 中心球）
    {
        unsigned int col = GetColor(80, 200, 255);
        VECTOR c = wanderCenter_; c.y += 5.0f;
        VECTOR prevP = {};
        for (int i = 0; i <= 32; ++i)
        {
            float a = static_cast<float>(i) / 32 * (2.0f * DX_PI_F);
            VECTOR p = { c.x + WANDER_RADIUS * cosf(a), c.y, c.z + WANDER_RADIUS * sinf(a) };
            if (i > 0) DrawLine3D(prevP, p, col);
            prevP = p;
        }
        DrawSphere3D(c, 30.0f, 12, col, col, false);
    }

    // 現在の徘徊目標（黄色の球 + 自機→目標の線）
    {
        unsigned int col = GetColor(255, 220, 40);
        VECTOR t = wanderTarget_; t.y += 5.0f;
        DrawSphere3D(t, 50.0f, 16, col, col, false);
        VECTOR from = transform_.pos; from.y += EYE_HEIGHT;
        VECTOR to = wanderTarget_;  to.y += EYE_HEIGHT;
        DrawLine3D(from, to, col);
    }

    // 壁法線（オレンジ）：壁に接触したフレームだけ表示
    if (VSize(wallNormalXZ_) > 0.0001f)
    {
        VECTOR from = transform_.pos; from.y += EYE_HEIGHT;
        DrawLine3D(from, VAdd(from, VScale(wallNormalXZ_, 150.0f)),
            GetColor(255, 128, 0));
    }

    // 実際の進行方向（水色）：壁スライド後の方向なので壁沿いに向いていれば正常
    if (VSize(moveDir_) > 0.0001f)
    {
        VECTOR from = transform_.pos; from.y += EYE_HEIGHT + 20.0f;
        DrawLine3D(from, VAdd(from, VScale(moveDir_, 200.0f)),
            GetColor(0, 255, 255));
    }
}
#endif