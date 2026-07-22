#include <DxLib.h>
#include "../../../Object/Actor/Charactor/Player.h"
#include "StatusUI.h"

StatusUI::StatusUI(const Player& player)
    :
    player_(player)
{
}

void StatusUI::Draw(void)
{
    int screenW, screenH;
    GetScreenState(&screenW, &screenH, nullptr);

    const int size = 130;
    const int padding = 35;
    const int slotSpan = size + padding;
    const int totalWidth = 5 * slotSpan - padding;
    const int startX = (screenW - totalWidth) / 2 + 10;
    const int marginBottom = 80;
    const int invTopY = screenH - size - marginBottom;

    constexpr int BAR_H = 28;
    constexpr int GAP = 16;
    constexpr int MID_GAP = 20;
    constexpr int FONT_LABEL = 24;
    constexpr int FONT_SUB = 16;

    const int barY = invTopY - GAP - BAR_H;
    const int halfW = (totalWidth - MID_GAP) / 2;

    // HPバー
    const int hp = player_.GetHp();
    const int hpL = startX;
    const int hpR = hpL + halfW;
    const int hpFill = static_cast<int>(halfW * (static_cast<float>(hp) / Player::MAX_HP));

    DrawBox(hpL, barY, hpL + hpFill, barY + BAR_H, 0xff3030, TRUE);
    DrawBox(hpL, barY, hpR, barY + BAR_H, 0xffffff, FALSE);

    // O2バー
    const float oxygen = player_.GetOxygen();
    const int   oxL = hpR + MID_GAP;
    const int   oxR = oxL + halfW;
    const int   oxFill = static_cast<int>(halfW * (oxygen / Player::MAX_OXYGEN));
    const unsigned int oxColor = (oxygen <= 0.0f) ? 0xff8800 : 0x30a0ff;

    DrawBox(oxL, barY, oxL + oxFill, barY + BAR_H, oxColor, TRUE);
    DrawBox(oxL, barY, oxR, barY + BAR_H, 0xffffff, FALSE);

    // ラベル
    int prevSize = GetFontSize();
    SetFontSize(FONT_LABEL);
    const int labelY = barY - FONT_LABEL - 2;

    DrawFormatString(hpL, labelY, 0xffffff, "HP  %d%%",
        static_cast<int>(static_cast<float>(hp) / Player::MAX_HP * 100));

    DrawString(oxL, labelY, "O", 0xffffff);
    const int oW = GetDrawStringWidth("O", 1);

    SetFontSize(FONT_SUB);
    DrawString(oxL + oW, labelY + (FONT_LABEL - FONT_SUB) + 4, "2", 0xffffff);
    const int twoW = GetDrawStringWidth("2", 1);

    SetFontSize(FONT_LABEL);
    DrawFormatString(oxL + oW + twoW + 4, labelY, 0xffffff, " %d%%",
        static_cast<int>(oxygen / Player::MAX_OXYGEN * 100));

    SetFontSize(prevSize);
}
