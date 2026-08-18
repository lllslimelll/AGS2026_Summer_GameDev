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
    attackHit_(false),
    attackHandFrame_(-1),
    isAware_(false),
    wanderDir_(AsoUtility::DIR_F),
    wanderTimer_(0.0f),
    idleTimer_(0.0f),
    // インスタンス固有シード：this のアドレスを XOR して
    // 複数体が同フレームで生成されても被らないようにする
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

    // 押し戻し後の位置から壁法線を推定（次フレームの ApplyWallSlide 用）
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

    // 新しいランダム方向と時間を選ぶ
    PickNewWanderDirection();

    // 前方が塞がっていたら空いてる方向に振り直す
    if (!IsDirectionClear(wanderDir_)) ReorientToClearDirection();

    moveDir_ = wanderDir_;
    faceDir_ = wanderDir_;

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

    if (!isAware_)
    {
        SoundManager::GetInstance().PlayFound();
        isAware_ = true;
    }
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
        ChangeState(STATE::WANDER);
    }
}

// ---------------------------------------------------------------
// 徘徊：ランダム方向にランダム時間だけ歩く。
// 前方に障害物を検知したら方向を振り直す。時間切れで IDLE へ。
// ---------------------------------------------------------------
void EnemyGiant::UpdateWander(void)
{
    if (IsPlayerInSight())
    {
        ChangeState(STATE::CHASE);
        return;
    }

    // 時間切れなら待機へ
    wanderTimer_ -= scnMng_.GetDeltaTime();
    if (wanderTimer_ <= 0.0f)
    {
        ChangeState(STATE::IDLE);
        return;
    }

    // 現在の方向が塞がれていたら振り直す。
    // 見つからない場合も次フレームまた試すのでその場で足踏みするだけ。
    if (!IsDirectionClear(wanderDir_))
    {
        if (!ReorientToClearDirection())
        {
            // どの方向も塞がっている：この場で足踏み → タイマー消化で IDLE へ
            movePow_ = AsoUtility::VECTOR_ZERO;
            return;
        }
    }

    // 詰まったら方向を強制的に振り直す
    if (IsStuck())
    {
        PickNewWanderDirection();
        ReorientToClearDirection();
    }

    moveDir_ = wanderDir_;
    faceDir_ = wanderDir_;

    ApplyWallSlide();
    LimitTurnRate();
    movePow_ = VScale(moveDir_, moveSpeed_);
}

void EnemyGiant::UpdateChase(void)
{
    float dist = DistToPlayer();

    // 見失った：その場で徘徊に戻る（座標を持たないので特別な処理不要）
    if (dist > DIST_LOSE_CHASE)
    {
        isAware_ = false;
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
            isAware_ = false;                       // ← 追加
            ChangeState(STATE::WANDER);
        }
        else if (dist <= DIST_ATTACK) ChangeState(STATE::ATTACK);
        else                          ChangeState(STATE::CHASE);
    }
}

// ===============================================================
// ヘルパー：徘徊
// ===============================================================

// ランダムな XZ 方向とランダムな歩行時間を設定する。
//
// 方向は極座標で θ ∈ [0, 2π) を抽選し、
//   dir = (cosθ, 0, sinθ)
// で XZ 平面上の単位ベクトルを作る。
void EnemyGiant::PickNewWanderDirection(void)
{
    float theta = RandAngle();
    wanderDir_.x = cosf(theta);
    wanderDir_.y = 0.0f;
    wanderDir_.z = sinf(theta);

    wanderTimer_ = RandFloat(WANDER_TIME_MIN, WANDER_TIME_MAX);
}

// 指定方向にレイを飛ばして遮蔽物がないか確認する。
bool EnemyGiant::IsDirectionClear(const VECTOR& dir) const
{
    VECTOR from = transform_.pos; from.y += EYE_HEIGHT;
    VECTOR to = VAdd(from, VScale(dir, WANDER_PROBE_DIST));

    return !const_cast<EnemyGiant*>(this)->IsOccludedByColliders(from, to);
}

// 現在の wanderDir_ を軸に左右へ WANDER_TURN_STEP_DEG ずつ振って、
// 最初に空いた方向を採用する。
//
// 探索順は +ステップ, -ステップ, +2ステップ, -2ステップ, ...
// これで壁に対して左右対称に扇状に探すので、
// 「わずかにズレた方向」を優先して選べる。
bool EnemyGiant::ReorientToClearDirection(void)
{
    const float stepRad = WANDER_TURN_STEP_DEG * DX_PI_F / 180.0f;

    for (int step = 1; step <= WANDER_TURN_MAX_STEP; ++step)
    {
        // +step 方向
        {
            Quaternion rot = Quaternion::AngleAxis(+stepRad * step, AsoUtility::AXIS_Y);
            VECTOR cand = rot.PosAxis(wanderDir_);
            if (IsDirectionClear(cand))
            {
                wanderDir_ = cand;
                return true;
            }
        }
        // -step 方向
        {
            Quaternion rot = Quaternion::AngleAxis(-stepRad * step, AsoUtility::AXIS_Y);
            VECTOR cand = rot.PosAxis(wanderDir_);
            if (IsDirectionClear(cand))
            {
                wanderDir_ = cand;
                return true;
            }
        }
    }

    return false;
}

// ===============================================================
// ヘルパー：乱数
// ===============================================================

// std::mt19937（メルセンヌ・ツイスタ）
// インスタンス固有シードで独立した乱数列を持つ。
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

    // ヒステリシス
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
        VECTOR endL = VAdd(myPos, VScale(dirL, AVOID_PROBE_DIST));
        VECTOR endR = VAdd(myPos, VScale(dirR, AVOID_PROBE_DIST));
        float  dL = VSize(VSub(plPos, endL));
        float  dR = VSize(VSub(plPos, endR));
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
void EnemyGiant::ApplyWallSlide(void)
{
    if (VSize(wallNormalXZ_) < 0.0001f) return;

    float vn = VDot(moveDir_, wallNormalXZ_);
    if (vn >= 0.0f) return;

    moveDir_ = VSub(moveDir_, VScale(wallNormalXZ_, vn));

    float len = VSize(moveDir_);
    if (len > 0.0001f)
    {
        moveDir_ = VScale(moveDir_, 1.0f / len);
    }
    else
    {
        // 壁法線を 90 度回転した接線を採用
        moveDir_.x = -wallNormalXZ_.z;
        moveDir_.y = 0.0f;
        moveDir_.z = wallNormalXZ_.x;
    }

    faceDir_ = moveDir_;
}

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

    // 視野円錐
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

    // 徘徊方向とプローブ距離（黄色）
    if (state_ == STATE::WANDER && VSize(wanderDir_) > 0.0001f)
    {
        VECTOR from = transform_.pos; from.y += EYE_HEIGHT;
        VECTOR to = VAdd(from, VScale(wanderDir_, WANDER_PROBE_DIST));
        unsigned int col = IsDirectionClear(wanderDir_)
            ? GetColor(255, 220, 40)   // 通れる
            : GetColor(255, 80, 80);  // 塞がれてる
        DrawLine3D(from, to, col);
    }

    // 壁法線（オレンジ）
    if (VSize(wallNormalXZ_) > 0.0001f)
    {
        VECTOR from = transform_.pos; from.y += EYE_HEIGHT;
        DrawLine3D(from, VAdd(from, VScale(wallNormalXZ_, 150.0f)),
            GetColor(255, 128, 0));
    }

    // 実際の進行方向（水色）
    if (VSize(moveDir_) > 0.0001f)
    {
        VECTOR from = transform_.pos; from.y += EYE_HEIGHT + 20.0f;
        DrawLine3D(from, VAdd(from, VScale(moveDir_, 200.0f)),
            GetColor(0, 255, 255));
    }
}
#endif