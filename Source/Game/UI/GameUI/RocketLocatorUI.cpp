#include <DxLib.h>
#include <cmath>
#include "../../Actor/Charactor/Player.h"
#include "../../Actor/Stage/Rocket.h"
#include "../../../Utility/Math.h"
#include "RocketLocatorUI.h"

RocketLocatorUI::RocketLocatorUI(const Player& player, const Rocket& rocket)
    : GameUI()
    , player_(player)
    , rocket_(rocket)
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

    // ===== XY 平面用の基底ベクトル計算 =====
    // 上方向は Y 固定なので接平面投影が不要になる

    // プレイヤーの前方向（Y 成分を捨てて水平面上に投影）
    Vector3 fwd = Vector3(player_.GetForward().x, 0.0f, player_.GetForward().z).Normalized();
    // 右方向（UP × forward）
    Vector3 right = Vector3::Cross(Vector3::UP, fwd).Normalized();

    // ワールドの北（+Z）
    Vector3 worldNorth = Vector3::FORWARD;

    // プレイヤー前方と北の相対角度（度）
    float cosPN = Vector3::Dot(fwd, worldNorth);
    float sinPN = Vector3::Dot(right, worldNorth);
    float northRel = atan2f(sinPN, cosPN) * Math::ToDegree(1.0f);

    constexpr float VIEW_RANGE = 90.0f;
    constexpr float DEG_PER_PX = BAR_W / (VIEW_RANGE * 2.0f);

    int prevSize = GetFontSize();

    // ===== 方角名テーブル（45度ごと）=====
    const char* CARDINAL[8] = { "N", "NE", "E", "SE", "S", "SW", "W", "NW" };

    // ===== 目盛り・ラベル描画 =====
    // プレイヤー前方を中心に ±VIEW_RANGE 度の範囲を 5 度刻みで描画
    for (int deg = -180; deg <= 180; deg += 5)
    {
        float rel = static_cast<float>(deg) - northRel;
        while (rel > 180.0f) rel -= 360.0f;
        while (rel < -180.0f) rel += 360.0f;

        if (rel < -VIEW_RANGE || rel > VIEW_RANGE) continue;

        int x = cx + static_cast<int>(rel * DEG_PER_PX);
        if (x < barL || x > barR) continue;

        int normDeg = ((deg % 360) + 360) % 360;

        if (normDeg % 45 == 0)
        {
            // 方角：長い線 + ラベル
            DrawLine(x, barT, x, barB, 0xffffff);
            SetFontSize(LABEL_FONT);
            const char* label = CARDINAL[normDeg / 45];
            int lw = GetDrawStringWidth(label, static_cast<int>(strlen(label)));
            DrawString(x - lw / 2, barB + 4, label, 0xffffff);
        }
        else if (normDeg % 15 == 0)
        {
            // 中目盛り：線 + 角度数字
            DrawLine(x, barT, x, barT + BAR_H * 2 / 3, 0xaaaaaa);
            SetFontSize(18);
            char buf[8];
            sprintf_s(buf, "%d", normDeg);
            int tw = GetDrawStringWidth(buf, static_cast<int>(strlen(buf)));
            DrawString(x - tw / 2, barB + 4, buf, 0xaaaaaa);
        }
        else
        {
            // 短い目盛り線のみ
            DrawLine(x, barT, x, barT + BAR_H / 3, 0x888888);
        }
    }

    // ===== 中央の ▼（現在の向きを示すカーソル）=====
    DrawTriangle(cx, barT - 2, cx - 7, barT - 14, cx + 7, barT - 14, 0xffffff, TRUE);

    // ===== ロケットマーカー（赤い丸）=====
    Vector3 playerPos = player_.GetPos();
    Vector3 rocketPos = rocket_.GetPos();

    // ロケットへの水平方向ベクトル
    Vector3 toRocket = Vector3(rocketPos.x - playerPos.x, 0.0f, rocketPos.z - playerPos.z).Normalized();

    float cosA = Vector3::Dot(fwd, toRocket);
    float sinA = Vector3::Dot(right, toRocket);
    float relRocket = atan2f(sinA, cosA) * Math::ToDegree(1.0f);

    int markerX = cx + static_cast<int>(relRocket * DEG_PER_PX);
    markerX = Math::Clamp(markerX, barL + MARKER_R, barR - MARKER_R);

    DrawCircle(markerX, barT - MARKER_R - 2, MARKER_R, 0xff2020, TRUE);
    DrawCircle(markerX, barT - MARKER_R - 2, MARKER_R, 0xff6060, FALSE);

    SetFontSize(prevSize);
}