#pragma once
#include <string>
#include <vector>
#include "SceneBase.h"

class PrologueScene : public SceneBase
{
public:
    PrologueScene(void);
    ~PrologueScene(void) override;

    void Init(void) override;
    void Update(void) override;
    void Draw(void) override;

private:
    // 表示フェーズ ---------------------------------------------
    enum class Phase
    {
        MESSAGE_1,
        MESSAGE_2,
    };

    // 色分けセグメント（1つの断片 = 文字列 + 色）
    struct TextSegment
    {
        std::string  text;
        unsigned int color;
    };

    // 1メッセージ（2行分のセグメント列 + それぞれのY座標）
    struct Message
    {
        std::vector<TextSegment> line1;
        std::vector<TextSegment> line2;
        int line1Y;   // line1が空なら未使用
        int line2Y;   // line2が空なら未使用
    };

    // タイミング設計（1フェーズ内の秒）--------------------------
    static constexpr float PHASE_DURATION = 8.0f;
    static constexpr float LINE1_FADE_IN_BEGIN = 0.4f;
    static constexpr float LINE1_FADE_IN_END = 1.6f;
    static constexpr float LINE2_FADE_IN_BEGIN = 3.0f;
    static constexpr float LINE2_FADE_IN_END = 4.2f;
    static constexpr float FADE_OUT_BEGIN = 6.8f;
    static constexpr float FADE_OUT_END = 8.0f;
    //-----------------------------------------------------------

    static constexpr int FONT_SIZE = 60;

    // 受信HUD（MESSAGE_2のみ表示）---------------------------
    static constexpr float HUD_FADE_IN_BEGIN = 0.0f;
    static constexpr float HUD_FADE_IN_END = 0.6f;
    static constexpr float HUD_BLINK_HZ = 1.5f;  // 点滅の周波数[Hz]

    // 状態 -----------------------------------------------------
    Phase phase_;
    float elapsed_;
    bool  hasRequestedNext_;

    // Init で構築するデータ
    Message message1_;
    Message message2_;

    const Message& GetCurrentMessage(void) const;

    // フェーズ進行： MESSAGE_1 → MESSAGE_2 → GAMEシーンへ遷移
    void AdvancePhase(void);

    // [begin,end] を 0→1 に写像する cosine イージング
    float CosineEase01(float now, float begin, float end) const;

    // 色分けセグメント列を中央寄せで1行描画
    void DrawSegmentedCentered(int y, int alpha255,
        const std::vector<TextSegment>& segs) const;

    // 左上に受信HUDを描画
    void DrawReceivingHUD(int alpha255) const;
};