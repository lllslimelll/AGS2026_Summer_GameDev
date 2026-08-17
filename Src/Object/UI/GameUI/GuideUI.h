// GuideUI.h
#pragma once
#include "GameUI.h"

class Player;
class PostEffectGuideUI;

class GuideUI : public GameUI
{
public:

    explicit GuideUI(const Player& player);
    ~GuideUI(void) override;

    void Update(void) override;
    void Draw(void) override;

private:
    const Player& player_;

    // ガイドUI用ポストエフェクト
    std::unique_ptr<PostEffectGuideUI> pEffectGuideUI_;

    // ---- フォントハンドル ----
    // 従来は Draw 内で 4 種類の SetFontSize を毎フレーム呼び分けていた。
    // DxLib のフォント切替コストが大きいため、
    // サイズ別ハンドルを事前生成し DrawStringToHandle 系で描画する。
    int fontGoal_ = -1;   // 42px "目的" テキスト
    int fontScore_ = -1;   // 50px 現在納品額
    int fontNorma_ = -1;   // 32px ノルマ表示
    int fontLine_ = -1;   // 36px 操作ラベル

    static constexpr int FONT_GOAL_SIZE = 42;
    static constexpr int FONT_SCORE_SIZE = 50;
    static constexpr int FONT_NORMA_SIZE = 32;
    static constexpr int FONT_LINE_SIZE = 36;

    // レイアウト定数
    static constexpr int MARGIN_RIGHT = 60;
    static constexpr int MARGIN_TOP = 60;
    static constexpr int LINE_SPAN = 50;
    static constexpr int MAX_LABELS = 4;
};