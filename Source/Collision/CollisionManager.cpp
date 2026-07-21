#include <algorithm>
#include "../Component/PrimitiveComponent.h"
#include "../Component/CapsuleComponent.h"
#include "../Component/SphereComponent.h"
#include "../Component/StaticMeshComponent.h"
#include "../Game/Actor/ActorBase.h"
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
    if (instance_ == nullptr) CreateInstance();
    return *instance_;
}

void CollisionManager::Destroy(void)
{
    delete instance_;
    instance_ = nullptr;
}

void CollisionManager::Clear(void)
{
    components_.clear();
    hitResults_.clear();
}

void CollisionManager::Register(PrimitiveComponent* component)
{
    auto it = std::find(components_.begin(), components_.end(), component);
    if (it == components_.end())
    {
        components_.push_back(component);
    }
}

void CollisionManager::Unregister(PrimitiveComponent* component)
{
    auto it = std::find(components_.begin(), components_.end(), component);
    if (it != components_.end())
    {
        components_.erase(it);
    }
}

// ---------------------------------------------------------------
// 毎フレーム全ての当たり判定を実行する
// ---------------------------------------------------------------
void CollisionManager::Update(void)
{
    hitResults_.clear();

    for (int i = 0; i < (int)components_.size(); i++)
    {
        for (int j = i + 1; j < (int)components_.size(); j++)
        {
            PrimitiveComponent* a = components_[i];
            PrimitiveComponent* b = components_[j];

            // どちらかが無効なら判定しない
            if (!a->IsValid() || !b->IsValid()) continue;

            // 同じ Actor のコンポーネント同士は判定しない
            if (&a->GetOwner() == &b->GetOwner()) continue;

            // 優先度方式でレスポンスを決定
            CollisionResponse response = ResolveResponse(a, b);

            // NONE なら判定しない
            if (response == CollisionResponse::NONE) continue;

            Solve(a, b);
        }
    }
}

// ---------------------------------------------------------------
// 優先度方式でレスポンスを決定する
// ---------------------------------------------------------------
CollisionResponse CollisionManager::ResolveResponse(
    const PrimitiveComponent* a,
    const PrimitiveComponent* b) const
{
    CollisionResponse responseA =
        a->GetProfile().GetResponse(b->GetChannel());
    CollisionResponse responseB =
        b->GetProfile().GetResponse(a->GetChannel());

    return static_cast<CollisionResponse>(
        max(static_cast<int>(responseA),
            static_cast<int>(responseB)));
}

// ---------------------------------------------------------------
// 形状の組み合わせで判定を分岐
// ---------------------------------------------------------------
void CollisionManager::Solve(
    PrimitiveComponent* a,
    PrimitiveComponent* b)
{
    using SHAPE = PrimitiveComponent::SHAPE;

    if (a->GetShape() == SHAPE::CAPSULE &&
        b->GetShape() == SHAPE::STATIC_MESH)
    {
        SolveCapsuleStaticMesh(a, b);
        return;
    }
    if (a->GetShape() == SHAPE::STATIC_MESH &&
        b->GetShape() == SHAPE::CAPSULE)
    {
        SolveCapsuleStaticMesh(b, a);
        return;
    }
    if (a->GetShape() == SHAPE::SPHERE &&
        b->GetShape() == SHAPE::SPHERE)
    {
        SolveSphereVsSphere(a, b);
        return;
    }
    if (a->GetShape() == SHAPE::CAPSULE &&
        b->GetShape() == SHAPE::CAPSULE)
    {
        SolveCapsuleVsCapsule(a, b);
        return;
    }
}

// ---------------------------------------------------------------
// Capsule vs StaticMesh の判定
// ---------------------------------------------------------------
void CollisionManager::SolveCapsuleStaticMesh(
    PrimitiveComponent* capsuleComp,
    PrimitiveComponent* meshComp)
{
    auto* capsule = static_cast<CapsuleComponent*>(capsuleComp);
    auto* mesh = static_cast<StaticMeshComponent*>(meshComp);

    CollisionResponse response = ResolveResponse(capsuleComp, meshComp);

    Vector3 top = capsule->GetTopPos();
    Vector3 bottom = capsule->GetBottomPos();

    auto hits = MV1CollCheck_Capsule(
        mesh->GetModelId(), -1,
        VGet(top.x, top.y, top.z),
        VGet(bottom.x, bottom.y, bottom.z),
        capsule->GetRadius());

    for (int i = 0; i < hits.HitNum; i++)
    {
        HitResult hit;
        hit.selfCollider = capsuleComp;
        hit.otherCollider = meshComp;
        hit.otherActor = &meshComp->GetOwner();
        hit.response = response;
        hit.isHit = true;
        hit.isBlocking = (response == CollisionResponse::BLOCK);
        hit.normal = hits.Dim[i].Normal;
        hit.point = hits.Dim[i].Position[0];

        if (response == CollisionResponse::BLOCK)
        {
            // 押し戻し座標を計算
            hit.pushBackPos = VGet(
                capsule->GetWorldPos().x,
                capsule->GetWorldPos().y,
                capsule->GetWorldPos().z);
        }

        NotifyHit(capsuleComp, meshComp, hit);
        break;
    }

    MV1CollResultPolyDimTerminate(hits);
}

// ---------------------------------------------------------------
// Sphere vs Sphere の判定
// ---------------------------------------------------------------
void CollisionManager::SolveSphereVsSphere(
    PrimitiveComponent* aComp,
    PrimitiveComponent* bComp)
{
    auto* a = static_cast<SphereComponent*>(aComp);
    auto* b = static_cast<SphereComponent*>(bComp);

    CollisionResponse response = ResolveResponse(aComp, bComp);

    Vector3 posA = a->GetCenter();
    Vector3 posB = b->GetCenter();
    float   dist = Vector3::Distance(posA, posB);
    float   sum = a->GetRadius() + b->GetRadius();

    if (dist >= sum) return;

    HitResult hit;
    hit.selfCollider = aComp;
    hit.otherCollider = bComp;
    hit.otherActor = &bComp->GetOwner();
    hit.response = response;
    hit.isHit = true;
    hit.isBlocking = (response == CollisionResponse::BLOCK);
    hit.distance = dist;

    NotifyHit(aComp, bComp, hit);
}

// ---------------------------------------------------------------
// Capsule vs Capsule の判定
// ---------------------------------------------------------------
void CollisionManager::SolveCapsuleVsCapsule(
    PrimitiveComponent* aComp,
    PrimitiveComponent* bComp)
{
    auto* a = static_cast<CapsuleComponent*>(aComp);
    auto* b = static_cast<CapsuleComponent*>(bComp);

    CollisionResponse response = ResolveResponse(aComp, bComp);

    Vector3 aTop = a->GetTopPos();
    Vector3 aBottom = a->GetBottomPos();
    Vector3 bTop = b->GetTopPos();
    Vector3 bBottom = b->GetBottomPos();

    bool isHit = HitCheck_Capsule_Capsule(
        VGet(aTop.x, aTop.y, aTop.z),
        VGet(aBottom.x, aBottom.y, aBottom.z),
        a->GetRadius(),
        VGet(bTop.x, bTop.y, bTop.z),
        VGet(bBottom.x, bBottom.y, bBottom.z),
        b->GetRadius());

    if (!isHit) return;

    HitResult hit;
    hit.selfCollider = aComp;
    hit.otherCollider = bComp;
    hit.otherActor = &bComp->GetOwner();
    hit.response = response;
    hit.isHit = true;
    hit.isBlocking = (response == CollisionResponse::BLOCK);

    NotifyHit(aComp, bComp, hit);
}

// ---------------------------------------------------------------
// HitResult を生成して OnHit / OnOverlap に通知する
// ---------------------------------------------------------------
void CollisionManager::NotifyHit(
    PrimitiveComponent* a,
    PrimitiveComponent* b,
    const HitResult& hit)
{
    hitResults_.push_back(hit);

    ActorBase& ownerA = a->GetOwner();
    ActorBase& ownerB = b->GetOwner();

    if (hit.response == CollisionResponse::BLOCK)
    {
        ownerA.OnHit(hit);

        // B 側の HitResult を作成
        HitResult hitB;
        hitB.selfCollider = b;
        hitB.otherCollider = a;
        hitB.otherActor = &ownerA;
        hitB.response = hit.response;
        hitB.isHit = true;
        hitB.isBlocking = true;
        ownerB.OnHit(hitB);
    }
    else if (hit.response == CollisionResponse::OVERLAP)
    {
        ownerA.OnOverlap(hit);

        HitResult hitB;
        hitB.selfCollider = b;
        hitB.otherCollider = a;
        hitB.otherActor = &ownerA;
        hitB.response = hit.response;
        hitB.isHit = true;
        hitB.isBlocking = false;
        ownerB.OnOverlap(hitB);
    }
}

// ---------------------------------------------------------------
// LineTrace
// ---------------------------------------------------------------
HitResult CollisionManager::LineTrace(
    const VECTOR& start,
    const VECTOR& end,
    CollisionChannel channel,
    const CollisionQueryParams& params) const
{
    HitResult nearest;
    nearest.distance = FLT_MAX;

    for (auto* comp : components_)
    {
        if (!comp->IsValid()) continue;
        if (!IsTargetChannel(comp, channel, params)) continue;
        if (comp->GetShape() != PrimitiveComponent::SHAPE::STATIC_MESH) continue;

        auto* mesh = static_cast<StaticMeshComponent*>(comp);

        MV1_COLL_RESULT_POLY result = MV1CollCheck_Line(
            mesh->GetModelId(), -1, start, end);

        if (!result.HitFlag) continue;

        float dist = VSize(VSub(result.HitPosition, start));
        if (dist < nearest.distance)
        {
            nearest.otherCollider = comp;
            nearest.otherActor = &comp->GetOwner();
            nearest.point = result.HitPosition;
            nearest.normal = result.Normal;
            nearest.distance = dist;
            nearest.isHit = true;
            nearest.isBlocking = true;
            nearest.response = CollisionResponse::BLOCK;
        }
    }

    return nearest;
}

// ---------------------------------------------------------------
// SweepSphere
// ---------------------------------------------------------------
HitResult CollisionManager::SweepSphere(
    const VECTOR& start,
    const VECTOR& end,
    float radius,
    CollisionChannel channel,
    const CollisionQueryParams& params) const
{
    HitResult nearest;
    nearest.distance = FLT_MAX;

    for (auto* comp : components_)
    {
        if (!comp->IsValid()) continue;
        if (!IsTargetChannel(comp, channel, params)) continue;

        if (comp->GetShape() == PrimitiveComponent::SHAPE::CAPSULE)
        {
            auto* capsule = static_cast<CapsuleComponent*>(comp);

            Vector3 top = capsule->GetTopPos();
            Vector3 bottom = capsule->GetBottomPos();

            bool isHit = HitCheck_Capsule_Capsule(
                start, end, radius,
                VGet(top.x, top.y, top.z),
                VGet(bottom.x, bottom.y, bottom.z),
                capsule->GetRadius());

            if (!isHit) continue;

            float dist = VSize(VSub(
                VGet(capsule->GetCenter().x,
                    capsule->GetCenter().y,
                    capsule->GetCenter().z),
                start));

            if (dist < nearest.distance)
            {
                nearest.otherCollider = comp;
                nearest.otherActor = &comp->GetOwner();
                nearest.distance = dist;
                nearest.isHit = true;
                nearest.isBlocking = true;
                nearest.response = CollisionResponse::BLOCK;
            }
        }
        else if (comp->GetShape() == PrimitiveComponent::SHAPE::STATIC_MESH)
        {
            auto* mesh = static_cast<StaticMeshComponent*>(comp);

            auto hits = MV1CollCheck_Capsule(
                mesh->GetModelId(), -1, start, end, radius);

            for (int i = 0; i < hits.HitNum; i++)
            {
                float dist = VSize(VSub(hits.Dim[i].HitPosition, start));
                if (dist < nearest.distance)
                {
                    nearest.otherCollider = comp;
                    nearest.otherActor = &comp->GetOwner();
                    nearest.point = hits.Dim[i].HitPosition;
                    nearest.normal = hits.Dim[i].Normal;
                    nearest.distance = dist;
                    nearest.isHit = true;
                    nearest.isBlocking = true;
                    nearest.response = CollisionResponse::BLOCK;
                }
            }

            MV1CollResultPolyDimTerminate(hits);
        }
    }

    return nearest;
}

// ---------------------------------------------------------------
// LineTrace / SweepSphere の対象か
// ---------------------------------------------------------------
bool CollisionManager::IsTargetChannel(
    const PrimitiveComponent* comp,
    CollisionChannel channel,
    const CollisionQueryParams& params) const
{
    // 無視リストに含まれていたらスキップ
    for (auto* ignored : params.ignoredActors)
    {
        if (ignored == &comp->GetOwner()) return false;
    }

    CollisionResponse response =
        comp->GetProfile().GetResponse(channel);

    return response != CollisionResponse::NONE;
}