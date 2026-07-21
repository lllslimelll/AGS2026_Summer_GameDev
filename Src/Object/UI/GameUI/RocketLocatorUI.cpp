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

    // ===== XZ平面上の方位計算 =====
    const VECTOR playerPos = player_.GetTransform().pos;

    // プレイヤー正面を XZ 平面に射影
    VECTOR fwd = player_.GetTransform().GetForward();
    fwd.y = 0.0f;
    if (VSize(fwd) < 0.0001f) fwd = AsoUtility::DIR_F;
    fwd = VNorm(fwd);

    // right = cross(up=+Y, fwd)  (XZ 平面上で fwd の右側)
    const VECTOR right = VNorm(VCross(AsoUtility::AXIS_Y, fwd));

    // 世界の「北」は +Z 固定
    const VECTOR worldNorth = AsoUtility::DIR_F; // (0,0,1)

    // プレイヤー正面と北の相対角（度）
    float cosPN = VDot(fwd, worldNorth);
    float sinPN = VDot(right, worldNorth);
    float northRel = atan2f(sinPN, cosPN) * (180.0f / DX_PI_F);

    constexpr float VIEW_RANGE = 90.0f;
    constexpr float DEG_PER_PX = BAR_W / (VIEW_RANGE * 2.0f);

    int prevSize = GetFontSize();
    const char* CARDINAL[8] = { "N", "NE", "E", "SE", "S", "SW", "W", "NW" };

    // 目盛り＆ラベル描画（ロジックは元と同じ）
    for (int deg = -180; deg <= 180; deg += 5)
    {
        float rel = (float)deg - northRel;
        while (rel > 180.0f)  rel -= 360.0f;
        while (rel < -180.0f) rel += 360.0f;

        if (rel < -VIEW_RANGE || rel > VIEW_RANGE) continue;

        int x = cx + (int)(rel * DEG_PER_PX);
        if (x < barL || x > barR) continue;

        int normDeg = ((deg % 360) + 360) % 360;

        if (normDeg % 45 == 0)
        {
            DrawLine(x, barT, x, barB, 0xffffff);
            SetFontSize(LABEL_FONT);
            const char* label = CARDINAL[normDeg / 45];
            int lw = GetDrawStringWidth(label, (int)strlen(label));
            DrawString(x - lw / 2, barB + 4, label, 0xffffff);
        }
        else if (normDeg % 15 == 0)
        {
            DrawLine(x, barT, x, barT + BAR_H * 2 / 3, 0xaaaaaa);
            SetFontSize(18);
            char buf[8];
            sprintf_s(buf, "%d", normDeg);
            int tw = GetDrawStringWidth(buf, (int)strlen(buf));
            DrawString(x - tw / 2, barB + 4, buf, 0xaaaaaa);
        }
        else
        {
            DrawLine(x, barT, x, barT + BAR_H / 3, 0x888888);
        }
    }

    DrawTriangle(cx, barT - 2, cx - 7, barT - 14, cx + 7, barT - 14, 0xffffff, TRUE);

    // ===== ロケットマーカー =====
    VECTOR toRocket = VSub(rocket_.GetPos(), playerPos);
    toRocket.y = 0.0f;                              // XZ 平面へ射影
    if (VSize(toRocket) < 0.0001f) toRocket = fwd;  // 真上/直下対策
    toRocket = VNorm(toRocket);

    float cosA = VDot(fwd, toRocket);
    float sinA = VDot(right, toRocket);
    float relRocket = atan2f(sinA, cosA) * (180.0f / DX_PI_F);

    int markerX = cx + (int)(relRocket * DEG_PER_PX);
    markerX = max(barL + MARKER_R, min(barR - MARKER_R, markerX));

    DrawCircle(markerX, barT - MARKER_R - 2, MARKER_R, 0xff2020, TRUE);
    DrawCircle(markerX, barT - MARKER_R - 2, MARKER_R, 0xff6060, FALSE);

    SetFontSize(prevSize);
}