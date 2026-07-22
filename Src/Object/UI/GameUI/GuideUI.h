// GuideUI.h
#pragma once
#include "GameUI.h"

class Player;
class PostEffectGuideUI;

class GuideUI : public GameUI
{
public:

    explicit GuideUI(const Player& player);
    ~GuideUI(void) override = default;

    void Update(void) override;
    void Draw(void) override;

private:
    const Player& player_;

    // ガイドUI用ポストエフェクト
    std::unique_ptr<PostEffectGuideUI> pEffectGuideUI_;

    static constexpr int FONT_SIZE = 50;
    static constexpr int LINE_SPAN = 60;
    static constexpr int MARGIN_TOP = 170;
    static constexpr int MARGIN_RIGHT = 60;
    static constexpr int MAX_LABELS = 4;
};