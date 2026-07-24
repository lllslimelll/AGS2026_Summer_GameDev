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
    step_(0.0f),
    patrolCornerIdx_(0),
    idleTimer_(0.0f),
    attackHit_(false),
    attackHandFrame_(-1)
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
    // 視野モデルはロジックFOVに移行したのでロード不要
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
    // 地面判定用の線分
    ColliderLine* colLine = new ColliderLine(
        ColliderBase::TAG::ENEMY, &transform_,
        COL_LINE_START_LOCAL_POS, COL_LINE_END_LOCAL_POS);
    ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::GROUND_LINE), colLine);

    // 本体カプセル
    ColliderCapsule* colCapsule = new ColliderCapsule(
        ColliderBase::TAG::ENEMY, &transform_,
        COL_CAPSULE_TOP_LOCAL_POS, COL_CAPSULE_DOWN_LOCAL_POS,
        COL_CAPSULE_RADIUS);
    ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::CAPSULE), colCapsule);

    // 視野判定はロジックベースに変更したので、視野モデルコライダは作成しない
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

    animCtrl_->Play(static_cast<int>(ANIM_TYPE::IDLE), 1.2f, true);
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

    // 最初の角からパトロール開始
    patrolCornerIdx_ = 0;
    ChangeState(STATE::PATROL);

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

    // パトロール中に移動可能範囲を外れたら、その角を諦めて次の角へ
    // （CHASE / RETURN 中は範囲外への移動を許可する）
    if (state_ == STATE::PATROL && !InMovableRange())
    {
        transform_.pos = prevPos_;
        transform_.Update();
        AdvancePatrolCorner();
        ChangeState(STATE::PATROL);
    }

    // 敵とプレイヤーの押し戻し
    PushBackFromPlayer();
}

void EnemyGiant::Draw(void)
{
    EnemyBase::Draw();

#ifdef _DEBUG
    // 攻撃球のデバッグ描画
    if (attackHandFrame_ >= 0)
    {
        VECTOR spherePos = GetAttackSpherePos();
        DrawSphere3D(spherePos, ATTACK_SPHERE_RADIUS, 16, 0xff0000, 0xff0000, false);
    }

    // AI デバッグ描画（視野・パトロール経路など）
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

void EnemyGiant::ChangeStateThink(void)
{
    // 互換のため残しているが、新しいフローでは経由しない
    stateUpdate_ = std::bind(&EnemyGiant::UpdateThink, this);
    ChangeState(STATE::PATROL);
}

void EnemyGiant::ChangeStateIdle(void)
{
    stateUpdate_ = std::bind(&EnemyGiant::UpdateIdle, this);
    idleTimer_ = IDLE_AT_CORNER_TIME;
    movePow_ = AsoUtility::VECTOR_ZERO;
    animCtrl_->Play(static_cast<int>(ANIM_TYPE::IDLE), 1.4f,  true);
}

void EnemyGiant::ChangeStatePatrol(void)
{
    stateUpdate_ = std::bind(&EnemyGiant::UpdatePatrol, this);

    // 現在の角へ向ける
    VECTOR target = GetCurrentPatrolTarget();
    SetMoveDirToTarget(target);

    moveSpeed_ = SPEED_PATROL;
    animCtrl_->Play(static_cast<int>(ANIM_TYPE::WALK), 1.5f, true);
}

void EnemyGiant::ChangeStateChase(void)
{
    stateUpdate_ = std::bind(&EnemyGiant::UpdateChase, this);
    moveSpeed_ = SPEED_CHASE;
    animCtrl_->Play(static_cast<int>(ANIM_TYPE::RUN), 1.4f, true);
}

void EnemyGiant::ChangeStateAttack(void)
{
    stateUpdate_ = std::bind(&EnemyGiant::UpdateAttack, this);
    movePow_ = AsoUtility::VECTOR_ZERO;
    attackHit_ = false;

    // 攻撃開始時にプレイヤーの方を向く
    SetMoveDirToTarget(player_.GetTransform().pos);

    // 既に ATTACK 再生中だと Play() が効かないので、Replay で最初に戻す
    if (animCtrl_->GetPlayType() == static_cast<int>(ANIM_TYPE::ATTACK))
    {
        animCtrl_->Replay();
    }
    else
    {
        animCtrl_->Play(static_cast<int>(ANIM_TYPE::ATTACK), 1.1f, false);
    }
}

void EnemyGiant::ChangeStateReturn(void)
{
    stateUpdate_ = std::bind(&EnemyGiant::UpdateReturn, this);
    moveSpeed_ = SPEED_RETURN;
    animCtrl_->Play(static_cast<int>(ANIM_TYPE::WALK), 1.5f, true);
}

void EnemyGiant::ChangeStateEnd(void)
{
    stateUpdate_ = std::bind(&EnemyGiant::UpdateEnd, this);
}

// ===============================================================
// 各ステート更新
// ===============================================================
void EnemyGiant::UpdateNone(void) {}
void EnemyGiant::UpdateThink(void) {}
void EnemyGiant::UpdateEnd(void) {}

void EnemyGiant::UpdateIdle(void)
{
    movePow_ = AsoUtility::VECTOR_ZERO;

    // 待機中でもプレイヤーを見つけたら追跡へ
    if (IsPlayerInSight())
    {
        ChangeState(STATE::CHASE);
        return;
    }

    idleTimer_ -= scnMng_.GetDeltaTime();
    if (idleTimer_ <= 0.0f)
    {
        AdvancePatrolCorner();
        ChangeState(STATE::PATROL);
    }
}

void EnemyGiant::UpdatePatrol(void)
{
    // プレイヤー検知が最優先
    if (IsPlayerInSight())
    {
        ChangeState(STATE::CHASE);
        return;
    }

    // 現在の角へ移動
    VECTOR target = GetCurrentPatrolTarget();

    VECTOR delta = VSub(target, transform_.pos);
    delta.y = 0.0f;
    float distXZ = VSize(delta);

    if (distXZ <= DIST_ARRIVE)
    {
        // ドリフト防止に XZ をコーナーへスナップしてから IDLE へ
        transform_.pos.x = target.x;
        transform_.pos.z = target.z;
        transform_.Update();

        ChangeState(STATE::IDLE);
        return;
    }

    // 移動
    SetMoveDirToTarget(target);
    movePow_ = VScale(moveDir_, moveSpeed_);
}

void EnemyGiant::UpdateChase(void)
{
    float dist = DistToPlayer();

    if (dist > DIST_LOSE_CHASE)
    {
        ChangeState(STATE::RETURN);
        return;
    }

    if (dist <= DIST_ATTACK)
    {
        ChangeState(STATE::ATTACK);
        return;
    }

    // 障害物回避を含めた追跡方向
    VECTOR dir = ComputeChaseDir();
    moveDir_ = dir;
    faceDir_ = dir;
    movePow_ = VScale(moveDir_, moveSpeed_);
}

void EnemyGiant::UpdateAttack(void)
{
    // 現在のアニメーション進捗（0.0 ? 1.0）
    const auto& anim = animCtrl_->GetPlayAnim();
    float progress = (anim.totalTime > 0.0f) ? (anim.step / anim.totalTime) : 0.0f;

    // 当たり判定は指定区間（アニメ後半）のみ有効
    bool hitboxActive =
        !attackHit_ &&
        progress >= ATTACK_HIT_START &&
        progress <= ATTACK_HIT_END;

    if (hitboxActive)
    {
        // オフセット適用済みの攻撃球中心
        VECTOR spherePos = GetAttackSpherePos();

        for (const auto& hitCol : hitColliders_)
        {
            if (hitCol->GetTag() != ColliderBase::TAG::PLAYER) continue;

            const ColliderCapsule* playerCapsule =
                dynamic_cast<const ColliderCapsule*>(hitCol);
            if (playerCapsule == nullptr) continue;

            VECTOR capTop = playerCapsule->GetPosTop();
            VECTOR capDown = playerCapsule->GetPosDown();
            float  capR = playerCapsule->GetRadius();

            if (AsoUtility::IsHitSphereCapsule(
                spherePos, ATTACK_SPHERE_RADIUS,
                capTop, capDown, capR))
            {
                player_.OnDamagedByEnemy(ATTACK_DAMAGE);
                SoundManager::GetInstance().PlayDamaged();
       
                attackHit_ = true;
                break;
            }
        }
    }

    // アニメ終了時に次の状態を決める
    if (animCtrl_->IsEnd())
    {
        float dist = DistToPlayer();
        if (dist > DIST_LOSE_CHASE)   ChangeState(STATE::RETURN);
        else if (dist <= DIST_ATTACK) ChangeState(STATE::ATTACK); // 連続攻撃
        else                          ChangeState(STATE::CHASE);
    }
}

void EnemyGiant::UpdateReturn(void)
{
    // 帰還中でも視界に入ったら再交戦
    if (IsPlayerInSight())
    {
        ChangeState(STATE::CHASE);
        return;
    }

    // 初期位置到達判定
    VECTOR delta = VSub(defaultPos_, transform_.pos);
    delta.y = 0.0f;
    if (VSize(delta) <= DIST_ARRIVE)
    {
        transform_.pos.x = defaultPos_.x;
        transform_.pos.z = defaultPos_.z;
        transform_.Update();

        patrolCornerIdx_ = 0;
        ChangeState(STATE::PATROL);
        return;
    }

    SetMoveDirToTarget(defaultPos_);
    movePow_ = VScale(moveDir_, moveSpeed_);
}

// ===============================================================
// ヘルパー
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

// ---- パトロールの角（+Y から見て時計回り） ----------------------
// 0: (+X, +Z)  1: (+X, -Z)  2: (-X, -Z)  3: (-X, +Z)
VECTOR EnemyGiant::GetPatrolCornerPos(int index) const
{
    VECTOR p = defaultPos_;
    switch (((index % 4) + 4) % 4)
    {
    case 0: p.x += PATROL_HALF; p.z += PATROL_HALF; break;
    case 1: p.x += PATROL_HALF; p.z -= PATROL_HALF; break;
    case 2: p.x -= PATROL_HALF; p.z -= PATROL_HALF; break;
    case 3: p.x -= PATROL_HALF; p.z += PATROL_HALF; break;
    }
    return p;
}

VECTOR EnemyGiant::GetCurrentPatrolTarget(void) const
{
    return GetPatrolCornerPos(patrolCornerIdx_);
}

void EnemyGiant::AdvancePatrolCorner(void)
{
    patrolCornerIdx_ = (patrolCornerIdx_ + 1) % 4;
}

// ---- ロジック視界判定（FOV円錐 + LOSレイキャスト） --------------
bool EnemyGiant::IsPlayerInSight(void)
{
    VECTOR myPos = transform_.pos;
    VECTOR plPos = player_.GetTransform().pos;

    VECTOR toPl = VSub(plPos, myPos);
    toPl.y = 0.0f;
    float dist = VSize(toPl);

    // 距離チェック
    if (dist > VIEW_RANGE) return false;
    if (dist < 0.0001f)    return true;

    VECTOR toPlDir = VScale(toPl, 1.0f / dist);

    // FOV円錐チェック（前方ベクトルとの内積を半視野角の cos と比較）
    VECTOR fwd = faceDir_;
    fwd.y = 0.0f;
    if (VSize(fwd) < 0.0001f) return false;
    fwd = VNorm(fwd);

    float cosAngle = VDot(fwd, toPlDir);
    float cosHalfFov = cosf(VIEW_HALF_FOV_RAD);
    if (cosAngle < cosHalfFov) return false;

    // 遮蔽チェック（地形の Occluder で視線が遮られたら見えない扱い）
    VECTOR eye = myPos; eye.y += EYE_HEIGHT;
    VECTOR chest = plPos; chest.y += EYE_HEIGHT;
    if (IsOccludedByColliders(eye, chest)) return false;

    return true;
}

// ---- 障害物回避を含めた追跡方向の決定 ---------------------------
VECTOR EnemyGiant::ComputeChaseDir(void)
{
    VECTOR myPos = transform_.pos;
    VECTOR plPos = player_.GetTransform().pos;

    // プレイヤーへの基準方向
    VECTOR base = VSub(plPos, myPos);
    base.y = 0.0f;
    float baseLen = VSize(base);
    if (baseLen < 0.001f) return faceDir_;
    base = VScale(base, 1.0f / baseLen);

    VECTOR probeFrom = myPos; probeFrom.y += EYE_HEIGHT;

    // 指定方向にレイを飛ばして遮蔽されているか
    auto probe = [&](const VECTOR& dir) -> bool {
        VECTOR to = VAdd(probeFrom, VScale(dir, AVOID_PROBE_DIST));
        return IsOccludedByColliders(probeFrom, to);
        };

    // 前方が空いてればそのまま進む
    if (!probe(base)) return base;

    // 左右 ±45度 を試行
    Quaternion rotL = Quaternion::AngleAxis(+AVOID_ANGLE_RAD, AsoUtility::AXIS_Y);
    Quaternion rotR = Quaternion::AngleAxis(-AVOID_ANGLE_RAD, AsoUtility::AXIS_Y);
    VECTOR dirL = rotL.PosAxis(base);
    VECTOR dirR = rotR.PosAxis(base);

    bool blockedL = probe(dirL);
    bool blockedR = probe(dirR);

    if (!blockedL && !blockedR)
    {
        // 両方空いているならプレイヤーに近い側を選ぶ
        VECTOR endL = VAdd(myPos, VScale(dirL, AVOID_PROBE_DIST));
        VECTOR endR = VAdd(myPos, VScale(dirR, AVOID_PROBE_DIST));
        float dL = VSize(VSub(plPos, endL));
        float dR = VSize(VSub(plPos, endR));
        return (dL <= dR) ? dirL : dirR;
    }
    if (!blockedL) return dirL;
    if (!blockedR) return dirR;

    // まだ詰まっているので ±90度 を試行
    Quaternion rotWL = Quaternion::AngleAxis(+AVOID_ANGLE_WIDE_RAD, AsoUtility::AXIS_Y);
    Quaternion rotWR = Quaternion::AngleAxis(-AVOID_ANGLE_WIDE_RAD, AsoUtility::AXIS_Y);
    VECTOR dirWL = rotWL.PosAxis(base);
    VECTOR dirWR = rotWR.PosAxis(base);

    if (!probe(dirWL)) return dirWL;
    if (!probe(dirWR)) return dirWR;

    // 完全に詰まっている場合は基準方向のまま押し込む（衝突で止まる）
    return base;
}

void EnemyGiant::PushBackFromPlayer(void)
{
    int capsuleType = static_cast<int>(COLLIDER_TYPE::CAPSULE);
    if (ownColliders_.count(capsuleType) == 0) return;

    ColliderCapsule* myCapsule = dynamic_cast<ColliderCapsule*>(ownColliders_.at(capsuleType));
    if (myCapsule == nullptr) return;

    for (const auto& hitCol : hitColliders_)
    {
        if (hitCol->GetTag() != ColliderBase::TAG::PLAYER) continue;

        const ColliderCapsule* playerCapsule = dynamic_cast<const ColliderCapsule*>(hitCol);
        if (playerCapsule == nullptr) continue;

        VECTOR myC = myCapsule->GetCenter();
        VECTOR plC = playerCapsule->GetCenter();

        // XZ 平面のみで押し戻し（Y は落下等に干渉させない）
        float dx = myC.x - plC.x;
        float dz = myC.z - plC.z;
        float distSq = dx * dx + dz * dz;

        float minDist = myCapsule->GetRadius() + playerCapsule->GetRadius();
        if (distSq >= minDist * minDist) return;

        float dist = sqrtf(distSq);
        if (dist < 0.0001f) { dx = 1.0f; dz = 0.0f; dist = 1.0f; }

        float overlap = minDist - dist;
        float k = overlap * 0.5f / dist;

        transform_.pos.x += dx * k;
        transform_.pos.z += dz * k;
        transform_.Update();
        return;
    }
}

// 攻撃球の中心位置を取得
// フレーム位置に、敵の向きを反映したローカルオフセットを加算する
VECTOR EnemyGiant::GetAttackSpherePos(void) const
{
    VECTOR framePos = MV1GetFramePosition(transform_.modelId, attackHandFrame_);
    // ローカルオフセットを敵の向きに合わせてワールド座標系へ変換
    VECTOR worldOffset = transform_.quaRot.PosAxis(ATTACK_SPHERE_OFFSET_LOCAL);
    return VAdd(framePos, worldOffset);
}

#ifdef _DEBUG
void EnemyGiant::DrawDebugAI(void)
{
    VECTOR pos = transform_.pos;
    pos.y += EYE_HEIGHT;

    VECTOR fwd = faceDir_;
    fwd.y = 0.0f;
    if (VSize(fwd) < 0.0001f) fwd = AsoUtility::DIR_F;
    fwd = VNorm(fwd);

    // プレイヤー検知中は赤、非検知は緑
    unsigned int color = IsPlayerInSight() ? GetColor(255, 40, 40) : GetColor(40, 255, 40);

    // 視野円錐の左右エッジ線
    Quaternion rotL = Quaternion::AngleAxis(+VIEW_HALF_FOV_RAD, AsoUtility::AXIS_Y);
    Quaternion rotR = Quaternion::AngleAxis(-VIEW_HALF_FOV_RAD, AsoUtility::AXIS_Y);
    VECTOR dirL = rotL.PosAxis(fwd);
    VECTOR dirR = rotR.PosAxis(fwd);

    VECTOR endF = VAdd(pos, VScale(fwd, VIEW_RANGE));
    VECTOR endL = VAdd(pos, VScale(dirL, VIEW_RANGE));
    VECTOR endR = VAdd(pos, VScale(dirR, VIEW_RANGE));

    DrawLine3D(pos, endF, color);
    DrawLine3D(pos, endL, color);
    DrawLine3D(pos, endR, color);

    // 視野円錐の前面弧
    const int SEG = 24;
    VECTOR prev = endL;
    for (int i = 1; i <= SEG; ++i)
    {
        float t = static_cast<float>(i) / SEG;
        float ang = -VIEW_HALF_FOV_RAD + 2.0f * VIEW_HALF_FOV_RAD * t;
        Quaternion rot = Quaternion::AngleAxis(ang, AsoUtility::AXIS_Y);
        VECTOR d = rot.PosAxis(fwd);
        VECTOR e = VAdd(pos, VScale(d, VIEW_RANGE));
        DrawLine3D(prev, e, color);
        prev = e;
    }

    // パトロール経路（青い正方形）
    unsigned int patrolCol = GetColor(80, 160, 255);
    VECTOR c0 = GetPatrolCornerPos(0);
    VECTOR c1 = GetPatrolCornerPos(1);
    VECTOR c2 = GetPatrolCornerPos(2);
    VECTOR c3 = GetPatrolCornerPos(3);
    // 線が地面に埋まらないように少し持ち上げる
    c0.y += 5.0f; c1.y += 5.0f; c2.y += 5.0f; c3.y += 5.0f;
    DrawLine3D(c0, c1, patrolCol);
    DrawLine3D(c1, c2, patrolCol);
    DrawLine3D(c2, c3, patrolCol);
    DrawLine3D(c3, c0, patrolCol);

    // 現在の目標コーナーをハイライト
    VECTOR cur = GetCurrentPatrolTarget();
    DrawSphere3D(cur, 60.0f, 16, patrolCol, patrolCol, false);
}
#endif