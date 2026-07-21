// Component/SphereComponent.cpp
#include "SphereComponent.h"

SphereComponent::SphereComponent(
    ActorBase& owner,
    float radius)
    : PrimitiveComponent(owner)
    , radius_(radius)
{
}

SphereComponent::~SphereComponent(void)
{
}

// ---------------------------------------------------------------
// パラメータの取得・設定
// ---------------------------------------------------------------

float SphereComponent::GetRadius(void) const
{
    return radius_;
}

void SphereComponent::SetRadius(float radius)
{
    radius_ = radius;
}

// ---------------------------------------------------------------
// ワールド座標での形状取得
// ---------------------------------------------------------------

// 球の中心のワールド座標
Vector3 SphereComponent::GetCenter(void) const
{
    return GetWorldPos();
}

// ---------------------------------------------------------------
// デバッグ描画
// ---------------------------------------------------------------

void SphereComponent::Draw(void)
{
#ifdef _DEBUG
    Vector3 center = GetCenter();
    DrawSphere3D(
        VGet(center.x, center.y, center.z),
        radius_, 16,
        0xff0000, 0xff0000,
        false);
#endif
}