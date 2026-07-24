// OptionScene.cpp
#include <algorithm>
#include <DxLib.h>
#include "OptionScene.h"
#include "SceneManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/SettingsManager.h"
#include "../Manager/SoundManager.h"

// ===== 色定数 =====
static constexpr unsigned int COL_BG = 0x000000; // 黒背景
static constexpr unsigned int COL_BG_DARK = 0x000000;
static constexpr unsigned int COL_BORDER = 0xb86820; // 深オレンジ枠線
static constexpr unsigned int COL_TAB_ACT = 0x1a1000; // アクティブタブ
static constexpr unsigned int COL_TAB_INACT = 0x0a0800; // 非アクティブタブ
static constexpr unsigned int COL_TEXT = 0xc87830; // 通常テキスト（深オレンジ）
static constexpr unsigned int COL_TEXT_SEL = 0xf0a050; // 選択テキスト（明るいオレンジ）
static constexpr unsigned int COL_TRACK = 0x3a2010; // スライダー溝
static constexpr unsigned int COL_FILL = 0xb86820; // 埋まり部分
static constexpr unsigned int COL_NODE = 0xb86820; // ノード
static constexpr unsigned int COL_NODE_SEL = 0xf0a050; // 選択ノード
static constexpr unsigned int COL_NODE_DRAG = 0xffd080; // ドラッグノード
static constexpr unsigned int COL_BTN = 0x3a1a00; // 閉じるボタン
// =======================================================
OptionScene::OptionScene(void) {}
OptionScene::~OptionScene(void) {}
void OptionScene::Init(void) {}

// =======================================================
void OptionScene::Update(void)
{
    UpdateMouse();
    UpdateInput();
}

// =======================================================
// キーボード入力
void OptionScene::UpdateInput(void)
{
    auto& ins = InputManager::GetInstance();

    // タブ切り替え（LB/RB）
    if (ins.IsTriggered(InputManager::InputCommand::UI_TAB_LEFT) && !isDragging_)
    {
        int t = static_cast<int>(currentTab_);
        t = (t - 1 + TAB_COUNT) % TAB_COUNT;
        currentTab_ = static_cast<TAB>(t);
        selectRow_ = 0;
        SoundManager::GetInstance().PlaySelect();
    }
    if (ins.IsTriggered(InputManager::InputCommand::UI_TAB_RIGHT) && !isDragging_)
    {
        int t = static_cast<int>(currentTab_);
        t = (t + 1) % TAB_COUNT;
        currentTab_ = static_cast<TAB>(t);
        selectRow_ = 0;
        SoundManager::GetInstance().PlaySelect();
    }

    int rowCount = GetRowCount();
    if (ins.IsTriggered(InputManager::InputCommand::UI_UP))
    {
        selectRow_ = (selectRow_ - 1 + rowCount) % rowCount;
        SoundManager::GetInstance().PlaySelect();
    }
    if (ins.IsTriggered(InputManager::InputCommand::UI_DOWN))
    {
        selectRow_ = (selectRow_ + 1) % rowCount;
        SoundManager::GetInstance().PlaySelect();
    }

    if (ins.IsTriggered(InputManager::InputCommand::UI_LEFT))  ChangeValue(-1);
    if (ins.IsTriggered(InputManager::InputCommand::UI_RIGHT)) ChangeValue(+1);

    if (ins.IsTriggered(InputManager::InputCommand::UI_CANCEL))
    {
        SettingsManager::GetInstance().Apply();
        SceneManager::GetInstance().PopOverlay();
    }
}

// =======================================================
// マウス入力（D&D・クリック）
void OptionScene::UpdateMouse(void)
{
    int mx, my;
    GetMousePoint(&mx, &my);
    bool curBtn = (GetMouseInput() & MOUSE_INPUT_LEFT) != 0;

    // リリース瞬間
    if (!curBtn && prevMouseBtn_)
    {
        if (isDragging_)
        {
            isDragging_ = false;
            SettingsManager::GetInstance().Apply();
        }
        else if (IsHitCloseBtn(mx, my))
        {
            SettingsManager::GetInstance().Apply();
            SceneManager::GetInstance().PopOverlay();
            prevMouseBtn_ = curBtn;
            return;
        }
        else
        {
            int hitTab = HitTestTab(mx, my);
            if (hitTab >= 0)
            {
                currentTab_ = static_cast<TAB>(hitTab);
                selectRow_ = 0;
                SoundManager::GetInstance().PlaySelect(); 
            }
        }
    }

    // 押した瞬間：D&D開始チェック
    if (curBtn && !prevMouseBtn_)
    {
        int hitSlider = HitTestSlider(mx, my);
        if (hitSlider >= 0)
        {
            isDragging_ = true;
            dragItemGlobal_ = hitSlider;
            selectRow_ = hitSlider - ToGlobalIdx(currentTab_, 0);
        }
    }

    // ドラッグ中
    if (isDragging_)
    {
        int* stepPtr = nullptr;
        int  maxStep = 0;
        if (GetStepPtr(dragItemGlobal_, stepPtr, maxStep))
        {
            int prevStep = *stepPtr;
            *stepPtr = XToStep(mx, maxStep); 
            if (*stepPtr != prevStep)
            {
                SoundManager::GetInstance().PlaySelect();
            }
            SettingsManager::GetInstance().Apply();
        }
    }

    prevMouseBtn_ = curBtn;
    prevMouseX_ = mx;
}

// =======================================================
void OptionScene::Draw(void)
{
    DrawWindow();
    DrawTabs();
    DrawContent();
    DrawCloseBtn();
}

// =======================================================
void OptionScene::DrawWindow(void) const
{
    // 背景
    DrawBox(WIN_X, WIN_Y, WIN_X + WIN_W, WIN_Y + WIN_H, COL_BG, TRUE);
    // 枠線
    DrawBox(WIN_X, WIN_Y, WIN_X + WIN_W, WIN_Y + WIN_H, COL_BORDER, FALSE);

    // タイトル
    SetFontSize(36);
    DrawString(WIN_X + 30, WIN_Y + 14, "設定画面", COL_TEXT_SEL);
    SetFontSize(24);
}

// =======================================================
void OptionScene::DrawTabs(void) const
{
    const char* tabNames[TAB_COUNT] = { "オーディオ", "キーボード", "コントローラー" };

    for (int i = 0; i < TAB_COUNT; i++)
    {
        int tx = TAB_X + i * (TAB_W + 4);
        int ty = TAB_Y;
        bool isActive = (i == static_cast<int>(currentTab_));

        unsigned int bgCol = isActive ? COL_TAB_ACT : COL_TAB_INACT;
        unsigned int txtCol = isActive ? COL_TEXT_SEL : 0x888888;

        DrawBox(tx, ty, tx + TAB_W, ty + TAB_H, bgCol, TRUE);

        if (isActive)
        {
            // アクティブタブは上・左・右に枠線、下は本体に溶け込む
            DrawBox(tx, ty, tx + TAB_W, ty + TAB_H, COL_BORDER, FALSE);
            // 下辺を背景色で塗りつぶして「繋がっている」ように見せる
            DrawBox(tx + 1, ty + TAB_H - 2, tx + TAB_W - 1, ty + TAB_H + 2, bgCol, TRUE);
        }
        else
        {
            DrawBox(tx, ty, tx + TAB_W, ty + TAB_H, 0x555555, FALSE);
        }

        SetFontSize(22);
        // テキスト中央揃え
        int textW = GetDrawStringWidth(tabNames[i], -1);
        DrawString(tx + (TAB_W - textW) / 2, ty + 12, tabNames[i], txtCol);
    }

    // タブ下の仕切り線
    DrawBox(WIN_X, TAB_Y + TAB_H, WIN_X + WIN_W, TAB_Y + TAB_H + 2, COL_BORDER, TRUE);
}

// =======================================================
void OptionScene::DrawContent(void) const
{
    auto& s = SettingsManager::GetInstance();

    struct RowInfo
    {
        const char* label;
        int         step;
        int         maxSteps;
        int         globalIdx;
    };

    RowInfo rows[4];
    int rowCount = 0;

    switch (currentTab_)
    {
    case TAB::AUDIO:
        rows[0] = { "BGM 音量", s.bgmVolumeStep, SettingsManager::VOLUME_STEPS, 0 };
        rows[1] = { "SE  音量", s.seVolumeStep,  SettingsManager::VOLUME_STEPS, 1 };
        rowCount = 2;
        break;
    case TAB::KEYBOARD:
        rows[0] = { "マウス 感度", s.mouseSensStep, SettingsManager::SENS_STEPS, 2 };
        rowCount = 1;
        break;
    case TAB::GAMEPAD:
        rows[0] = { "コントローラー 感度", s.padSensStep, SettingsManager::SENS_STEPS, 3 };
        rowCount = 1;
        break;
    default:
        break;
    }

    // コンテンツ背景
    int cy0 = CONTENT_Y;
    DrawBox(WIN_X + 2, cy0, WIN_X + WIN_W - 2, WIN_Y + WIN_H - 2, COL_BG, TRUE);

    SetFontSize(26);

    for (int i = 0; i < rowCount; i++)
    {
        int rowY = cy0 + i * ROW_H + ROW_H / 2;  // 行の中央Y
        bool isSel = (i == selectRow_);
        bool isDrag = isDragging_ && (dragItemGlobal_ == rows[i].globalIdx);

        unsigned int lblCol = isSel ? COL_TEXT_SEL : COL_TEXT;

        // 選択行のハイライト背景
        if (isSel)
        {
            DrawBox(WIN_X + 4, rowY - ROW_H / 2 + 4,
                WIN_X + WIN_W - 4, rowY + ROW_H / 2 - 4,
                0x1a0e00, TRUE);
        }

        // ラベル
        DrawString(LABEL_X, rowY - 13, rows[i].label, lblCol);

        // スライダー
        DrawSlider(SLIDER_X, rowY, rows[i].step, rows[i].maxSteps, isSel, isDrag);

        // 区切り線（最終行以外）
        if (i < rowCount - 1)
        {
            DrawBox(CONTENT_X, rowY + ROW_H / 2 - 2,
                CONTENT_X + CONTENT_W, rowY + ROW_H / 2 - 1,
                0x444444, TRUE);
        }
    }
}

// =======================================================
void OptionScene::DrawSlider(int lx, int cy, int step, int maxSteps,
    bool isSelected, bool isDrag) const
{
    // トラック（背景溝）
    DrawBox(lx, cy - TRACK_H / 2,
        lx + SLIDER_W, cy + TRACK_H / 2,
        COL_TRACK, TRUE);

    // 埋まり部分
    int fillX = CalcNodeX(step, maxSteps);
    DrawBox(lx, cy - TRACK_H / 2,
        fillX, cy + TRACK_H / 2,
        COL_FILL, TRUE);

    // 各ステップのノード（小さい点）
    for (int j = 0; j < maxSteps; j++)
    {
        int nx = CalcNodeX(j, maxSteps);
        bool isThis = (j == step);

        unsigned int nodeCol;
        if (isThis && isDrag)         nodeCol = COL_NODE_DRAG;
        else if (isThis && isSelected) nodeCol = COL_NODE_SEL;
        else if (j < step)             nodeCol = COL_FILL;
        else                           nodeCol = COL_TRACK;

        int r = isThis ? NODE_R : NODE_R - 4;
        DrawCircle(nx, cy, r, nodeCol, TRUE);
        DrawCircle(nx, cy, r, 0x222222, FALSE); // 輪郭
    }

    // 現在値テキスト（右）
    SetFontSize(24);
    unsigned int txtCol = isSelected ? COL_TEXT_SEL : COL_TEXT;
    DrawFormatString(lx + SLIDER_W + 20, cy - 13, txtCol,
        "%d / %d", step + 1, maxSteps);
}

// =======================================================
void OptionScene::DrawCloseBtn(void) const
{
    int mx, my;
    GetMousePoint(&mx, &my);
    bool hover = IsHitCloseBtn(mx, my);

    unsigned int bg = hover ? 0x665533 : COL_BTN;
    unsigned int txt = hover ? 0xffffff : COL_TEXT;

    DrawBox(BTN_CLOSE_X, BTN_CLOSE_Y,
        BTN_CLOSE_X + BTN_CLOSE_W, BTN_CLOSE_Y + BTN_CLOSE_H, bg, TRUE);
    DrawBox(BTN_CLOSE_X, BTN_CLOSE_Y,
        BTN_CLOSE_X + BTN_CLOSE_W, BTN_CLOSE_Y + BTN_CLOSE_H, COL_BORDER, FALSE);

    SetFontSize(24);
    DrawString(BTN_CLOSE_X + 28, BTN_CLOSE_Y + 10, "閉じる", txt);
}

// =======================================================
// ヘルパー実装
// =======================================================

int OptionScene::GetRowCount(void) const
{
    switch (currentTab_)
    {
    case TAB::AUDIO:    return AUDIO_ITEM_COUNT;
    case TAB::KEYBOARD: return KEYBOARD_ITEM_COUNT;
    case TAB::GAMEPAD:  return GAMEPAD_ITEM_COUNT;
    default: return 0;
    }
}

int OptionScene::ToGlobalIdx(TAB tab, int row) const
{
    // AUDIO:0,1  KEYBOARD:2  GAMEPAD:3
    switch (tab)
    {
    case TAB::AUDIO:    return row;          // 0 or 1
    case TAB::KEYBOARD: return 2;
    case TAB::GAMEPAD:  return 3;
    default: return -1;
    }
}

bool OptionScene::GetStepPtr(int globalIdx, int*& outStep, int& outMax)
{
    auto& s = SettingsManager::GetInstance();
    switch (globalIdx)
    {
    case 0: outStep = &s.bgmVolumeStep; outMax = SettingsManager::VOLUME_STEPS; return true;
    case 1: outStep = &s.seVolumeStep;  outMax = SettingsManager::VOLUME_STEPS; return true;
    case 2: outStep = &s.mouseSensStep; outMax = SettingsManager::SENS_STEPS;   return true;
    case 3: outStep = &s.padSensStep;   outMax = SettingsManager::SENS_STEPS;   return true;
    default: return false;
    }
}

int OptionScene::CalcNodeX(int step, int maxSteps) const
{
    if (maxSteps <= 1) return SLIDER_X;
    return SLIDER_X + step * SLIDER_W / (maxSteps - 1);
}

int OptionScene::XToStep(int mouseX, int maxSteps) const
{
    float t = static_cast<float>(mouseX - SLIDER_X) / static_cast<float>(SLIDER_W);
    int step = static_cast<int>(t * (maxSteps - 1) + 0.5f);
    return std::clamp(step, 0, maxSteps - 1);
}

bool OptionScene::IsHitCloseBtn(int mx, int my) const
{
    return mx >= BTN_CLOSE_X && mx <= BTN_CLOSE_X + BTN_CLOSE_W &&
        my >= BTN_CLOSE_Y && my <= BTN_CLOSE_Y + BTN_CLOSE_H;
}

int OptionScene::HitTestTab(int mx, int my) const
{
    for (int i = 0; i < TAB_COUNT; i++)
    {
        int tx = TAB_X + i * (TAB_W + 4);
        if (mx >= tx && mx <= tx + TAB_W &&
            my >= TAB_Y && my <= TAB_Y + TAB_H)
        {
            return i;
        }
    }
    return -1;
}

int OptionScene::HitTestSlider(int mx, int my) const
{
    int rowCount = GetRowCount();
    int cy0 = CONTENT_Y;

    for (int i = 0; i < rowCount; i++)
    {
        int rowY = cy0 + i * ROW_H + ROW_H / 2;
        int globalIdx = ToGlobalIdx(currentTab_, i);

        int* stepPtr = nullptr;
        int   maxStep = 0;
        // const_cast: HitTestは副作用なし、GetStepPtrが non-const のため
        const_cast<OptionScene*>(this)->GetStepPtr(globalIdx, stepPtr, maxStep);

        // スライダー領域 + ノード半径分の余白
        int hitR = NODE_R + 6;
        if (mx >= SLIDER_X - hitR && mx <= SLIDER_X + SLIDER_W + hitR &&
            my >= rowY - hitR && my <= rowY + hitR)
        {
            return globalIdx;
        }
    }
    return -1;
}

void OptionScene::ChangeValue(int dir)
{
    int globalIdx = ToGlobalIdx(currentTab_, selectRow_);
    int* stepPtr = nullptr;
    int  maxStep = 0;
    if (GetStepPtr(globalIdx, stepPtr, maxStep))
    {
        *stepPtr = std::clamp(*stepPtr + dir, 0, maxStep - 1);
        SettingsManager::GetInstance().Apply();
        SoundManager::GetInstance().PlaySelect();
    }
}
