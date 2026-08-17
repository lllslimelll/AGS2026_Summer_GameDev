#include <DxLib.h>
#include "../../../Object/Actor/Charactor/Player.h"
#include "StatusUI.h"

StatusUI::StatusUI(const Player& player)
    :
    player_(player)
{
    // フォントハンドル生成（デフォルトフォント、アンチエイリアス指定なし）
    fontLabel_ = CreateFontToHandle(nullptr, FONT_LABEL_SIZE, -1);
    fontSub_ = CreateFontToHandle(nullptr, FONT_SUB_SIZE, -1);
}

StatusUI::~StatusUI(void)
{
    if (fontLabel_ != -1) DeleteFontToHandle(fontLabel_);
    if (fontSub_ != -1) DeleteFontToHandle(fontSub_);
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
    const int labelY = barY - FONT_LABEL_SIZE - 2;

    // HPラベル
    DrawFormatStringToHandle(hpL, labelY, 0xffffff, fontLabel_, "HP  %d%%",
        static_cast<int>(static_cast<float>(hp) / Player::MAX_HP * 100));

    // O2ラベル："O" (大) + "2" (小) + " %%" (大)
    DrawStringToHandle(oxL, labelY, "O", 0xffffff, fontLabel_);
    const int oW = GetDrawStringWidthToHandle("O", 1, fontLabel_);

    // "2" は小フォントでベースラインを下げて描画
    DrawStringToHandle(oxL + oW, labelY + (FONT_LABEL_SIZE - FONT_SUB_SIZE) + 4,
        "2", 0xffffff, fontSub_);
    const int twoW = GetDrawStringWidthToHandle("2", 1, fontSub_);

    DrawFormatStringToHandle(oxL + oW + twoW + 4, labelY, 0xffffff, fontLabel_,
        " %d%%", static_cast<int>(oxygen / Player::MAX_OXYGEN * 100));
}