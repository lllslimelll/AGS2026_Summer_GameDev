// GuideUI.cpp
#include <DxLib.h>
#include "../../Actor/Charactor/Player.h"
#include "GuideUI.h"

GuideUI::GuideUI(const Player& player)
    : GameUI(),
    player_(player)
{
}

void GuideUI::Draw(void)
{
    const auto info = player_.GetGuideInfo();

    int screenW, screenH;
    GetScreenState(&screenW, &screenH, nullptr);

    const char* labels[MAX_LABELS];
    int count = 0;

    // 拾う
    if (info.canPickUp)
    {
        labels[count++] = info.isPad ? "拾う : [X]" : "拾う : [F]";
    }

    // ロケット照準中
    if (info.isAimingRocket)
    {
        if (info.hasSelectedItem)
        {
            labels[count++] = info.isPad ? "納品 : [X]" : "納品 : [F]";
        }
        labels[count++] = info.isPad ? "帰還 : [B]" : "帰還 : [E]";
    }

    // 置く
    if (info.hasSelectedItem && info.isIdle)
    {
        labels[count++] = info.isPad ? "置く : [Y]" : "置く : [G]";
    }

    if (count == 0) return;

    int prevSize = GetFontSize();
    SetFontSize(FONT_SIZE);

    for (int i = 0; i < count; i++)
    {
        const char* buf = labels[i];
        int textW = GetDrawStringWidth(buf, (int)strlen(buf));
        int x = screenW - textW - MARGIN_RIGHT;
        int y = MARGIN_TOP + i * LINE_SPAN;
        DrawString(x, y, buf, GetColor(255, 255, 255));
    }

    SetFontSize(prevSize);
}