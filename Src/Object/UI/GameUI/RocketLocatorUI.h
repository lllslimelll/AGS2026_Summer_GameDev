#pragma once
#include "GameUI.h"

class Player;
class Rocket;

class RocketLocatorUI : public GameUI
{
public:

    RocketLocatorUI(const Player& player, const Rocket& rocket);
    ~RocketLocatorUI(void) override;

    void Draw(void) override;

private:

    const Player& player_;
    const Rocket& rocket_;

    // バーのレイアウト定数
    static constexpr int   BAR_W = 800;
    static constexpr int   BAR_H = 30;
    static constexpr int   BAR_Y = 20;
    static constexpr int   MARKER_R = 12;
    static constexpr int   LABEL_FONT = 24;   // N/E/S/W ラベル
    static constexpr int   TICK_FONT = 18;   // 15° ごとの度数表示

    // 目盛間隔（度）
    static constexpr int   TICK_STEP = 5;
    // 表示範囲（プレイヤー前方 ±VIEW_RANGE 度）
    static constexpr float VIEW_RANGE = 90.0f;

    // フォントハンドル（ループ内で SetFontSize を呼ばないよう事前生成）
    int fontLabel_ = -1;
    int fontTick_ = -1;
};