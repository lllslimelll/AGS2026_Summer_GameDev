// OptionScene.h
#pragma once
#include "SceneBase.h"

class OptionScene : public SceneBase
{
public:
    OptionScene(void);
    ~OptionScene(void) override;

    void Init(void)   override;
    void Update(void) override;
    void Draw(void)   override;

private:

    // ===== タブ =====
    enum class TAB
    {
        AUDIO = 0,   // オーディオ（音量）
        KEYBOARD,    // キーボード（マウス感度）
        GAMEPAD,     // ゲームパッド感度
        MAX
    };
    static constexpr int TAB_COUNT = static_cast<int>(TAB::MAX);

    // ===== 各タブの項目数 =====
    static constexpr int AUDIO_ITEM_COUNT = 2; // BGM, SE
    static constexpr int KEYBOARD_ITEM_COUNT = 1; // マウス感度
    static constexpr int GAMEPAD_ITEM_COUNT = 1; // パッド感度

    // ===== レイアウト定数（1920x1080想定） =====
    static constexpr int WIN_X = 0;
    static constexpr int WIN_Y = 0;
    static constexpr int WIN_W = 1920;
    static constexpr int WIN_H = 1080;

    static constexpr int TAB_X = WIN_X;
    static constexpr int TAB_Y = WIN_Y + 60;
    static constexpr int TAB_W = 190;
    static constexpr int TAB_H = 48;

    static constexpr int CONTENT_X = WIN_X + 40;
    static constexpr int CONTENT_Y = TAB_Y + TAB_H + 20;
    static constexpr int CONTENT_W = WIN_W - 80;

    static constexpr int ROW_H = 90;           // 1行の高さ
    static constexpr int LABEL_X = CONTENT_X;
    static constexpr int SLIDER_X = CONTENT_X + 480;
    static constexpr int SLIDER_W = 700;          // スライダー全体幅
    static constexpr int SLIDER_CY = 0;            // 行内の縦オフセット（中央）
    static constexpr int NODE_R = 10;           // ノード円の半径
    static constexpr int TRACK_H = 4;             // トラック線の太さ

    static constexpr int BTN_CLOSE_X = WIN_X + WIN_W - 160;
    static constexpr int BTN_CLOSE_Y = WIN_Y + 10;
    static constexpr int BTN_CLOSE_W = 140;
    static constexpr int BTN_CLOSE_H = 44;

    // ===== 状態 =====
    TAB  currentTab_ = TAB::AUDIO;
    int  selectRow_ = 0;   // タブ内の選択行

    // ドラッグ中スライダー情報
    bool  isDragging_ = false;
    int   dragItemGlobal_ = -1;  // ドラッグ中の全体アイテムインデックス
    int   prevMouseX_ = 0;
    bool  prevMouseBtn_ = false;

    // ===== ヘルパー =====
    // 現在タブの行数を返す
    int GetRowCount(void) const;

    // 全体アイテムインデックス → step値ポインタ・maxStepsを返す
    bool GetStepPtr(int globalIdx, int*& outStep, int& outMax);

    // 現在タブ・行 → 全体インデックス
    int ToGlobalIdx(TAB tab, int row) const;

    // スライダーのノード X 座標を計算
    int CalcNodeX(int step, int maxSteps) const;

    // マウス X 座標 → 最近傍ステップ
    int XToStep(int mouseX, int maxSteps) const;

    // 閉じるボタンのヒット判定
    bool IsHitCloseBtn(int mx, int my) const;

    // タブボタンのヒット判定（-1 = 外れ）
    int HitTestTab(int mx, int my) const;

    // スライダーノードのヒット判定（-1 = 外れ、>=0 = globalIdx）
    int HitTestSlider(int mx, int my) const;

    void ChangeValue(int dir);

    // ===== 更新 =====
    void UpdateInput(void);
    void UpdateMouse(void);

    // ===== 描画 =====
    void DrawWindow(void)  const;
    void DrawTabs(void)    const;
    void DrawContent(void) const;
    void DrawCloseBtn(void) const;

    // スライダー1本描画
    void DrawSlider(int cx, int cy, int step, int maxSteps,
        bool isSelected, bool isDrag) const;
};