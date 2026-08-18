#include <cmath>
#include <DxLib.h>
#include "../Application.h"
#include "../Manager/InputManager.h"
#include "../Manager/SoundManager.h"
#include "SceneManager.h"
#include "PrologueScene.h"

namespace
{
    constexpr unsigned int COLOR_WHITE = 0xFFFFFF;
    constexpr unsigned int COLOR_GREEN = 0x33DD33; // 画像の緑に近い色
    constexpr float PI_F = 3.14159265f;
}

PrologueScene::PrologueScene(void)
    :
    SceneBase(),
    phase_(Phase::MESSAGE_1),
    elapsed_(0.0f),
    hasRequestedNext_(false)
{
}

PrologueScene::~PrologueScene(void)
{
}

void PrologueScene::Init(void)
{
    // 画面中心Yから、フォント高さぶんを引くと「文字の中央」が画面中央に来る
    constexpr int CENTER_Y = Application::SCREEN_SIZE_Y / 2;
    constexpr int HALF_FONT = FONT_SIZE / 2;
    constexpr int TWO_LINE_GAP = 260; // 2行構成のときの行間

    // ---- メッセージ1（2行構成：上下に振り分け）----
    message1_.line1 = {
        { "とある惑星から高エネルギー反応を検知。", COLOR_WHITE },
    };
    message1_.line2 = {
        { "探査員(プレイヤー)",                    COLOR_GREEN },
        { "がレア鉱物を求めて星へ乗り込む...",     COLOR_WHITE },
    };
    message1_.line1Y = CENTER_Y - TWO_LINE_GAP / 2 - HALF_FONT;
    message1_.line2Y = CENTER_Y + TWO_LINE_GAP / 2 - HALF_FONT;

    // ---- メッセージ2（1行構成：ど真ん中）----
    // ヘッダ情報は左上の受信HUDへ移したので、本文（引用文）のみ表示
    message2_.line1 = {};
    message2_.line2 = {
        { "「鉱石を回収し、ロケットに",  COLOR_WHITE },
        { "納品",                         COLOR_GREEN },
        { "せよ。」",                     COLOR_WHITE },
    };
    message2_.line1Y = 0;                       // 未使用
    message2_.line2Y = CENTER_Y - HALF_FONT;
}

void PrologueScene::Update(void)
{
    elapsed_ += sceMng_.GetDeltaTime();

    // スキップ判定
    const auto& ins = InputManager::GetInstance();
    const bool skip =
        ins.IsTriggered(InputManager::InputCommand::UI_DECIDE) ||
        ins.IsTriggered(InputManager::InputCommand::UI_CANCEL) ||
        ins.IsTriggered(InputManager::InputCommand::JUMP) ||
        ins.IsTriggered(InputManager::InputCommand::PAUSE);

    if (elapsed_ >= PHASE_DURATION || skip)
    {
        AdvancePhase();
    }
}

void PrologueScene::AdvancePhase(void)
{
    if (hasRequestedNext_) { return; }

    switch (phase_)
    {
    case Phase::MESSAGE_1:
        phase_ = Phase::MESSAGE_2;
        elapsed_ = 0.0f;
        break;

    case Phase::MESSAGE_2:
        sceMng_.ChangeScene(SceneManager::SCENE_ID::GAME);
        hasRequestedNext_ = true;
        break;
    }
}

const PrologueScene::Message& PrologueScene::GetCurrentMessage(void) const
{
    return (phase_ == Phase::MESSAGE_1) ? message1_ : message2_;
}

void PrologueScene::Draw(void)
{
    // 背景（黒）
    DrawBox(0, 0,
        Application::SCREEN_SIZE_X, Application::SCREEN_SIZE_Y,
        0x000000, TRUE);

    SetFontSize(FONT_SIZE);

    // フェーズ末尾のグローバル・フェードアウト（1 → 0）
    // このフェーズの [FADE_OUT_BEGIN, FADE_OUT_END] で 1 から 0 へ滑らかに落ちる
    const float globalA =
        1.0f - CosineEase01(elapsed_, FADE_OUT_BEGIN, FADE_OUT_END);

    // 各行のフェードイン α × グローバル α
    // → 表示中は各行 α で決まり、末尾で全体が沈む「エンベロープ合成」
    const float a1 = CosineEase01(elapsed_, LINE1_FADE_IN_BEGIN, LINE1_FADE_IN_END) * globalA;
    const float a2 = CosineEase01(elapsed_, LINE2_FADE_IN_BEGIN, LINE2_FADE_IN_END) * globalA;

    const int alpha1 = static_cast<int>(a1 * 255.0f);
    const int alpha2 = static_cast<int>(a2 * 255.0f);

    const Message& msg = GetCurrentMessage();
    DrawSegmentedCentered(msg.line1Y, alpha1, msg.line1);
    DrawSegmentedCentered(msg.line2Y, alpha2, msg.line2);

    if (phase_ == Phase::MESSAGE_2)
    {
        const float aHud = CosineEase01(elapsed_, HUD_FADE_IN_BEGIN, HUD_FADE_IN_END) * globalA;
        DrawReceivingHUD(static_cast<int>(aHud * 255.0f));
    }
}

// -----------------------------------------------------------------------------
// alpha(t) = (1 - cos(π * t)) / 2
//   ・t ? begin → 0
//   ・t ? end   → 1
//   ・両端で導関数 0（動き出し／止まりがやわらかい）
// -----------------------------------------------------------------------------
float PrologueScene::CosineEase01(float now, float begin, float end) const
{
    if (now <= begin) { return 0.0f; }
    if (now >= end) { return 1.0f; }

    const float t = (now - begin) / (end - begin);
    return 0.5f - 0.5f * std::cosf(PI_F * t);
}

// 色分けセグメント列を1行として中央寄せ描画する
// 総幅を先に計算 → 開始X を決める → 順に描く、の3ステップ
void PrologueScene::DrawSegmentedCentered(int y, int alpha255,
    const std::vector<TextSegment>& segs) const
{
    if (alpha255 <= 0 || segs.empty()) { return; }

    // 総幅
    int totalW = 0;
    for (const auto& s : segs)
    {
        totalW += GetDrawStringWidth(
            s.text.c_str(), static_cast<int>(s.text.size()));
    }

    int x = (Application::SCREEN_SIZE_X - totalW) / 2;

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha255);
    for (const auto& s : segs)
    {
        DrawString(x, y, s.text.c_str(), s.color);
        x += GetDrawStringWidth(
            s.text.c_str(), static_cast<int>(s.text.size()));
    }
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

// 左上に「メッセージ受信中」風のHUDを描画する
// ・左端に緑の縦アクセントバー
// ・上段：点滅ドット + 「通信受信」
// ・下段：送信元 / 宛先（メールヘッダ風）
void PrologueScene::DrawReceivingHUD(int alpha255) const
{
    if (alpha255 <= 0) { return; }

    // レイアウト
    constexpr int HUD_X = 80;
    constexpr int HUD_Y = 70;
    constexpr int HUD_HEAD_FONT = 36;
    constexpr int HUD_SUB_FONT = 24;
    constexpr int HUD_HEAD_TO_SUB = 48;   // 見出し → 送信元 の間隔
    constexpr int HUD_SUB_GAP = 34;   // 送信元 → 宛先   の間隔
    constexpr int BAR_W = 4;
    constexpr int BAR_LEFT_MARGIN = 20;
    constexpr int DOT_RADIUS = 9;

    // ---- 点滅α（sin を [0.4, 1.0] にリマップ）---------------
    const float raw = std::sinf(2.0f * PI_F * HUD_BLINK_HZ * elapsed_);
    const float blink01 = 0.4f + 0.6f * ((raw + 1.0f) * 0.5f);
    const int   dotAlpha = static_cast<int>(alpha255 * blink01);

    // ---- 各行のY座標を先に計算しておく ------------------------
    const int headY = HUD_Y;
    const int subY1 = headY + HUD_HEAD_TO_SUB + 6;               // 送信元
    const int subY2 = subY1 + HUD_SUB_GAP;                       // 宛先

    // ---- 左端アクセントバー（3行分をカバーする高さ）---------
    const int barTop = headY - 6;
    const int barBottom = subY2 + HUD_SUB_FONT + 6;

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha255);
    DrawBox(
        HUD_X - BAR_LEFT_MARGIN - BAR_W, barTop,
        HUD_X - BAR_LEFT_MARGIN, barBottom,
        COLOR_GREEN, TRUE);

    // ---- 点滅ドット ------------------------------------------
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, dotAlpha);
    DrawCircle(HUD_X + DOT_RADIUS, headY + HUD_HEAD_FONT / 2 + 2,
        DOT_RADIUS, COLOR_GREEN, TRUE);

    // ---- 見出し ----------------------------------------------
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha255);
    SetFontSize(HUD_HEAD_FONT);
    DrawString(HUD_X + DOT_RADIUS * 2 + 16, headY,
        "通信受信", COLOR_GREEN);

    // ---- 送信元 / 宛先 ---------------------------------------
    SetFontSize(HUD_SUB_FONT);
    DrawString(HUD_X, subY1, "送信元: 惑星探査指令局", COLOR_WHITE);
    DrawString(HUD_X, subY2, "宛先　: 探査員", COLOR_WHITE);

    // ---- 後始末 ----------------------------------------------
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    SetFontSize(FONT_SIZE);
}