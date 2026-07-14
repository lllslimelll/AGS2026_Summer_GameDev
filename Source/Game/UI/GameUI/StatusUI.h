#pragma once
#include "GameUI.h"
class StatusUI : public GameUI
{
public:

	StatusUI(const Player& player);
	~StatusUI(void) override = default;

	void Draw(void) override;

private:

	const Player& player_;
};

