#include <DxLib.h>
#include "ShotStraight.h"


ShotStraight::ShotStraight(TYPE type, int baseModelId)
	: ShotBase(type, baseModelId)
{
}

ShotStraight::~ShotStraight(void)
{
}

void ShotStraight::SetParam()
{
	OutputDebugStringA("SetParam called\n");
	scales_ = { 0.4f, 0.4f, 0.4f };

	speed_ = 8.0f;

	isAlive_ = true;

	cntAlive_ = 300;

	collisionRadius_ = 30.0f;
}
