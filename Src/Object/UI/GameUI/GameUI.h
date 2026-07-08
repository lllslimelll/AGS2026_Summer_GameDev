#pragma once
#include <memory>
#include "../UI.h"

class Player;

class GameUI : public UI
{
public:

    GameUI(const Player& player);
    virtual ~GameUI(void) = default;

    virtual void Draw(void) override = 0;

protected:

    const Player& player_;
};

