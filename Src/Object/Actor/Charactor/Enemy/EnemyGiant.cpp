#include "../../../../Utility/AsoUtility.h"
#include "../../../../Manager/ResourceManager.h"
#include "../../../../Scene/SceneManager.h"
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

    viewRangeTransform_.SetModel(
        resMng_.LoadModelDuplicate(ResourceManager::SRC::VIEW_RANGE));
}

void EnemyGiant::InitTransform(void)
{
    transform_.scl = VScale(AsoUtility::VECTOR_ONE, SCALE);
    transform_.quaRot = Quaternion();
    transform_.quaRotLocal = Quaternion::Euler(DEFAULT_LOCAL_ROT);
    transform_.pos = defaultPos_;

    // 地面にめり込まないよう +Y に少し持ち上げる
    transform_.pos.y += 10.0f;

    transform_.Update();

    // 以下 viewRange の初期化はそのまま
    viewRangeTransform_.scl = VIEW_RANGE_SCL;
    viewRangeTransform_.pos =
        MV1GetFramePosition(transform_.modelId, VIEW_RANGE_SYNC_FRAME);
    viewRangeTransform_.quaRot =
        transform_.quaRot.Mult(
            Quaternion::AngleAxis(VIEW_RANGE_ROT_X, AsoUtility::AXIS_X));
    viewRangeTransform_.quaRotLocal =
        Quaternion::AngleAxis(VIEW_RANGE_LOCAL_ROT_X, AsoUtility::AXIS_X);
    viewRangeTransform_.Update();

   // spawnPos_ = defaultPos_;
}

void EnemyGiant::InitCollider(void)
{
    // 地面衝突用線分
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

    // 視野モデルコライダ
    MV1SetupCollInfo(viewRangeTransform_.modelId);
    ColliderModel* colView =
        new ColliderModel(ColliderBase::TAG::VIEW_RANGE, &viewRangeTransform_);
    ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::VIEW_RANGE), colView);
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

    // 視野モデル同期
    viewRangeTransform_.pos =
        MV1GetFramePosition(transform_.modelId, VIEW_RANGE_SYNC_FRAME);
    viewRangeTransform_.quaRot =
        transform_.quaRot.Mult(
            Quaternion::AngleAxis(VIEW_RANGE_ROT_X, AsoUtility::AXIS_X));
    viewRangeTransform_.Update();

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

    // XZ平面上でランダム方向
    float angle = static_cast<float>(GetRand(360)) * DX_PI_F / 180.0f;
    moveDir_ = { cosf(angle), 0.0f, sinf(angle) };
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

    if (InSearchConeModel())
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

    // XZ平面上に矯正（Y成分を捨てる）
    moveDir_.y = 0.0f;
    if (VSize(moveDir_) < 0.0001f) moveDir_ = AsoUtility::DIR_F;
    moveDir_ = VNorm(moveDir_);

    faceDir_ = moveDir_;
    movePow_ = VScale(moveDir_, moveSpeed_);

    if (InSearchConeModel()) { ChangeState(STATE::CHASE); }
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
    // 攻撃アニメーション終了で次の状態へ
    if (animCtrl_->IsEnd())
    {
        float dist = DistToPlayer();
        if (dist <= DIST_ATTACK)
        {
            ChangeState(STATE::ATTACK);  // 連続攻撃
        }
        else if (dist > DIST_CHASE)
        {
            ChangeState(STATE::RETURN);
        }
        else
        {
            ChangeState(STATE::CHASE);
        }
        return;
    }

    // 手フレームの球体コライダとプレイヤーカプセルの当たり判定
    if (!attackHit_)
    {
        // 手フレームのワールド座標取得
        VECTOR handPos = MV1GetFramePosition(transform_.modelId, attackHandFrame_);

        for (const auto& hitCol : hitColliders_)
        {
            if (hitCol->GetTag() != ColliderBase::TAG::PLAYER) continue;

            const ColliderCapsule* playerCapsule =
                dynamic_cast<const ColliderCapsule*>(hitCol);
            if (playerCapsule == nullptr) continue;

            // 手の球体とプレイヤーカプセルの当たり判定
            VECTOR capTop = playerCapsule->GetPosTop();
            VECTOR capDown = playerCapsule->GetPosDown();
            float  capR = playerCapsule->GetRadius();

            if (AsoUtility::IsHitSphereCapsule(
                handPos, ATTACK_SPHERE_RADIUS,
                capTop, capDown, capR))
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
    if (InSearchConeModel())
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
    VECTOR dir = VSub(target, transform_.pos);
    dir.y = 0.0f;  // XZ平面へ射影

    // ターゲットがほぼ真上/真下なら無視
    if (VSize(dir) < 0.01f) return;

    moveDir_ = VNorm(dir);
    faceDir_ = moveDir_;
}

bool EnemyGiant::InSearchConeModel(void)
{
    int viewRangeType = static_cast<int>(COLLIDER_TYPE::VIEW_RANGE);
    if (ownColliders_.count(viewRangeType) == 0) return false;

    ColliderModel* colliderModel =
        dynamic_cast<ColliderModel*>(ownColliders_.at(viewRangeType));
    if (colliderModel == nullptr) return false;

    MV1RefreshCollInfo(colliderModel->GetFollow()->modelId);

    for (const auto& hitCol : hitColliders_)
    {
        if (hitCol->GetTag() != ColliderBase::TAG::PLAYER) continue;

        const ColliderCapsule* colCapsule =
            dynamic_cast<const ColliderCapsule*>(hitCol);
        if (colCapsule == nullptr) continue;

        if (colCapsule->IsHit(colliderModel, false, false))
        {
            return true;
        }
    }
    return false;
}

float EnemyGiant::DistToPlayer(void) const
{
    return VSize(VSub(player_.GetTransform().pos, transform_.pos));
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

        // XZ 距離のみ
        float dx = myC.x - plC.x;
        float dz = myC.z - plC.z;
        float distSq = dx * dx + dz * dz;

        float minDist = myCapsule->GetRadius() + playerCapsule->GetRadius();
        if (distSq >= minDist * minDist) return;

        float dist = sqrtf(distSq);
        if (dist < 0.0001f) { dx = 1.0f; dz = 0.0f; dist = 1.0f; }

        float overlap = minDist - dist;
        float k = overlap * 0.5f / dist;

        // 敵側のみ動かす（プレイヤーは既存挙動を維持）
        transform_.pos.x += dx * k;
        transform_.pos.z += dz * k;
        transform_.Update();
        return;
    }
}