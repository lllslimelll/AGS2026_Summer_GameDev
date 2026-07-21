#include "../../../../Utility/AsoUtility.h"
#include "../../../../Collision/HitResult.h"
#include "../../../../Component/PrimitiveComponent.h"
#include "../Player.h"
#include "EnemyBase.h"

EnemyBase::EnemyBase(const EnemyBase::EnemyData& data, Player& player)
	: CharactorBase()
	, player_(player)
	, type_(data.type)
	, hp_(data.hp)
	, defaultPos_(data.defaultPos)
	, movableRange_(data.movableRange)
{
	SetPos(data.defaultPos);
}

EnemyBase::~EnemyBase(void)
{
}

void EnemyBase::Draw(void)
{
	CharactorBase::Draw();
}

void EnemyBase::OnHit(const HitResult& hit)
{
	// ワールド（ステージ等）からの押し戻しは基底に任せる
	CharactorBase::OnHit(hit);

	if (!hit.isHit || !hit.isBlocking) return;
	if (hit.otherCollider == nullptr) return;

	// キャラ同士（PAWN チャンネル）の押し出し
	if (hit.otherCollider->GetChannel() != CollisionChannel::PAWN) return;

	Vector3 normal = Vector3::FromVECTOR(hit.normal);

	if (dynamic_cast<Player*>(hit.otherActor) != nullptr)
	{
		// プレイヤーと当たったら敵側が全量押し出される
		SetPos(GetPos() + normal * hit.depth);
	}
	else
	{
		// 敵同士なら半分ずつ押し合う（両者の OnHit で計 1 回分離）
		SetPos(GetPos() + normal * (hit.depth * 0.5f));
	}
}

bool EnemyBase::InSearchCone(float dist, float halfFovRad) const
{
	Vector3 playerPos = player_.GetPos();
	Vector3 myPos = GetPos();
	VECTOR toPlayer = VSub(playerPos.ToVECTOR(), myPos.ToVECTOR());

	float distToPlayer = VSize(toPlayer);
	if (distToPlayer > dist) return false;

	VECTOR forward = GetForward().ToVECTOR();
	VECTOR dirToPlayer = VNorm(toPlayer);
	float dot = VDot(forward, dirToPlayer);
	if (dot < cosf(halfFovRad)) return false;

	// TODO: LineTrace 遮蔽チェック
	return true;
}

bool EnemyBase::InMovableRange(void) const
{
	float dis = static_cast<float>(
		AsoUtility::SqrMagnitude(defaultPos_.ToVECTOR(), GetPos().ToVECTOR()));
	return dis < movableRange_ * movableRange_;
}