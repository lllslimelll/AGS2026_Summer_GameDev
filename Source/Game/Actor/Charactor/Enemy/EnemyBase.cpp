#include "../../../../Utility/AsoUtility.h"
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
	CharactorBase::Draw();

#ifdef _DEBUG

	// 移動可能範囲のデバッグ範囲
	//DrawSphere3D(defaultPos_, movableRange_, 16, 0x000099, 0x000099, false);

#endif // _DEBUG
}

bool EnemyBase::InSearchCone(float dist, float halfFovRad) const
{
	// Step1: 距離チェック
	VECTOR toPlayer = VSub(player_.GetTransform().pos, transform_.pos);
	float distToPlayer = VSize(toPlayer);
	if (distToPlayer > dist) return false;

	// Step2: 角度チェック（内積）
	VECTOR forward = transform_.GetForward();
	VECTOR dirToPlayer = VNorm(toPlayer);
	float dot = VDot(forward, dirToPlayer);

	// cos(halfFov) より小さければ視野外
	if (dot < cosf(halfFovRad)) return false;

	// Step3: 遮蔽チェック（TODO: LineTrace 実装後に追加）

	return true;
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
