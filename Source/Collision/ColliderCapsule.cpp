#include "../Common/Transform.h"
#include "../Game/Actor/ActorBase.h"
#include "ColliderModel.h"
#include "ColliderCapsule.h"


ColliderCapsule::ColliderCapsule(
    CollisionProfileType type,
    ActorBase* owner,
    const VECTOR& localPosTop,
    const VECTOR& localPosDown,
    float radius)
    :
    ColliderBase(SHAPE::CAPSULE, type, owner),
    localPosTop_(localPosTop),
    localPosDown_(localPosDown),
    radius_(radius)
{
}

ColliderCapsule::~ColliderCapsule(void)
{
}

const VECTOR& ColliderCapsule::GetLocalPosTop(void) const
{
    return localPosTop_;
}

const VECTOR& ColliderCapsule::GetLocalPosDown(void) const
{
    return localPosDown_;
}

void ColliderCapsule::SetLocalPosTop(const VECTOR& pos)
{
    localPosTop_ = pos;
}

void ColliderCapsule::SetLocalPosDown(const VECTOR& pos)
{
    localPosDown_ = pos;
}

VECTOR ColliderCapsule::GetPosTop(void) const
{
    return GetRotPos(localPosTop_);
}

VECTOR ColliderCapsule::GetPosDown(void) const
{
	return GetRotPos(localPosDown_);
}

float ColliderCapsule::GetRadius(void) const
{
    return radius_;
}

void ColliderCapsule::SetRadius(float radius)
{
    radius_ = radius;
}

float ColliderCapsule::GetHeight(void) const
{
    return localPosTop_.y;
}

VECTOR ColliderCapsule::GetCenter(void) const
{
    VECTOR top = GetPosTop();
    VECTOR down = GetPosDown();

    VECTOR diff = VSub(top, down);
    return VAdd(down, VScale(diff, 0.5f));
}

void ColliderCapsule::PushBackAlongNormal(
    const ColliderModel* colliderModel, Transform& transform,
    int maxTryCnt, float pushDistance, bool isExclude, bool isTarget) const
{
    // モデルとカプセルの衝突判定
    auto hits = MV1CollCheck_Capsule(
        colliderModel->GetOwner()->GetTransform().modelId, -1,
        GetPosTop(), GetPosDown(), GetRadius());

    // 衝突した複数のポリゴンと衝突回避するまで、位置を移動させる
    for (int i = 0; i < hits.HitNum; i++)
    {
        auto hitPoly = hits.Dim[i];

        // 除外フレームは無視する
        if (isExclude && colliderModel->IsExcludeFrame(hitPoly.FrameIndex)) continue;
        // 対象フレーム以外は無視する
        if (isTarget && colliderModel->IsTargetFrame(hitPoly.FrameIndex)) continue;

        // 指定された回数と距離で三角形の法線方向に押し戻す
        transform.pos =
            GetPosPushBackAlongNormal(hitPoly, maxTryCnt, pushDistance);
    }

    // 検出したポリゴン情報の後始末
    MV1CollResultPolyDimTerminate(hits);    
}

VECTOR ColliderCapsule::GetPosPushBackAlongNormal(
    const MV1_COLL_RESULT_POLY& hitCollPoly, int maxTryCnt, float pushDistance) const
{
    // コピー生成
    Transform tmpTransform = owner_->GetTransform();
    ColliderCapsule tmpCapsule = *this;

    // 衝突補正処理
    int tryCnt = 0;
    VECTOR pos = tmpTransform.pos;

    while (tryCnt < maxTryCnt)
    {
        VECTOR top = VAdd(pos, localPosTop_);
        VECTOR down = VAdd(pos, localPosDown_);

        // カプセルと三角形の当たり判定
        if (!HitCheck_Capsule_Triangle(
             top, down, radius_,
            hitCollPoly.Position[0], hitCollPoly.Position[1], hitCollPoly.Position[2]))
        {
            break;
        }

        // 衝突していたら法線方向に押し戻し
        pos = VAdd(pos, VScale(hitCollPoly.Normal, pushDistance));

        tryCnt++;
    }

    // 押し戻した座標を返す
    return pos;
}

bool ColliderCapsule::IsHit(const ColliderModel* colliderModel, bool isExclude, bool isTarget) const
{
    bool ret = false;

    // モデルとカプセルの衝突判定
    auto hits = MV1CollCheck_Capsule(
        colliderModel->GetOwner()->GetTransform().modelId, -1,
        GetPosTop(), GetPosDown(), GetRadius());

    // 衝突した複数のポリゴンと衝突回避するまで、位置を移動させる
    for (int i = 0; i < hits.HitNum; i++)
    {
        auto hitPoly = hits.Dim[i];

        // 除外フレームは無視する
        if (isExclude && colliderModel->IsExcludeFrame(hitPoly.FrameIndex)) continue;
        // 対象フレーム以外は無視する
        if (isTarget && colliderModel->IsTargetFrame(hitPoly.FrameIndex)) continue;

       // 衝突している
        ret = true;
        break;
    }

    // 検出したポリゴン情報の後始末
    MV1CollResultPolyDimTerminate(hits);

    return ret;
}

void ColliderCapsule::DrawDebug(int color)
{
    // 上の球体
    VECTOR pos1 = GetPosTop();
    DrawSphere3D(pos1, radius_, 5, color, color, false);

    // 下の球体
    VECTOR pos2 = GetPosDown();
    DrawSphere3D(pos2, radius_, 5, color, color, false);

    VECTOR dir;
    VECTOR s;
    VECTOR e;

    const Transform& trans = owner_->GetTransform();

    // 球体を繋ぐ線(X+)
    dir = trans.GetRight();
    s = VAdd(pos1, VScale(dir, radius_));
    e = VAdd(pos2, VScale(dir, radius_));
    DrawLine3D(s, e, color);

    // 球体を繋ぐ線(X-)
    dir = trans.GetLeft();
    s = VAdd(pos1, VScale(dir, radius_));
    e = VAdd(pos2, VScale(dir, radius_));
    DrawLine3D(s, e, color);

    // 球体を繋ぐ線(Z+)
    dir = trans.GetForward();
    s = VAdd(pos1, VScale(dir, radius_));
    e = VAdd(pos2, VScale(dir, radius_));
    DrawLine3D(s, e, color);

    // 球体を繋ぐ線(Z-)
    dir = trans.GetBack();
    s = VAdd(pos1, VScale(dir, radius_));
    e = VAdd(pos2, VScale(dir, radius_));
    DrawLine3D(s, e, color);

    // カプセルの中心
    DrawSphere3D(GetCenter(), 5.0f, 10, color, color, true);
}
