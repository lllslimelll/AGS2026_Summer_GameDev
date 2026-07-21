// Component/PrimitiveComponent.cpp
#include "PrimitiveComponent.h"
#include "../Collision/CollisionManager.h"
#include "../Collision/CollisionProfileManager.h"

PrimitiveComponent::PrimitiveComponent(ActorBase& owner)
    : SceneComponent(owner)
    , isValid_(true)
{
}

PrimitiveComponent::~PrimitiveComponent(void)
{
}

// ---------------------------------------------------------------
// ライフサイクル
// ---------------------------------------------------------------

// 初期化時に CollisionManager に自動登録
void PrimitiveComponent::Init(void)
{
    CollisionManager::GetInstance().Register(this);
}

// 解放時に CollisionManager から自動解除
void PrimitiveComponent::Release(void)
{
    CollisionManager::GetInstance().Unregister(this);
}

// ---------------------------------------------------------------
// CollisionProfile
// ---------------------------------------------------------------

void PrimitiveComponent::SetProfile(CollisionProfileType type)
{
    profile_ = CollisionProfileManager::GetInstance().GetProfile(type);
}

const CollisionProfile& PrimitiveComponent::GetProfile(void) const
{
    return profile_;
}

CollisionChannel PrimitiveComponent::GetChannel(void) const
{
    return profile_.channel_;
}

// ---------------------------------------------------------------
// 有効フラグ
// ---------------------------------------------------------------

bool PrimitiveComponent::IsValid(void) const
{
    return isValid_;
}

void PrimitiveComponent::SetValid(bool valid)
{
    isValid_ = valid;
}