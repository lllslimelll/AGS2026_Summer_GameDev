#include <DxLib.h>
#include <cmath>
#include "../../Actor/Charactor/Player.h"
#include "../../Actor/Stage/Rocket.h"
#include "../../../Utility/AsoUtility.h"
#include "RocketLocatorUI.h"

RocketLocatorUI::RocketLocatorUI(const Player& player, const Rocket& rocket)
    : GameUI(),
    player_(player),
    rocket_(rocket)
{
}

void RocketLocatorUI::Draw(void)
{
    int screenW, screenH;
    GetScreenState(&screenW, &screenH, nullptr);

    const int barL = (screenW - BAR_W) / 2;
    const int barR = barL + BAR_W;
    const int barT = BAR_Y;
    const int barB = barT + BAR_H;
    const int cx = screenW / 2;

    // ===== 接平面上での基底ベクトル計算 =====
    const VECTOR playerPos = player_.GetTransform().pos;
    const VECTOR moonCenter = { 0.0f, 0.0f, 0.0f };
    const VECTOR up = VNorm(VSub(playerPos, moonCenter));

    VECTOR fwd = player_.GetTransform().GetForward();
    float dotF = VDot(fwd, up);
    fwd = VNorm(VSub(fwd, VScale(up, dotF)));
    const VECTOR right = VNorm(VCross(up, fwd));

    // ワールドの北（+Z）を接平面に投影
    VECTOR worldNorth = { 0.0f, 0.0f, 1.0f };
    float dotN = VDot(worldNorth, up);
    worldNorth = VNorm(VSub(worldNorth, VScale(up, dotN)));
    const VECTOR rightOfNorth = VNorm(VCross(up, worldNorth));

    // プレイヤー前方と北の相対角度（度）
    float cosPN = VDot(fwd, worldNorth);
    float sinPN = VDot(right, worldNorth);
    float northRel = atan2f(sinPN, cosPN) * (180.0f / DX_PI_F);

    constexpr float VIEW_RANGE = 90.0f;
    constexpr float DEG_PER_PX = BAR_W / (VIEW_RANGE * 2.0f);

    int prevSize = GetFontSize();

    // ===== 方角名テーブル（45度ごと）=====
    const char* CARDINAL[8] = { "N", "NE", "E", "SE", "S", "SW", "W", "NW" };

    // ===== 目盛り・ラベル描画 =====
    // プレイヤー前方を中心に±VIEW_RANGE度の範囲だけ描画
    // 5度刻みなので最大36本
    for (int deg = -180; deg <= 180; deg += 5)
    {
        // バー上のX座標
        float rel = (float)deg - northRel;
        while (rel > 180.0f) rel -= 360.0f;
        while (rel < -180.0f) rel += 360.0f;

        if (rel < -VIEW_RANGE || rel > VIEW_RANGE) continue;

        int x = cx + (int)(rel * DEG_PER_PX);
        if (x < barL || x > barR) continue;

        int normDeg = ((deg % 360) + 360) % 360;

        if (normDeg % 45 == 0)
        {
            // 方角（長い線＋ラベル）
            DrawLine(x, barT, x, barB, 0xffffff);
            SetFontSize(LABEL_FONT);
            const char* label = CARDINAL[normDeg / 45];
            int lw = GetDrawStringWidth(label, (int)strlen(label));
            DrawString(x - lw / 2, barB + 4, label, 0xffffff);
        }
        else if (normDeg % 15 == 0)
        {
            // 中程度（線＋数字）
            DrawLine(x, barT, x, barT + BAR_H * 2 / 3, 0xaaaaaa);
            SetFontSize(18);
            char buf[8];
            sprintf_s(buf, "%d", normDeg);
            int tw = GetDrawStringWidth(buf, (int)strlen(buf));
            DrawString(x - tw / 2, barB + 4, buf, 0xaaaaaa);
        }
        else
        {
            // 短い線のみ
            DrawLine(x, barT, x, barT + BAR_H / 3, 0x888888);
        }
    }

    // ===== 中央の▼ =====
    DrawTriangle(cx, barT - 2, cx - 7, barT - 14, cx + 7, barT - 14, 0xffffff, TRUE);

    // ===== ロケットマーカー（赤い丸）=====
    const VECTOR rocketPos = rocket_.GetPos();
    VECTOR toRocket = VNorm(VSub(rocketPos, playerPos));
    float dotR = VDot(toRocket, up);
    toRocket = VNorm(VSub(toRocket, VScale(up, dotR)));

    float cosA = VDot(fwd, toRocket);
    float sinA = VDot(right, toRocket);
    float relRocket = atan2f(sinA, cosA) * (180.0f / DX_PI_F);

    int markerX = cx + (int)(relRocket * DEG_PER_PX);
    markerX = max(barL + MARKER_R, min(barR - MARKER_R, markerX));

    DrawCircle(markerX, barT - MARKER_R - 2, MARKER_R, 0xff2020, TRUE);
    DrawCircle(markerX, barT - MARKER_R - 2, MARKER_R, 0xff6060, FALSE);
    
    SetFontSize(prevSize);
}