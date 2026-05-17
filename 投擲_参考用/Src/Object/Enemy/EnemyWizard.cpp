#include "EnemyWizard.h"

EnemyWizard::EnemyWizard(void)
{
}

EnemyWizard::~EnemyWizard(void)
{
}

void EnemyWizard::SetParam()
{
	// UŒ‚ŠÔŠu
	cntAttack_ = 0.0f;

	scales_ = { 0.5f, 0.5f, 0.5f };
	speed_ = 5.0f;

	hp_ = 1;

	// Õ“Ë”»’è—p”¼Œa
	collisionRadius_ = 50.0f;

	// ƒXƒe[ƒW‚Ì”¼Œa
	radius_ = 800.0f;
}

void EnemyWizard::UpdateStandby()
{
	cntAttack_++;

	LookPlayer();

	if (cntAttack_ >= TERM_ATTACK)
	{
		ChangeState(STATE::ATTACK);
	}

	
}
