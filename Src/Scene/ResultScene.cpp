#include "ResultScene.h"
#include "SceneManager.h"
#include "../Manager/InputManager.h"
#include <DxLib.h>
#include <cmath>

ResultScene::ResultScene()
    :
    SceneBase()
{
}

ResultScene::~ResultScene()
{
}

void ResultScene::Init(void)
{
    //totalScore_ = SceneManager::GetInstance().GetResultScore();
}

void ResultScene::Update(void)
{
    auto const& ins = InputManager::GetInstance();

    // シーン遷移
    // 決定
    bool decide = ins.IsTriggered(InputManager::InputCommand::UI_DECIDE);

    if (decide)
    {
        sceMng_.ChangeScene(SceneManager::SCENE_ID::TITLE);
    }
}

void ResultScene::Draw(void)
{
    constexpr int SCREEN_W = 1920;
    constexpr int SCREEN_H = 1080;

    int prevSize = GetFontSize();

    // ===== 左半分エリア =====
    int leftHalfX = 0;
    int leftHalfW = SCREEN_W / 2;

    // 上半分の中心ライン
    int upperHalfCenterY = SCREEN_H / 4;

    // ===== 「調査結果」見出し（上半分より上） =====
    constexpr int HEAD_FONT = 160;
    SetFontSize(HEAD_FONT);
    int headW = GetDrawStringWidth("調査結果", (int)strlen("調査結果"));
    int headX = leftHalfX + (leftHalfW - headW) / 2;
    int headY = upperHalfCenterY - HEAD_FONT;
    DrawString(headX, headY, "調査結果", 0xffff80);

    // ===== スコア値（見出しの下） =====
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
    withComma(totalScore_, scoreStr);
    char buf[48];
    sprintf_s(buf, "$%s", scoreStr);

    constexpr int SCORE_FONT = 180;
    SetFontSize(SCORE_FONT);
    int scoreW = GetDrawStringWidth(buf, (int)strlen(buf));
    int scoreX = leftHalfX + (leftHalfW - scoreW) / 2;
    int scoreY = headY + HEAD_FONT + 190;
    DrawString(scoreX, scoreY, buf, 0x00ff40);

    // ===== 成功 / 失敗スタンプ =====
    constexpr int CLEAR_THRESHOLD = 5000;
    const bool isSuccess = totalScore_ >= CLEAR_THRESHOLD;

    const char* stampText = isSuccess ? "成功" : "失敗";
    unsigned int stampColor = isSuccess ? 0xff2020 : 0x3060c0;
    constexpr int STAMP_FONT = 120;
    const double STAMP_ANGLE = -15.0 * DX_PI / 180.0;  // 反時計回り15度

    SetFontSize(STAMP_FONT);
    int stampW = GetDrawStringWidth(stampText, (int)strlen(stampText));

    // スタンプの中心位置（スコアの右下）
    int centerX = scoreX + scoreW + 40;
    int centerY = scoreY + SCORE_FONT - 20;

    // 文字を回転描画（中心基準）
    DrawRotaString(
        centerX, centerY,                // 描画基準位置
        1.0, 1.0,                        // 拡縮
        stampW / 2.0, STAMP_FONT / 2.0,  // 回転中心（文字の中心を指定）
        STAMP_ANGLE,                     // 回転角
        stampColor, 0, FALSE,
        stampText);

    // 枠を線分4本で描く（中心からの相対座標を回転）
    auto rotated = [&](double dx, double dy, int& x, int& y)
        {
            double c = cos(STAMP_ANGLE);
            double s = sin(STAMP_ANGLE);
            x = centerX + static_cast<int>(dx * c - dy * s);
            y = centerY + static_cast<int>(dx * s + dy * c);
        };

    auto drawRotatedBox = [&](double halfW, double halfH)
        {
            int x0, y0, x1, y1, x2, y2, x3, y3;
            rotated(-halfW, -halfH, x0, y0);
            rotated(halfW, -halfH, x1, y1);
            rotated(halfW, halfH, x2, y2);
            rotated(-halfW, halfH, x3, y3);
            DrawLineAA((float)x0, (float)y0, (float)x1, (float)y1, stampColor, 3.0f);
            DrawLineAA((float)x1, (float)y1, (float)x2, (float)y2, stampColor, 3.0f);
            DrawLineAA((float)x2, (float)y2, (float)x3, (float)y3, stampColor, 3.0f);
            DrawLineAA((float)x3, (float)y3, (float)x0, (float)y0, stampColor, 3.0f);
        };

    // 内枠と外枠
    double innerHalfW = stampW / 2.0 + 20;
    double innerHalfH = STAMP_FONT / 2.0 + 15;
    drawRotatedBox(innerHalfW, innerHalfH);
    drawRotatedBox(innerHalfW + 8, innerHalfH + 8);

    // ===== 右半分：ランキング =====
    int rightHalfX = SCREEN_W / 2;
    int rightHalfW = SCREEN_W / 2;

    // 「ランキング」見出し（左の「調査結果」と同じ高さ）
    SetFontSize(HEAD_FONT);
    int rankHeadW = GetDrawStringWidth("ランキング", (int)strlen("ランキング"));
    int rankHeadX = rightHalfX + (rightHalfW - rankHeadW) / 2;
    DrawString(rankHeadX, headY, "ランキング", 0xffff80);

    // ランキング表示用スペース（枠だけ）
    constexpr int RANK_MARGIN_SIDE = 80;
    constexpr int RANK_MARGIN_TOP = 40;
    constexpr int RANK_MARGIN_BOT = 80;

    int rankBoxL = rightHalfX + RANK_MARGIN_SIDE;
    int rankBoxR = rightHalfX + rightHalfW - RANK_MARGIN_SIDE;
    int rankBoxT = headY + HEAD_FONT + RANK_MARGIN_TOP;
    int rankBoxB = SCREEN_H - RANK_MARGIN_BOT;

    DrawBox(rankBoxL, rankBoxT, rankBoxR, rankBoxB, 0xffffff, FALSE);

    SetFontSize(prevSize);
}