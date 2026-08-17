// GuideUI.cpp
#include <DxLib.h>
#include <cstring>
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
    int screen = ScreenManager::GetInstance().GetGuideUIScreen();
    pEffectGuideUI_ = std::make_unique<PostEffectGuideUI>();
    pEffectGuideUI_->Init(screen);
    pEffectGuideUI_->SetEnabled(true);

    // フォントハンドル生成
    fontGoal_ = CreateFontToHandle(nullptr, FONT_GOAL_SIZE, -1);
    fontScore_ = CreateFontToHandle(nullptr, FONT_SCORE_SIZE, -1);
    fontNorma_ = CreateFontToHandle(nullptr, FONT_NORMA_SIZE, -1);
    fontLine_ = CreateFontToHandle(nullptr, FONT_LINE_SIZE, -1);
}

GuideUI::~GuideUI(void)
{
    if (fontGoal_ != -1) DeleteFontToHandle(fontGoal_);
    if (fontScore_ != -1) DeleteFontToHandle(fontScore_);
    if (fontNorma_ != -1) DeleteFontToHandle(fontNorma_);
    if (fontLine_ != -1) DeleteFontToHandle(fontLine_);
}

void GuideUI::Update(void)
{
    pEffectGuideUI_->Update();
}

void GuideUI::Draw(void)
{
    int guideScreen = ScreenManager::GetInstance().GetGuideUIScreen();
    int mainScreen = ScreenManager::GetInstance().GetMainScreen();

    // ① guideScreen にテキスト描画
    SetDrawScreen(guideScreen);
    ClearDrawScreen();

    const auto info = player_.GetGuideInfo();

    int screenW, screenH;
    GetScreenState(&screenW, &screenH, nullptr);

    int y = MARGIN_TOP;

    // ===== 目的表示 =====
    const char* goal;
    if (info.totalDelivered >= info.quota)
    {
        goal = "ロケットに帰還せよ";
    }
    else if (info.hasAnyItem)
    {
        goal = "ロケットに納品せよ";
    }
    else
    {
        goal = "アイテムを収集せよ";
    }

    const int goalW = GetDrawStringWidthToHandle(
        goal, static_cast<int>(strlen(goal)), fontGoal_);
    DrawStringToHandle(screenW - goalW - MARGIN_RIGHT, y, goal, 0x00ff80, fontGoal_);
    y += FONT_GOAL_SIZE + 10;

    // ===== 現在の納品額 =====
    auto withComma = [](int value, char* out)
        {
            char tmp[32];
            sprintf_s(tmp, 32, "%d", value);
            int len = static_cast<int>(strlen(tmp));
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

    const int scoreW = GetDrawStringWidthToHandle(
        scoreBuf, static_cast<int>(strlen(scoreBuf)), fontScore_);
    DrawStringToHandle(screenW - scoreW - MARGIN_RIGHT, y, scoreBuf, 0xffffff, fontScore_);
    y += FONT_SCORE_SIZE + 4;

    // ===== ノルマ額 =====
    char quotaStr[32];
    withComma(info.quota, quotaStr);
    char quotaBuf[48];
    sprintf_s(quotaBuf, "/ $%s", quotaStr);

    const int quotaW = GetDrawStringWidthToHandle(
        quotaBuf, static_cast<int>(strlen(quotaBuf)), fontNorma_);
    DrawStringToHandle(screenW - quotaW - MARGIN_RIGHT, y, quotaBuf, 0x888888, fontNorma_);
    y += FONT_NORMA_SIZE + 16;

    // ===== 区切り線 =====
    const int lineR = screenW - MARGIN_RIGHT;
    int maxW = goalW;
    if (scoreW > maxW) maxW = scoreW;
    if (quotaW > maxW) maxW = quotaW;
    const int lineL = lineR - maxW - 20;
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

    for (int i = 0; i < count; i++)
    {
        const char* buf = labels[i];
        const int textW = GetDrawStringWidthToHandle(
            buf, static_cast<int>(strlen(buf)), fontLine_);
        DrawStringToHandle(screenW - textW - MARGIN_RIGHT,
            y + i * LINE_SPAN, buf, 0xdddddd, fontLine_);
    }

    // ポストエフェクト（Skew処理）
    pEffectGuideUI_->Draw();

    // ② mainScreen に合成
    SetDrawScreen(mainScreen);
    DrawGraph(0, 0, guideScreen, true);
}