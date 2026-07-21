#include "CapsuleComponent.h"

CapsuleComponent::CapsuleComponent(
    ActorBase& owner,
    float radius,
    float halfHeight)
    : PrimitiveComponent(owner)
    , radius_(radius)
    , halfHeight_(halfHeight)
{
}

CapsuleComponent::~CapsuleComponent(void)
{
}

// ---------------------------------------------------------------
// パラメータの取得・設定
// ---------------------------------------------------------------

float CapsuleComponent::GetRadius(void) const
{
    return radius_;
}

float CapsuleComponent::GetHalfHeight(void) const
{
    return halfHeight_;
}

void CapsuleComponent::SetRadius(float radius)
{
    radius_ = radius;
}

void CapsuleComponent::SetHalfHeight(float halfHeight)
{
    halfHeight_ = halfHeight;
}

// ---------------------------------------------------------------
// ワールド座標での形状取得
// ---------------------------------------------------------------

// カプセル上端のワールド座標
// ワールド座標 + 上方向 * halfHeight
Vector3 CapsuleComponent::GetTopPos(void) const
{
    return GetWorldPos() + GetUp() * halfHeight_;
}

// カプセル下端のワールド座標
// ワールド座標 - 上方向 * halfHeight
Vector3 CapsuleComponent::GetBottomPos(void) const
{
    return GetWorldPos() - GetUp() * halfHeight_;
}

// カプセル中心のワールド座標
Vector3 CapsuleComponent::GetCenter(void) const
{
    return GetWorldPos();
}

// ---------------------------------------------------------------
// デバッグ描画
// ---------------------------------------------------------------

void CapsuleComponent::Draw(void)
{
#ifdef _DEBUG
    Vector3 top = GetTopPos();
    Vector3 bottom = GetBottomPos();

    VECTOR dxTop = VGet(top.x, top.y, top.z);
    VECTOR dxBottom = VGet(bottom.x, bottom.y, bottom.z);

    DrawCapsule3D(dxTop, dxBottom, radius_, 8, 0xff0000, 0xff0000, false);
#endif
}