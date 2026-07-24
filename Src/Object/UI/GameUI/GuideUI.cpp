// GuideUI.cpp
#include <DxLib.h>
#include "../../Actor/Charactor/Player.h"
#include "../../../Manager/ScreenManager.h"
#include "../../../PostEffect/PostEffectGuideUI.h"
#include "GuideUI.h"

GuideUI::GuideUI(const Player& player)
    : 
    GameUI(),
    player_(player)
{
    // ガイドUI用ポストエフェクト
    int screen = ScreenManager::GetInstance().GetGuideUIScreen(); // メインスクリーン取得
    pEffectGuideUI_ = std::make_unique<PostEffectGuideUI>(); // 生成
    // 初期化と対象スクリーンの設定
    pEffectGuideUI_->Init(screen);
    pEffectGuideUI_->SetEnabled(true);
}

void GuideUI::Update(void)
{
    pEffectGuideUI_->Update();
}

void GuideUI::Draw(void)
{
    int guideScreen = ScreenManager::GetInstance().GetGuideUIScreen();
    int mainScreen = ScreenManager::GetInstance().GetMainScreen();

    // ① guideScreenにテキスト描画
    SetDrawScreen(guideScreen);
    ClearDrawScreen();

    const auto info = player_.GetGuideInfo();

    int screenW, screenH;
    GetScreenState(&screenW, &screenH, nullptr);

    int prevSize = GetFontSize();

    constexpr int MARGIN_RIGHT = 60;
    constexpr int MARGIN_TOP = 60;
    constexpr int GOAL_FONT = 42;

    constexpr int SCORE_FONT = 50;
    constexpr int NORMA_FONT = 32;
    constexpr int LINE_FONT = 36;
    constexpr int LINE_SPAN = 50;

    int y = MARGIN_TOP;

    // ===== 目的表示 =====
    SetFontSize(GOAL_FONT);

    const char* goal;
    if (info.totalDelivered >= info.quota)
    {
        goal = "ロケットに帰還する";
    }
    else if (info.hasAnyItem)
    {
        goal = "ロケットに納品する";
    }
    else
    {
        goal = "アイテムを収集する";
    }

    int goalW = GetDrawStringWidth(goal, (int)strlen(goal));
    DrawString(screenW - goalW - MARGIN_RIGHT, y, goal, 0x00ff80);
    y += GOAL_FONT + 10;

    // ===== 現在の納品額 =====
    SetFontSize(SCORE_FONT);

    auto withComma = [](int value, char* out)
        {
            char tmp[32];
            sprintf_s(tmp, 32, "%d", value);
            int len = (int)strlen(tmp);
            int o = 0;
            int firstLen = len % 3;
            if (firstLen == 0) firstLen = 3;
            for (int i = 0; i < firstLen; i++) out[o++] = tmp[i];
            for (int i = firstLen; i < len; i += 3)
            {
                out[o++] = ',';
                out[o++] = tmp[i];
                out[o++] = tmp[i + 1];
                out[o++] = tmp[i + 2];
            }
            out[o] = '\0';
        };

    char scoreStr[32];
    withComma(info.totalDelivered, scoreStr);
    char scoreBuf[48];
    sprintf_s(scoreBuf, "$%s", scoreStr);

    int scoreW = GetDrawStringWidth(scoreBuf, (int)strlen(scoreBuf));
    DrawString(screenW - scoreW - MARGIN_RIGHT, y, scoreBuf, 0xffffff);
    y += SCORE_FONT + 4;
    
    // ===== ノルマ金額 =====
    SetFontSize(NORMA_FONT);

    char quotaStr[32];
    withComma(info.quota, quotaStr);
    char quotaBuf[48];
    sprintf_s(quotaBuf, "/ $%s", quotaStr);

    int quotaW = GetDrawStringWidth(quotaBuf, (int)strlen(quotaBuf));
    DrawString(screenW - quotaW - MARGIN_RIGHT, y, quotaBuf, 0x888888);
    y += NORMA_FONT + 16;

    // ===== 横線 =====
    const int lineR = screenW - MARGIN_RIGHT;
    const int lineL = lineR - max(max(goalW, scoreW), quotaW) - 20;
    DrawLine(lineL, y, lineR, y, 0x888888);
    y += 20;

    // ===== 操作ガイド =====
    const char* labels[MAX_LABELS];
    int count = 0;

    if (info.canPickUp)
    {
        labels[count++] = info.isPad ? "拾う : [X]" : "拾う : [F]";
    }

    if (info.isAimingRocket)
    {
        if (info.hasSelectedItem)
        {
            labels[count++] = info.isPad ? "納品 : [X]" : "納品 : [F]";
        }
        labels[count++] = info.isPad ? "帰還 : [B]" : "帰還 : [E]";
    }

    if (info.hasSelectedItem && info.isIdle)
    {
        labels[count++] = info.isPad ? "置く : [Y]" : "置く : [G]";
    }

    SetFontSize(LINE_FONT);

    for (int i = 0; i < count; i++)
    {
        const char* buf = labels[i];
        int textW = GetDrawStringWidth(buf, (int)strlen(buf));
        DrawString(screenW - textW - MARGIN_RIGHT, y + i * LINE_SPAN, buf, 0xdddddd);
    }

    SetFontSize(prevSize);

    // ポストエフェクト
    pEffectGuideUI_->Draw();

    // ③ mainScreenに歪み済みを合成
    SetDrawScreen(mainScreen);
    DrawGraph(0, 0, guideScreen, true);
}