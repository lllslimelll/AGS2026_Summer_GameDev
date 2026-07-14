#pragma once
#include "GameUI.h"

class Player;
class Rocket;

class RocketLocatorUI : public GameUI
{
public:

    RocketLocatorUI(const Player& player, const Rocket& rocket);
    ~RocketLocatorUI(void) override = default;

    void Draw(void) override;

private:

    const Player& player_;
    const Rocket& rocket_;

    // バーのレイアウト定数
    static constexpr int   BAR_W = 800;
    static constexpr int   BAR_H = 30;
    static constexpr int   BAR_Y = 20;
    static constexpr int   MARKER_R = 12;
    static constexpr int   LABEL_FONT = 24;
};