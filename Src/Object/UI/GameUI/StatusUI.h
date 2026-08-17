#pragma once
#include "GameUI.h"

class Player;

class StatusUI : public GameUI
{
public:

    StatusUI(const Player& player);
    ~StatusUI(void) override;

    void Draw(void) override;

private:

    const Player& player_;

    // フォントハンドル（サイズ別に事前生成）
    // 毎フレーム SetFontSize を呼ぶとフォント切り替えコストが発生するため、
    // サイズごとにハンドルを持ち、DrawStringToHandle 系で描画する
    int fontLabel_ = -1;   // 24px
    int fontSub_ = -1;   // 16px（"O2" の "2" 部分など）

    static constexpr int FONT_LABEL_SIZE = 24;
    static constexpr int FONT_SUB_SIZE = 16;
};