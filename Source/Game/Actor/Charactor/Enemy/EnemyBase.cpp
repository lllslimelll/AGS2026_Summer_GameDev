#include "../../../../Utility/AsoUtility.h"
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