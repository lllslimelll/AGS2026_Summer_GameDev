#pragma once
#include "ShotBase.h"

class ShotStraight : public ShotBase
{
public:
	ShotStraight(TYPE type, int baseModelId);
	~ShotStraight(void);


protected:

	void SetParam() override;
};

