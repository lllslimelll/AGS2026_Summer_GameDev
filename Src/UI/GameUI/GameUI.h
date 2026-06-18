#pragma once
#include <memory>
#include "../UI.h"

class Player;

class GameUI : public UI
{
public:

    GameUI(std::unique_ptr<Player> player);
    virtual ~GameUI(void) = default;

    virtual void Init(void) override;
    virtual void Draw(void) override;
    virtual void Release(void) override;

public:

    std::unique_ptr<Player> player_;
};

