#include "../Common/Transform.h"
#include "../Game/Actor/ActorBase.h"
#include "CollisionProfileManager.h"
#include "ColliderBase.h"

ColliderBase::ColliderBase(
	SHAPE shape,
	CollisionProfileType type,
	ActorBase* owner)
	:
	shape_(shape),
	profile_(CollisionProfileManager::GetInstance().GetProfile(type)),
	owner_(owner),
	isValid_(true)
{
}

ColliderBase::~ColliderBase(void)
{
}

void ColliderBase::Draw(void)
{
	int color = isValid_ ? COLOR_VALID : COLOR_INVALID;

	DrawDebug(color);
}

VECTOR ColliderBase::GetRotPos(const VECTOR& localPos) const
{
	const Transform& t = owner_->GetTransform();

	VECTOR localRotPos = t.quaRot.PosAxis(localPos);

	return VAdd(t.pos, localRotPos);
}