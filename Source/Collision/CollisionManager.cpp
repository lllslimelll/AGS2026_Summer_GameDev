#include <algorithm>
#include "../Component/PrimitiveComponent.h"
#include "../Component/CapsuleComponent.h"
#include "../Component/SphereComponent.h"
#include "../Component/StaticMeshComponent.h"
#include "../Game/Actor/ActorBase.h"
#include "../Utility/Math.h"
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
// 当たったポリゴンの法線方向へ少しずつ押し戻して
// 貫通が解消される座標（pushBackPos）を求める
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
    float   radius = capsule->GetRadius();

    VECTOR dxTop = top.ToVECTOR();
    VECTOR dxBottom = bottom.ToVECTOR();

    // 当たり判定対象フレームごとにヒットポリゴンを収集
    std::vector<MV1_COLL_RESULT_POLY> polys;
    for (int frame : mesh->GetCollisionFrames())
    {
        auto hits = MV1CollCheck_Capsule(
            mesh->GetModelId(), frame,
            dxTop, dxBottom, radius);

        for (int i = 0; i < hits.HitNum; i++)
        {
            polys.push_back(hits.Dim[i]);
        }
        MV1CollResultPolyDimTerminate(hits);
    }

    if (polys.empty()) return;

    // 代表法線（最も上向きのもの＝接地判定に使う）と衝突点
    VECTOR repNormal = polys[0].Normal;
    VECTOR repPoint = polys[0].Position[0];
    for (const auto& p : polys)
    {
        if (p.Normal.y > repNormal.y)
        {
            repNormal = p.Normal;
            repPoint = p.Position[0];
        }
    }

    HitResult hitA;
    hitA.selfCollider = capsuleComp;
    hitA.otherCollider = meshComp;
    hitA.otherActor = &meshComp->GetOwner();
    hitA.response = response;
    hitA.isHit = true;
    hitA.isBlocking = (response == CollisionResponse::BLOCK);
    hitA.normal = repNormal;
    hitA.point = repPoint;

    if (response == CollisionResponse::BLOCK)
    {
        // 床（上向きポリゴン）とそれ以外（壁・天井）に分類する
        constexpr float FLOOR_NORMAL_Y = 0.6f;

        std::vector<MV1_COLL_RESULT_POLY> floorPolys;
        std::vector<MV1_COLL_RESULT_POLY> wallPolys;
        for (const auto& p : polys)
        {
            if (p.Normal.y > FLOOR_NORMAL_Y) floorPolys.push_back(p);
            else                             wallPolys.push_back(p);
        }

        VECTOR delta = VGet(0.0f, 0.0f, 0.0f);

        // ---- 壁・天井：法線方向へ少しずつ押し出す ----
        // 押す方向は基本水平に投影し、高さ（床の解決）に干渉させない
        for (int tryCnt = 0; tryCnt < PUSH_TRY_COUNT; tryCnt++)
        {
            bool isHitAny = false;

            for (const auto& p : wallPolys)
            {
                bool isHit = HitCheck_Capsule_Triangle(
                    VAdd(dxTop, delta),
                    VAdd(dxBottom, delta),
                    radius,
                    p.Position[0],
                    p.Position[1],
                    p.Position[2]);

                if (!isHit) continue;

                VECTOR n = p.Normal;
                VECTOR flat = VGet(n.x, 0.0f, n.z);
                float  flatLen = VSize(flat);
                // ほぼ水平な法線なら水平投影、天井など縦向きなら法線のまま
                if (flatLen > 0.3f) n = VScale(flat, 1.0f / flatLen);

                delta = VAdd(delta, VScale(n, PUSH_BACK_DIS));
                isHitAny = true;
                break;
            }

            if (!isHitAny) break;
        }

        // ---- 床：垂直スナップで足元を床面に合わせる ----
        // 反復押し出しだと解決後の高さが毎フレーム微妙に変わり
        // ガクつくため、床は交点計算で一発でぴったり合わせる
        bool isSnapped = false;
        if (!floorPolys.empty())
        {
            VECTOR bottomSphere = VAdd(dxBottom, delta);
            float  feetY = bottomSphere.y - radius;

            // カプセル中心軸に沿った下向きの線分
            VECTOR lineStart = VAdd(dxTop, delta);
            VECTOR lineEnd = VGet(bottomSphere.x, feetY - 10.0f, bottomSphere.z);

            float bestY = -FLT_MAX;
            for (const auto& p : floorPolys)
            {
                HITRESULT_LINE line = HitCheck_Line_Triangle(
                    lineStart, lineEnd,
                    p.Position[0], p.Position[1], p.Position[2]);

                if (!line.HitFlag) continue;

                if (line.Position.y > bestY)
                {
                    bestY = line.Position.y;
                    repNormal = p.Normal;
                    repPoint = line.Position;
                    isSnapped = true;
                }
            }

            // 足が床より下（めり込み）なら、ちょうど接する高さまで持ち上げる
            if (isSnapped && bestY > feetY)
            {
                delta.y += bestY - feetY;
            }
        }

        // 足元の真下に床ポリゴンが無い（カプセルの縁だけが段差の角に
        // 触れている等）場合のみ、従来の反復押し出しで解消する
        if (!floorPolys.empty() && !isSnapped)
        {
            for (int tryCnt = 0; tryCnt < PUSH_TRY_COUNT; tryCnt++)
            {
                bool isHitAny = false;

                for (const auto& p : floorPolys)
                {
                    bool isHit = HitCheck_Capsule_Triangle(
                        VAdd(dxTop, delta),
                        VAdd(dxBottom, delta),
                        radius,
                        p.Position[0],
                        p.Position[1],
                        p.Position[2]);

                    if (!isHit) continue;

                    delta = VAdd(delta, VScale(p.Normal, PUSH_BACK_DIS));
                    isHitAny = true;
                    break;
                }

                if (!isHitAny) break;
            }
        }

        // hitA の法線・衝突点を更新（床スナップ時は床のもの）
        hitA.normal = repNormal;
        hitA.point = repPoint;

        // オーナー Actor をどこへ動かせば良いか（カプセルと同じ分だけ平行移動）
        Vector3 ownerPos = capsuleComp->GetOwner().GetPos();
        hitA.pushBackPos = VAdd(ownerPos.ToVECTOR(), delta);
        hitA.depth = VSize(delta);
    }

    // メッシュ側へ渡す HitResult
    HitResult hitB;
    hitB.selfCollider = meshComp;
    hitB.otherCollider = capsuleComp;
    hitB.otherActor = &capsuleComp->GetOwner();
    hitB.response = response;
    hitB.isHit = true;
    hitB.isBlocking = hitA.isBlocking;
    hitB.normal = VScale(repNormal, -1.0f);
    hitB.point = repPoint;
    hitB.pushBackPos = meshComp->GetOwner().GetPos().ToVECTOR();
    hitB.depth = 0.0f;

    NotifyHit(capsuleComp, meshComp, hitA, hitB);
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

    // B → A 方向の法線（A を押し出す方向）
    Vector3 normal = (dist > 0.0001f)
        ? (posA - posB) / dist
        : Vector3::UP;
    float depth = sum - dist;

    HitResult hitA;
    hitA.selfCollider = aComp;
    hitA.otherCollider = bComp;
    hitA.otherActor = &bComp->GetOwner();
    hitA.response = response;
    hitA.isHit = true;
    hitA.isBlocking = (response == CollisionResponse::BLOCK);
    hitA.distance = dist;
    hitA.depth = depth;
    hitA.normal = normal.ToVECTOR();
    hitA.point = (posB + normal * b->GetRadius()).ToVECTOR();
    hitA.pushBackPos = (aComp->GetOwner().GetPos() + normal * depth).ToVECTOR();

    HitResult hitB = hitA;
    hitB.selfCollider = bComp;
    hitB.otherCollider = aComp;
    hitB.otherActor = &aComp->GetOwner();
    hitB.normal = (-normal).ToVECTOR();
    hitB.pushBackPos = (bComp->GetOwner().GetPos() - normal * depth).ToVECTOR();

    NotifyHit(aComp, bComp, hitA, hitB);
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

    // 中心線分同士の最近接点を求める
    VECTOR cA, cB;
    ClosestPtSegmentSegment(
        aTop.ToVECTOR(), aBottom.ToVECTOR(),
        bTop.ToVECTOR(), bBottom.ToVECTOR(),
        cA, cB);

    float dist = VSize(VSub(cA, cB));
    float sum = a->GetRadius() + b->GetRadius();

    if (dist >= sum) return;

    // B → A 方向の法線（A を押し出す方向）
    Vector3 normal = (dist > 0.0001f)
        ? Vector3::FromVECTOR(VScale(VSub(cA, cB), 1.0f / dist))
        : Vector3::UP;

    // キャラ同士の押し合いは水平方向のみに投影する
    // （上下に押し出されて頭に乗ったり地面にめり込むのを防ぐ）
    Vector3 flatNormal = Vector3(normal.x, 0.0f, normal.z);
    if (!flatNormal.IsNearlyZero()) normal = flatNormal.Normalized();

    float depth = sum - dist;

    HitResult hitA;
    hitA.selfCollider = aComp;
    hitA.otherCollider = bComp;
    hitA.otherActor = &bComp->GetOwner();
    hitA.response = response;
    hitA.isHit = true;
    hitA.isBlocking = (response == CollisionResponse::BLOCK);
    hitA.distance = dist;
    hitA.depth = depth;
    hitA.normal = normal.ToVECTOR();
    hitA.point = VScale(VAdd(cA, cB), 0.5f);
    hitA.pushBackPos = (aComp->GetOwner().GetPos() + normal * depth).ToVECTOR();

    HitResult hitB = hitA;
    hitB.selfCollider = bComp;
    hitB.otherCollider = aComp;
    hitB.otherActor = &aComp->GetOwner();
    hitB.normal = (-normal).ToVECTOR();
    hitB.pushBackPos = (bComp->GetOwner().GetPos() - normal * depth).ToVECTOR();

    NotifyHit(aComp, bComp, hitA, hitB);
}

// ---------------------------------------------------------------
// 線分同士の最近接点を求める
// p1-q1 上の c1, p2-q2 上の c2 を返す
// ---------------------------------------------------------------
void CollisionManager::ClosestPtSegmentSegment(
    const VECTOR& p1, const VECTOR& q1,
    const VECTOR& p2, const VECTOR& q2,
    VECTOR& c1, VECTOR& c2)
{
    const float EPS = 1e-6f;

    VECTOR d1 = VSub(q1, p1); // 線分1の方向
    VECTOR d2 = VSub(q2, p2); // 線分2の方向
    VECTOR r = VSub(p1, p2);

    float a = VDot(d1, d1);
    float e = VDot(d2, d2);
    float f = VDot(d2, r);

    float s = 0.0f;
    float t = 0.0f;

    if (a <= EPS && e <= EPS)
    {
        // 両方とも点
    }
    else if (a <= EPS)
    {
        // 線分1が点
        t = f / e;
        t = max(0.0f, min(1.0f, t));
    }
    else
    {
        float c = VDot(d1, r);
        if (e <= EPS)
        {
            // 線分2が点
            s = max(0.0f, min(1.0f, -c / a));
        }
        else
        {
            float b = VDot(d1, d2);
            float denom = a * e - b * b;

            if (denom > EPS)
            {
                s = max(0.0f, min(1.0f, (b * f - c * e) / denom));
            }

            t = (b * s + f) / e;

            if (t < 0.0f)
            {
                t = 0.0f;
                s = max(0.0f, min(1.0f, -c / a));
            }
            else if (t > 1.0f)
            {
                t = 1.0f;
                s = max(0.0f, min(1.0f, (b - c) / a));
            }
        }
    }

    c1 = VAdd(p1, VScale(d1, s));
    c2 = VAdd(p2, VScale(d2, t));
}

// ---------------------------------------------------------------
// HitResult を OnHit / OnOverlap で両者に通知する
// ---------------------------------------------------------------
void CollisionManager::NotifyHit(
    PrimitiveComponent* a,
    PrimitiveComponent* b,
    const HitResult& hitA,
    const HitResult& hitB)
{
    hitResults_.push_back(hitA);

    ActorBase& ownerA = a->GetOwner();
    ActorBase& ownerB = b->GetOwner();

    if (hitA.response == CollisionResponse::BLOCK)
    {
        ownerA.OnHit(hitA);
        ownerB.OnHit(hitB);
    }
    else if (hitA.response == CollisionResponse::OVERLAP)
    {
        ownerA.OnOverlap(hitA);
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

        for (int frame : mesh->GetCollisionFrames())
        {
            MV1_COLL_RESULT_POLY result = MV1CollCheck_Line(
                mesh->GetModelId(), frame, start, end);

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

            for (int frame : mesh->GetCollisionFrames())
            {
                auto hits = MV1CollCheck_Capsule(
                    mesh->GetModelId(), frame, start, end, radius);

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