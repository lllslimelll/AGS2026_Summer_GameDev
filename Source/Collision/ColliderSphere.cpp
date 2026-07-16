#include "../Common/Transform.h"
#include "../Game/Actor/ActorBase.h"
#include "ColliderSphere.h"

ColliderSphere::ColliderSphere(
	CollisionProfileType type,
	ActorBase* owner,
	const VECTOR& localPos,
	float radius)
	:
	ColliderBase(SHAPE::SPHERE, type, owner),
	localPos_(localPos),
	radius_(radius)
{
}

ColliderSphere::~ColliderSphere(void)
{
}

const VECTOR& ColliderSphere::GetLocalPos(void) const
{
	return localPos_;
}

void ColliderSphere::SetLocalPos(const VECTOR& localPos)
{
	localPos_ = localPos;
}

VECTOR ColliderSphere::GetPos(void) const
{
	return GetRotPos(localPos_);
}

float ColliderSphere::GetRadius(void) const
{
	return radius_;
}

void ColliderSphere::SetRadius(float radius)
{
	radius_ = radius;
}

bool ColliderSphere::IsHitRay(const VECTOR& rayStart, const VECTOR& rayEnd) const
{
	const VECTOR center = GetPos();
	const VECTOR dir = VSub(rayEnd, rayStart);
	const VECTOR toCenter = VSub(rayStart, center);

	const float a = VDot(dir, dir);
	const float b = 2.0f * VDot(toCenter, dir);
	const float c = VDot(toCenter, toCenter) - radius_ * radius_;

	// 判別式
	const float discriminant = b * b - 4.0f * a * c;
	if (discriminant < 0.0f) return false;

	// ヒットパラメータ
	const float sqrtD = sqrtf(discriminant);
	const float len = sqrtf(a);
	const float t0 = (-b - sqrtD) / (2.0f * a);
	const float t1 = (-b + sqrtD) / (2.0f * a);

	return (t0 >= 0.0f && t0 <= len) || (t1 >= 0.0f && t1 <= len);
}

VECTOR ColliderSphere::GetPosPushBackAlongNormal(const MV1_COLL_RESULT_POLY& hitPoly, int maxTryCnt, float pushDistance) const
{
	// 座標
	VECTOR pos = owner_->GetTransform().pos;

	// 衝突補正処理
	int tryCnt = 0;
	while (tryCnt < maxTryCnt)
	{
		VECTOR spherePos = VAdd(pos, localPos_);

		// 球体と三角形の当たり判定
		if (!HitCheck_Sphere_Triangle(
			spherePos, radius_,
			hitPoly.Position[0], hitPoly.Position[1], hitPoly.Position[2]))
		{
			break;
		}

		// 衝突していたら法線方向に押し戻し
		pos = VAdd(pos, VScale(hitPoly.Normal, pushDistance));

		tryCnt++;
	}

	// 押し戻し座標を返す
	return pos;
}

void ColliderSphere::DrawDebug(int color)
{
	DrawSphere3D(GetPos(), radius_, 16, color, color, false);
}
