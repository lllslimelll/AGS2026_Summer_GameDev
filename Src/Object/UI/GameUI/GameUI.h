#pragma once
#include <memory>
#include "../UI.h"

class GameUI : public UI
{
public:

    GameUI(void) = default;
    virtual ~GameUI(void) = default;

    virtual void Load(void) {};
    virtual void Update(void) {};
    virtual void Draw(void) override = 0;
};

