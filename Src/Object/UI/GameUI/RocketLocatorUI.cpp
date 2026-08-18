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
    // フォントハンドル生成
    // 従来は毎フレーム SetFontSize を目盛りループの中で複数回呼んでいたが、
    // これはフォント切替コストが大きく FPS 低下の主因になっていたため、
    // サイズ別にハンドルを事前生成し DrawStringToHandle で描く
    fontLabel_ = CreateFontToHandle(nullptr, LABEL_FONT, -1);
    fontTick_ = CreateFontToHandle(nullptr, TICK_FONT, -1);

	rocketIcon_ = LoadGraph("Data/Image/rocket.png");
}

RocketLocatorUI::~RocketLocatorUI(void)
{
    if (fontLabel_ != -1) DeleteFontToHandle(fontLabel_);
    if (fontTick_ != -1) DeleteFontToHandle(fontTick_);
    if (rocketIcon_ != -1) DeleteGraph(rocketIcon_);   // ← 追加
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

    // right = cross(up=+Y, fwd)  (XZ 平面上での fwd の右手)
    const VECTOR right = VNorm(VCross(AsoUtility::AXIS_Y, fwd));

    // 世界の「北」は +Z 固定
    const VECTOR worldNorth = AsoUtility::DIR_F; // (0,0,1)

    // プレイヤー正面と北の相対角(度)
    const float cosPN = VDot(fwd, worldNorth);
    const float sinPN = VDot(right, worldNorth);
    const float northRel = atan2f(sinPN, cosPN) * (180.0f / DX_PI_F);

    constexpr float DEG_PER_PX = BAR_W / (VIEW_RANGE * 2.0f);

    const char* CARDINAL[8] = { "N", "NE", "E", "SE", "S", "SW", "W", "NW" };

    // ===== ループ範囲の絞り込み =====
    // 従来: [-180, 180] を TICK_STEP 刻みで全周ループし、
    //       中で ±VIEW_RANGE の外側は continue で捨てていた（無駄反復多数）
    // 改善: 画面に映る範囲を先に計算して、最初から必要な度数だけ回す
    //
    // 画面に映るのは「北基準の絶対度数 deg」が northRel ± VIEW_RANGE の範囲。
    // TICK_STEP 刻みで丸めて開始/終了を出す。
    const float startDegF = northRel - VIEW_RANGE;
    const float endDegF = northRel + VIEW_RANGE;

    // 5刻みに切り上げ/切り下げ（負値を含むため lroundf ではなく手動で）
    auto ceilToStep = [](float v, int step) {
        return static_cast<int>(std::ceil(v / step)) * step;
        };
    auto floorToStep = [](float v, int step) {
        return static_cast<int>(std::floor(v / step)) * step;
        };

    const int startDeg = ceilToStep(startDegF, TICK_STEP);
    const int endDeg = floorToStep(endDegF, TICK_STEP);

    for (int deg = startDeg; deg <= endDeg; deg += TICK_STEP)
    {
        const float rel = static_cast<float>(deg) - northRel; // ±VIEW_RANGE 内が保証される
        const int   x = cx + static_cast<int>(rel * DEG_PER_PX);
        if (x < barL || x > barR) continue; // 端の丸め対策で残す

        const int normDeg = ((deg % 360) + 360) % 360;

        if (normDeg % 45 == 0)
        {
            // 主目盛り + N/E/S/W ラベル
            DrawLine(x, barT, x, barB, 0xffffff);

            const char* label = CARDINAL[normDeg / 45];
            const int lw = GetDrawStringWidthToHandle(
                label, static_cast<int>(strlen(label)), fontLabel_);
            DrawStringToHandle(x - lw / 2, barB + 4, label, 0xffffff, fontLabel_);
        }
        else if (normDeg % 15 == 0)
        {
            // 中目盛り + 度数
            DrawLine(x, barT, x, barT + BAR_H * 2 / 3, 0xaaaaaa);

            char buf[8];
            sprintf_s(buf, "%d", normDeg);
            const int tw = GetDrawStringWidthToHandle(
                buf, static_cast<int>(strlen(buf)), fontTick_);
            DrawStringToHandle(x - tw / 2, barB + 4, buf, 0xaaaaaa, fontTick_);
        }
        else
        {
            // 小目盛りのみ
            DrawLine(x, barT, x, barT + BAR_H / 3, 0x888888);
        }
    }

    // 中央の▽インジケーター
    DrawTriangle(cx, barT - 2, cx - 7, barT - 14, cx + 7, barT - 14, 0xffffff, TRUE);

    // ===== ロケットマーカー =====
    VECTOR toRocket = VSub(rocket_.GetPos(), playerPos);
    toRocket.y = 0.0f;                              // XZ平面へ射影
    if (VSize(toRocket) < 0.0001f) toRocket = fwd;  // 真上/真下の対策
    toRocket = VNorm(toRocket);

    const float cosA = VDot(fwd, toRocket);
    const float sinA = VDot(right, toRocket);
    const float relRocket = atan2f(sinA, cosA) * (180.0f / DX_PI_F);

    int markerX = cx + static_cast<int>(relRocket * DEG_PER_PX);
    // 端クランプはアイコンの見た目半幅で行う（円のMARKER_Rから変更）
    if (markerX < barL + ICON_HALF_W) markerX = barL + ICON_HALF_W;
    if (markerX > barR - ICON_HALF_W) markerX = barR - ICON_HALF_W;

    const int markerCY = barT - ICON_Y_OFFSET;

    if (rocketIcon_ != -1)
    {
        // DrawRotaGraph は「中心(cx,cy)を起点に、スケール・回転を掛けて」描く。
        // アイコンの画像サイズを呼び出し側で気にしなくてよいのが利点。
        // 第4:スケール, 第5:回転角(rad), 第6:ハンドル, 第7:TRUE=透過色有効
        DrawRotaGraph(markerX, markerCY,
            ICON_SCALE, 0.0, rocketIcon_, TRUE);
    }
    else
    {
        // 画像読み込みに失敗した場合のフォールバック（旧・赤丸）
        DrawCircle(markerX, markerCY, MARKER_R, 0xff2020, TRUE);
        DrawCircle(markerX, markerCY, MARKER_R, 0xff6060, FALSE);
    }
}