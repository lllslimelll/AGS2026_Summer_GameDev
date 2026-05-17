#include <DxLib.h>
#include "EnemyDemon.h"

EnemyDemon::EnemyDemon(void)
{
}
EnemyDemon::~EnemyDemon(void)
{
}

void EnemyDemon::SetParam()
{
	cntAttack_ = 0;

	scales_ = { 0.7f, 0.7f, 0.7f };
	speed_ = 5.0f;

	hp_ = 2;

	// 衝突判定用半径
	collisionRadius_ = 50.0f;

	// ステージの半径
	radius_ = 980.0f;
}
