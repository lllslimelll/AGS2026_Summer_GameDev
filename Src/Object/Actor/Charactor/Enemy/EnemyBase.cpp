#include "../../../../Utility/AsoUtility.h"
#include "../../../Collider/ColliderCapsule.h"
#include "EnemyBase.h"

EnemyBase::EnemyBase(const EnemyBase::EnemyData& data, Player& player)
	:
	CharactorBase(),
	player_(player),
	type_(data.type),
	hp_(data.hp),
	defaultPos_(data.defaultPos),
	movableRange_(data.movableRange)
{
	// 初期座標の設定
	transform_.pos = data.defaultPos;
}

EnemyBase::~EnemyBase(void)
{
}

void EnemyBase::Draw(void)
{
	ActorBase::Draw();

#ifdef _DEBUG

	// 移動可能範囲のデバッグ範囲
	//DrawSphere3D(defaultPos_, movableRange_, 16, 0x000099, 0x000099, false);

#endif // _DEBUG
}

bool EnemyBase::InMovableRange(void) const
{
	bool ret = false;

	// 初期位置からの距離
	float dis = static_cast<float>(
		AsoUtility::SqrMagnitude(defaultPos_, transform_.pos));
		
	// 指定距離判定
	if (dis < movableRange_ * movableRange_)
	{
		return true;
	}

	return ret;
}

void EnemyBase::ApplyPushBackXZ(const VECTOR& deltaXZ)
{
	transform_.pos.x += deltaXZ.x;
	transform_.pos.z += deltaXZ.z;
	// Y は触らない：落下・ジャンプに干渉させない
	transform_.Update();
}

const ColliderCapsule* EnemyBase::GetBodyCapsule(void) const
{
	int capsuleType = static_cast<int>(COLLIDER_TYPE::CAPSULE);
	if (ownColliders_.count(capsuleType) == 0) return nullptr;
	return dynamic_cast<const ColliderCapsule*>(ownColliders_.at(capsuleType));
}