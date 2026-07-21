#include "../../../Application.h"
#include "../../../Manager/ResourceManager.h"
#include "../../../Common/AnimationController.h"
#include "../../../Utility/Math.h"
#include "../../../Component/CapsuleComponent.h"
#include "../../../Component/StaticMeshComponent.h"
#include "../../../Collision/CollisionManager.h"
#include "../../../Collision/HitResult.h"
#include "../../Scene/SceneManager.h"
#include "CharactorBase.h"

CharactorBase::CharactorBase(void)
    : ActorBase()
{
}

CharactorBase::~CharactorBase(void)
{
}

void CharactorBase::Init(void)
{
    ActorBase::Init();
}

void CharactorBase::Update(void)
{
    // 移動前座標を保存
    prevPos_ = GetPos();

    // キャラクターごとの更新
    UpdateProcess();

    // 向きを更新
    Rotate();

    // 重力を加算
    CalcGravityPow();

    // コライダーサイズ調整
    CollisionReserve();

    // 位置に移動量・ジャンプ量を反映
    Collision();

    // アニメーション更新
    if (animCtrl_ != nullptr) animCtrl_->Update();

    // キャラクターごとの更新後処理
    UpdateProcessPost();

    ActorBase::Update();
}

void CharactorBase::Draw(void)
{
    ActorBase::Draw();
}

void CharactorBase::Release(void)
{
    if (animCtrl_ != nullptr)
    {
        animCtrl_->Release();
        delete animCtrl_;
        animCtrl_ = nullptr;
    }
    ActorBase::Release();
}

// ---------------------------------------------------------------
// 当たり判定通知（CollisionManager から呼ばれる）
// ---------------------------------------------------------------

void CharactorBase::OnHit(const HitResult& hit)
{
    if (!hit.isHit) return;

    CollisionChannel ch = hit.otherCollider->GetChannel();

    if (hit.isBlocking &&
        (ch == CollisionChannel::WORLD_STATIC ||
            ch == CollisionChannel::WORLD_DYNAMIC))
    {
        PushBackGravity(hit);
        PushBackCapsule(hit);
    }
}

void CharactorBase::OnOverlap(const HitResult& hit)
{
    // 派生クラスで必要に応じて実装する
}

// ---------------------------------------------------------------
// 押し戻し処理
// ---------------------------------------------------------------

void CharactorBase::PushBackGravity(const HitResult& hit)
{
    // XY 平面化後は上方向が Y 固定
    Vector3 upDir = Vector3::UP;

    // 上昇中は地面との押し戻しをしない
    float upSpeed = Vector3::Dot(jumpPow_, upDir);
    if (upSpeed > 0.0f) return;

    auto* mesh = dynamic_cast<StaticMeshComponent*>(hit.otherCollider);
    if (mesh == nullptr) return;

    // 接地判定：ジャンプ量をリセット
    isJump_ = false;
    jumpPow_ = Vector3::ZERO;
    stepJump_ = 0.0f;

    SetPos(Vector3::FromVECTOR(hit.pushBackPos));
}

void CharactorBase::PushBackCapsule(const HitResult& hit)
{
    auto* mesh = dynamic_cast<StaticMeshComponent*>(hit.otherCollider);
    if (mesh == nullptr) return;

    SetPos(Vector3::FromVECTOR(hit.pushBackPos));
}

// ---------------------------------------------------------------
// 移動・回転・重力
// ---------------------------------------------------------------

void CharactorBase::ChangeState(int state)
{
    stateBase_ = state;
    stateChanges_[stateBase_]();
}

void CharactorBase::Rotate(void)
{
    if (faceDir_.IsNearlyZero()) return;

    // XY 平面化後は上方向が Y 固定
    SetRot(Quaternion::LookRotation(faceDir_, Vector3::UP));
}

void CharactorBase::CalcGravityPow(void)
{
    // XY 平面化後は重力方向が -Y 固定
    Vector3 dirGravity = -Vector3::UP;

    float gravityPow = Application::GetInstance().GetGravityPow()
        * scnMng_.GetDeltaTime();

    jumpPow_ = jumpPow_ + dirGravity * gravityPow;

    // 終端速度クランプ（落下速度の上限）
    float fallSpeed = Vector3::Dot(jumpPow_, dirGravity);
    if (fallSpeed > MAX_FALL_SPEED)
    {
        Vector3 lateral = jumpPow_ - dirGravity * fallSpeed;
        jumpPow_ = lateral + dirGravity * MAX_FALL_SPEED;
    }
}

void CharactorBase::Collision(void)
{
    SetPos(GetPos() + movePow_);
    SetPos(GetPos() + jumpPow_);
}