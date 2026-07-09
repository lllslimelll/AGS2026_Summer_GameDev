#include "../Common//Transform.h"
#include "ColliderSphere.h"

ColliderSphere::ColliderSphere(
	TAG tag, const Transform* follow,
	const VECTOR& localPos, float radius)
	:
	ColliderBase(SHAPE::SPHERE, tag, follow),
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
	return MV1CollCheck_Line(
		rayStart, rayEnd, 0.0f,  // 半径0のカプセル = 線分
		GetPos(), radius_) == TRUE;
}

VECTOR ColliderSphere::GetPosPushBackAlongNormal(const MV1_COLL_RESULT_POLY& hitPoly, int maxTryCnt, float pushDistance) const
{
	// 追従先の自身のインスタンスをコピー生成
	Transform tmpTransform = *follow_;
	ColliderSphere tmpSphere = *this;
	tmpSphere.SetFollow(&tmpTransform);

	// 衝突補正処理
	int tryCnt = 0;
	while (tryCnt < maxTryCnt)
	{
		// 球体と三角形の当たり判定
		if (!HitCheck_Sphere_Triangle(
			tmpSphere.GetPos(), tmpSphere.GetRadius(),
			hitPoly.Position[0], hitPoly.Position[1], hitPoly.Position[2]))
		{
			break;
		}

		// 衝突していたら法線方向に押し戻し
		tmpTransform.pos =
			VAdd(tmpTransform.pos, VScale(hitPoly.Normal, pushDistance));

		tryCnt++;
	}

	// 押し戻し座標を返す
	return tmpTransform.pos;
}

void ColliderSphere::DrawDebug(int color)
{
	DrawSphere3D(GetPos(), radius_, 16, color, color, false);
}
