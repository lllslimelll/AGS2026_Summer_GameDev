#include "../Game/Actor/ActorBase.h"
#include "ColliderBase.h"
#include "ColliderCapsule.h"
#include "ColliderModel.h"
#include "ColliderSphere.h"
#include "CollisionManager.h"

CollisionManager* CollisionManager::instance_ = nullptr;

void CollisionManager::CreateInstance(void)
{
    if (instance_ == nullptr)
    {
        instance_ = new CollisionManager();
    }
}

CollisionManager& CollisionManager::GetInstance(void)
{
    if (instance_ == nullptr)
    {
        CreateInstance();
    }
    return *instance_;
}

void CollisionManager::Destroy(void)
{
    delete instance_;
    instance_ = nullptr;
}

// ---------------------------------------------------------------
// シーン切り替え時に全登録をクリア
// ---------------------------------------------------------------
void CollisionManager::Clear(void)
{
    colliders_.clear();
    hitResults_.clear();
}

// ---------------------------------------------------------------
// コライダーを登録する
// ---------------------------------------------------------------
void CollisionManager::Register(ColliderBase* collider)
{
    // 重複登録を防ぐ
    auto it = std::find(colliders_.begin(), colliders_.end(), collider);
    if (it == colliders_.end())
    {
        colliders_.push_back(collider);
    }
}

// ---------------------------------------------------------------
// コライダーの登録を解除する
// ---------------------------------------------------------------
void CollisionManager::Unregister(ColliderBase* collider)
{
    auto it = std::find(colliders_.begin(), colliders_.end(), collider);
    if (it != colliders_.end())
    {
        colliders_.erase(it);
    }
}

// ---------------------------------------------------------------
// 毎フレーム全ての当たり判定を実行する
// ---------------------------------------------------------------
void CollisionManager::Update(void)
{
    // 前フレームの結果をクリア
    hitResults_.clear();

    // 全コライダーのペアを判定
    for (int i = 0; i < (int)colliders_.size(); i++)
    {
        for (int j = i + 1; j < (int)colliders_.size(); j++)
        {
            ColliderBase* a = colliders_[i];
            ColliderBase* b = colliders_[j];

            // どちらかが無効なら判定しない
            if (!a->IsValid() || !b->IsValid()) continue;

            // 同じ Actor のコライダー同士は判定しない
            if (a->GetOwner() == b->GetOwner()) continue;

            // 優先度方式でレスポンスを決定
            CollisionResponse response = ResolveResponse(a, b);

            // IGNORE なら判定しない
            if (response == CollisionResponse::NONE) continue;

            // 判定を実行
            Solve(a, b);
        }
    }
}

// ---------------------------------------------------------------
// HitResult を取得する
// ---------------------------------------------------------------
std::vector<HitResult> CollisionManager::GetHits(
    const ActorBase* actor) const
{
    std::vector<HitResult> result;
    for (const auto& hit : hitResults_)
    {
        if (hit.selfCollider->GetOwner() == actor ||
            hit.otherCollider->GetOwner() == actor)
        {
            result.push_back(hit);
        }
    }
    return result;
}

std::vector<HitResult> CollisionManager::GetBlockHits(
    const ActorBase* actor) const
{
    std::vector<HitResult> result;
    for (const auto& hit : hitResults_)
    {
        if (hit.response != CollisionResponse::BLOCK) continue;
        if (hit.selfCollider->GetOwner() == actor ||
            hit.otherCollider->GetOwner() == actor)
        {
            result.push_back(hit);
        }
    }
    return result;
}

std::vector<HitResult> CollisionManager::GetOverlapHits(
    const ActorBase* actor) const
{
    std::vector<HitResult> result;
    for (const auto& hit : hitResults_)
    {
        if (hit.response != CollisionResponse::OVERLAP) continue;
        if (hit.selfCollider->GetOwner() == actor ||
            hit.otherCollider->GetOwner() == actor)
        {
            result.push_back(hit);
        }
    }
    return result;
}

// ---------------------------------------------------------------
// 優先度方式でレスポンスを決定する
// BLOCK > OVERLAP > IGNORE
// ---------------------------------------------------------------
CollisionResponse CollisionManager::ResolveResponse(
    const ColliderBase* a,
    const ColliderBase* b) const
{
    CollisionResponse responseA =
        a->GetProfile().GetResponse(b->GetChannel());
    CollisionResponse responseB =
        b->GetProfile().GetResponse(a->GetChannel());

    // 値の大きいほうを採用
    return static_cast<CollisionResponse>(
        max(
            static_cast<int>(responseA),
            static_cast<int>(responseB)));
}

// ---------------------------------------------------------------
// 2つのコライダーの判定を実行
// ---------------------------------------------------------------
void CollisionManager::Solve(ColliderBase* a, ColliderBase* b)
{
    CollisionResponse response = ResolveResponse(a, b);

    // 形状の組み合わせで判定を分岐
    if (a->GetShape() == ColliderBase::SHAPE::CAPSULE &&
        b->GetShape() == ColliderBase::SHAPE::MODEL)
    {
        SolveCapsuleModel(a, b);
        return;
    }
    if (a->GetShape() == ColliderBase::SHAPE::MODEL &&
        b->GetShape() == ColliderBase::SHAPE::CAPSULE)
    {
        SolveCapsuleModel(b, a);
        return;
    }
    if (a->GetShape() == ColliderBase::SHAPE::SPHERE &&
        b->GetShape() == ColliderBase::SHAPE::SPHERE)
    {
        SolveSphereVsSphere(a, b);
        return;
    }
    if (a->GetShape() == ColliderBase::SHAPE::CAPSULE &&
        b->GetShape() == ColliderBase::SHAPE::CAPSULE)
    {
        SolveCapsuleVsCapsule(a, b);
        return;
    }
}

// ---------------------------------------------------------------
// Capsule vs Model の判定
// ---------------------------------------------------------------
void CollisionManager::SolveCapsuleModel(
    ColliderBase* capsuleBase, ColliderBase* modelBase)
{
    auto* capsule = static_cast<ColliderCapsule*>(capsuleBase);
    auto* model = static_cast<ColliderModel*>(modelBase);

    CollisionResponse response = ResolveResponse(capsuleBase, modelBase);

    auto hits = MV1CollCheck_Capsule(
        model->GetOwner()->GetTransform().modelId, -1,
        capsule->GetPosTop(),
        capsule->GetPosDown(),
        capsule->GetRadius());

    for (int i = 0; i < hits.HitNum; i++)
    {
        HitResult hit;
        hit.selfCollider = capsuleBase;
        hit.otherCollider = modelBase;
        hit.otherActor = modelBase->GetOwner();
        hit.response = response;
        hit.isHit = true;
        hit.isBlocking = (response == CollisionResponse::BLOCK);
        hit.normal = hits.Dim[i].Normal;
        hit.point = hits.Dim[i].Position[0];

        // BLOCK の場合は押し戻し座標を計算
        if (response == CollisionResponse::BLOCK)
        {
            hit.pushBackPos = capsule->GetPosPushBackAlongNormal(
                hits.Dim[i], 10, 1.0f);
        }

        hitResults_.push_back(hit);
        break; // 最初の衝突だけ取る
    }

    MV1CollResultPolyDimTerminate(hits);
}

// ---------------------------------------------------------------
// Sphere vs Sphere の判定
// ---------------------------------------------------------------
void CollisionManager::SolveSphereVsSphere(
    ColliderBase* aBase, ColliderBase* bBase)
{
    auto* a = static_cast<ColliderSphere*>(aBase);
    auto* b = static_cast<ColliderSphere*>(bBase);

    CollisionResponse response = ResolveResponse(aBase, bBase);

    VECTOR posA = a->GetPos();
    VECTOR posB = b->GetPos();
    VECTOR diff = VSub(posB, posA);

    float distSq = VDot(diff, diff);
    float radiusSum = a->GetRadius() + b->GetRadius();

    if (distSq >= radiusSum * radiusSum) return;

    HitResult hit;
    hit.selfCollider = aBase;
    hit.otherCollider = bBase;
    hit.otherActor = bBase->GetOwner();
    hit.response = response;
    hit.isHit = true;
    hit.isBlocking = (response == CollisionResponse::BLOCK);
    hit.distance = sqrtf(distSq);

    hitResults_.push_back(hit);
}

// ---------------------------------------------------------------
// Capsule vs Capsule の判定
// ---------------------------------------------------------------
void CollisionManager::SolveCapsuleVsCapsule(
    ColliderBase* aBase, ColliderBase* bBase)
{
    auto* a = static_cast<ColliderCapsule*>(aBase);
    auto* b = static_cast<ColliderCapsule*>(bBase);

    CollisionResponse response = ResolveResponse(aBase, bBase);

    // DxLib のカプセル vs カプセル判定
    bool isHit = HitCheck_Capsule_Capsule(
        a->GetPosTop(), a->GetPosDown(), a->GetRadius(),
        b->GetPosTop(), b->GetPosDown(), b->GetRadius());

    if (!isHit) return;

    HitResult hit;
    hit.selfCollider = aBase;
    hit.otherCollider = bBase;
    hit.otherActor = bBase->GetOwner();
    hit.response = response;
    hit.isHit = true;
    hit.isBlocking = (response == CollisionResponse::BLOCK);

    hitResults_.push_back(hit);
}