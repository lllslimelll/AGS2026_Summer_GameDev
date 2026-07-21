#include "ActorComponent.h"
#include "../Game/Actor/ActorBase.h"

ActorComponent::ActorComponent(ActorBase& owner)
    :
    owner_(owner),
    active_(true)
{
}

ActorComponent::~ActorComponent(void)
{
}

// ---------------------------------------------------------------
// 所有者
// ---------------------------------------------------------------

ActorBase& ActorComponent::GetOwner(void) const
{
    return owner_;
}

// ---------------------------------------------------------------
// 有効フラグ
// ---------------------------------------------------------------

bool ActorComponent::IsActive(void) const
{
    return active_;
}

void ActorComponent::SetActive(bool active)
{
    active_ = active;
}