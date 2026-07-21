#include <algorithm>
#include "SceneComponent.h"

SceneComponent::SceneComponent(ActorBase& owner)
    : ActorComponent(owner)
    , localPos_(Vector3::ZERO)
    , localRot_(Quaternion::Identity())
    , localScl_(Vector3::ONE)
    , parent_(nullptr)
{
}

SceneComponent::~SceneComponent(void)
{
    Detach();
}

// ---------------------------------------------------------------
// 親子関係
// ---------------------------------------------------------------

void SceneComponent::AttachTo(SceneComponent* parent)
{
    if (parent_ == parent) return;

    Detach();

    parent_ = parent;
    if (parent_ != nullptr)
    {
        parent_->children_.push_back(this);
    }
}

void SceneComponent::Detach(void)
{
    if (parent_ == nullptr) return;

    auto& siblings = parent_->children_;
    siblings.erase(
        std::remove(siblings.begin(), siblings.end(), this),
        siblings.end());

    parent_ = nullptr;
}

SceneComponent* SceneComponent::GetParent(void) const
{
    return parent_;
}

const std::vector<SceneComponent*>& SceneComponent::GetChildren(void) const
{
    return children_;
}

// ---------------------------------------------------------------
// ローカル Transform の設定
// ---------------------------------------------------------------

void SceneComponent::SetLocalPos(const Vector3& pos)
{
    localPos_ = pos;
}

void SceneComponent::SetLocalRot(const Quaternion& rot)
{
    localRot_ = rot;
}

void SceneComponent::SetLocalScl(const Vector3& scl)
{
    localScl_ = scl;
}

// ---------------------------------------------------------------
// ローカル Transform の取得
// ---------------------------------------------------------------

const Vector3& SceneComponent::GetLocalPos(void) const
{
    return localPos_;
}

const Quaternion& SceneComponent::GetLocalRot(void) const
{
    return localRot_;
}

const Vector3& SceneComponent::GetLocalScl(void) const
{
    return localScl_;
}

// ---------------------------------------------------------------
// ワールド Transform の取得
// ---------------------------------------------------------------

Vector3 SceneComponent::GetWorldPos(void) const
{
    if (parent_ == nullptr)
    {
        return localPos_;
    }

    return parent_->GetWorldPos() +
        parent_->GetWorldRot().RotateVector(
            localPos_ * parent_->GetWorldScl());
}

Quaternion SceneComponent::GetWorldRot(void) const
{
    if (parent_ == nullptr)
    {
        return localRot_;
    }
    return parent_->GetWorldRot() * localRot_;
}

Vector3 SceneComponent::GetWorldScl(void) const
{
    if (parent_ == nullptr)
    {
        return localScl_;
    }
    Vector3 parentScl = parent_->GetWorldScl();
    return Vector3(
        parentScl.x * localScl_.x,
        parentScl.y * localScl_.y,
        parentScl.z * localScl_.z);
}

Matrix4x4 SceneComponent::GetWorldMat(void) const
{
    return Matrix4x4::Compose(
        GetWorldPos(),
        GetWorldRot(),
        GetWorldScl());
}

// ---------------------------------------------------------------
// ワールド Transform の設定
// ---------------------------------------------------------------

void SceneComponent::SetWorldPos(const Vector3& pos)
{
    if (parent_ == nullptr)
    {
        localPos_ = pos;
        return;
    }

    Vector3    parentPos = parent_->GetWorldPos();
    Quaternion parentRot = parent_->GetWorldRot();
    Vector3    parentScl = parent_->GetWorldScl();

    Vector3 diff = pos - parentPos;
    Vector3 localPos = parentRot.Inverse().RotateVector(diff);

    localPos_ = Vector3(
        localPos.x / parentScl.x,
        localPos.y / parentScl.y,
        localPos.z / parentScl.z);
}

void SceneComponent::SetWorldRot(const Quaternion& rot)
{
    if (parent_ == nullptr)
    {
        localRot_ = rot;
        return;
    }
    localRot_ = parent_->GetWorldRot().Inverse() * rot;
}

// ---------------------------------------------------------------
// 方向ベクトルの取得（ワールド空間）
// ---------------------------------------------------------------

Vector3 SceneComponent::GetForward(void) const
{
    return GetWorldRot().RotateVector(Vector3::FORWARD);
}

Vector3 SceneComponent::GetBack(void) const
{
    return GetWorldRot().RotateVector(Vector3::BACK);
}

Vector3 SceneComponent::GetUp(void) const
{
    return GetWorldRot().RotateVector(Vector3::UP);
}

Vector3 SceneComponent::GetDown(void) const
{
    return GetWorldRot().RotateVector(Vector3::DOWN);
}

Vector3 SceneComponent::GetLeft(void) const
{
    return GetWorldRot().RotateVector(Vector3::LEFT);
}

Vector3 SceneComponent::GetRight(void) const
{
    return GetWorldRot().RotateVector(Vector3::RIGHT);
}