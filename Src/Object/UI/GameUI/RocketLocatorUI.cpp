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
    const int barCY = barT + BAR_H / 2;

    // バー背景
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 140);
    DrawBox(barL, barT, barR, barB, 0x000000, TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    DrawBox(barL, barT, barR, barB, 0x555555, FALSE);

    int prevSize = GetFontSize();
    SetFontSize(LABEL_FONT);

    // ===== 月面接平面上での基底ベクトル計算 =====
    const VECTOR playerPos = player_.GetTransform().pos;
    const VECTOR moonCenter = { 0.0f, 0.0f, 0.0f };

    // 月の中心からプレイヤーへの上方向
    const VECTOR up = VNorm(VSub(playerPos, moonCenter));

    // プレイヤーの前方向を接平面に投影
    VECTOR fwd = player_.GetTransform().GetForward();
    float dotF = VDot(fwd, up);
    fwd = VNorm(VSub(fwd, VScale(up, dotF)));

    // 接平面上の右方向
    const VECTOR right = VNorm(VCross(up, fwd));

    // ===== ワールドの北方向（+Z）を接平面に投影 =====
    VECTOR worldNorth = { 0.0f, 0.0f, 1.0f };
    float dotN = VDot(worldNorth, up);
    worldNorth = VNorm(VSub(worldNorth, VScale(up, dotN)));

    // 北の右方向（北と上の外積）
    const VECTOR rightOfNorth = VNorm(VCross(up, worldNorth));

    // ===== 方角ラベル =====
    struct Compass { const char* label; float angle; };
    constexpr Compass DIRS[] = {
        { "N",    0.0f   },
        { "NE",   45.0f  },
        { "E",    90.0f  },
        { "SE",   135.0f },
        { "S",    180.0f },
        { "SW",   225.0f },
        { "W",    270.0f },
        { "NW",   315.0f },
    };

    for (const auto& d : DIRS)
    {
        // 北を基準に接平面上で回転した方向ベクトル
        float rad = d.angle * DX_PI_F / 180.0f;
        VECTOR dirVec = VAdd(VScale(worldNorth, cosf(rad)), VScale(rightOfNorth, sinf(rad)));

        // プレイヤーの前方向からの相対角度
        float cosA = VDot(fwd, dirVec);
        float sinA = VDot(right, dirVec);
        float rel = atan2f(sinA, cosA) * (180.0f / DX_PI_F);

        float t = rel / 180.0f;
        int x = barL + BAR_W / 2 + (int)(t * BAR_W / 2);

        if (x < barL || x > barR) continue;

        int labelW = GetDrawStringWidth(d.label, (int)strlen(d.label));
        DrawString(x - labelW / 2, barT + (BAR_H - LABEL_FONT) / 2, d.label, 0xcccccc);
    }

    // ===== ロケットマーカー（赤い丸）=====
    const VECTOR rocketPos = rocket_.GetPos();
    VECTOR toRocket = VNorm(VSub(rocketPos, playerPos));
    float dotR = VDot(toRocket, up);
    toRocket = VNorm(VSub(toRocket, VScale(up, dotR)));

    float cosA = VDot(fwd, toRocket);
    float sinA = VDot(right, toRocket);
    float relRocket = atan2f(sinA, cosA) * (180.0f / DX_PI_F);

    float t = relRocket / 180.0f;
    int markerX = barL + BAR_W / 2 + (int)(t * BAR_W / 2);
    markerX = max(barL + MARKER_R, min(barR - MARKER_R, markerX));

    DrawCircle(markerX, barCY, MARKER_R, 0xff2020, TRUE);
    DrawCircle(markerX, barCY, MARKER_R, 0xff6060, FALSE);

    // 中央の縦線（正面の目印）
    DrawLine(screenW / 2, barT, screenW / 2, barB, 0xffffff);

    SetFontSize(prevSize);
}