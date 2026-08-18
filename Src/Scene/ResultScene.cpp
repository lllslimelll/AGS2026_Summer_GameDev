#include "ResultScene.h"
#include "SceneManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/SoundManager.h"
#include <DxLib.h>

ResultScene::ResultScene()
    :
    SceneBase(),
    totalScore_(0),
    revealTimer_(0.0f)
{
}

ResultScene::~ResultScene()
{
}

void ResultScene::Init(void)
{
    totalScore_ = SceneManager::GetInstance().GetResultScore();
    revealTimer_ = 0.0f;
}

void ResultScene::Update(void)
{
    auto& ins = InputManager::GetInstance();
    bool isPad = (GetJoypadNum() != 0);

    revealTimer_ += SceneManager::GetInstance().GetDeltaTime();

    // 各項目が出た瞬間に1回だけ鳴らす
    if (!playedScore_ && revealTimer_ >= TIME_SCORE) { SoundManager::GetInstance().PlayResult(); playedScore_ = true; }
    if (!playedStatus_ && revealTimer_ >= TIME_STATUS) { SoundManager::GetInstance().PlayResult(); playedStatus_ = true; }
    if (!playedGrade_ && revealTimer_ >= TIME_GRADE) { SoundManager::GetInstance().PlayResult(); playedGrade_ = true; }

    // 全部表示されるまで操作不可
    if (revealTimer_ < TIME_GRADE) return;

    if (isPad)
    {
        if (ins.IsTriggered(InputManager::InputCommand::UI_DECIDE))
        {
            sceMng_.ChangeScene(SceneManager::SCENE_ID::TITLE);
        }
        return;
    }

    int screenW, screenH;
    GetScreenState(&screenW, &screenH, nullptr);

    constexpr int PANEL_H = 660;
    constexpr int BTN_W = 300;
    constexpr int BTN_H = 60;
    const int panelT = (screenH - PANEL_H) / 2;
    const int panelB = panelT + PANEL_H;
    const int btnL = (screenW - BTN_W) / 2;
    const int btnR = btnL + BTN_W;
    const int btnT = panelB - 80;
    const int btnB = btnT + BTN_H;

    int mouseX, mouseY;
    GetMousePoint(&mouseX, &mouseY);

    bool isHovering = (mouseX >= btnL && mouseX <= btnR &&
        mouseY >= btnT && mouseY <= btnB);

    if (isHovering && (GetMouseInput() & MOUSE_INPUT_LEFT))
    {
        sceMng_.ChangeScene(SceneManager::SCENE_ID::TITLE);
    }
}

void ResultScene::Draw(void)
{
    int screenW, screenH;
    GetScreenState(&screenW, &screenH, nullptr);

    constexpr int PANEL_W = 860;
    constexpr int PANEL_H = 660;
    constexpr int MARGIN = 40;
    constexpr int BTN_W = 300;
    constexpr int BTN_H = 60;
    constexpr int QUOTA = 5000;
    constexpr int GRADE_B = QUOTA + 2000;
    constexpr int ROW_H = 110;
    constexpr int ROW_FONT = 46;
    constexpr int CELL_GAP = 10;

    const int panelL = (screenW - PANEL_W) / 2;
    const int panelT = (screenH - PANEL_H) / 2;
    const int panelR = panelL + PANEL_W;
    const int panelB = panelT + PANEL_H;
    const int btnL = (screenW - BTN_W) / 2;
    const int btnR = btnL + BTN_W;
    const int btnT = panelB - 80;
    const int btnB = btnT + BTN_H;
    const int rowL = panelL + MARGIN;
    const int rowR = panelR - MARGIN;
    const int rowW = rowR - rowL;

    bool isPad = (GetJoypadNum() != 0);
    bool cleared = (totalScore_ >= QUOTA);

    // パネル背景
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);
    DrawBox(panelL, panelT, panelR, panelB, 0x333333, TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    DrawBox(panelL, panelT, panelR, panelB, 0xffffff, FALSE);

    int prevSize = GetFontSize();

    // ===== 探査結果 =====
    constexpr int TITLE_FONT = 70;
    SetFontSize(TITLE_FONT);
    const char* title = "探査結果";
    int titleW = GetDrawStringWidth(title, (int)strlen(title));
    DrawString((screenW - titleW) / 2, panelT + 25, title, 0xffff80);

    // ===== ノルマ金額行（3セル）=====
    const int row1T = panelT + 180;
    const int row1B = row1T + ROW_H;
    const int cell1W = (int)(rowW * 0.45f) - CELL_GAP;
    const int cell2W = (int)(rowW * 0.20f) - CELL_GAP;
    const int cell1L = rowL;
    const int cell1R = cell1L + cell1W;
    const int cell2L = cell1R + CELL_GAP;
    const int cell2R = cell2L + cell2W;
    const int cell3L = cell2R + CELL_GAP;
    const int cell3R = rowR;
    const int cellY = row1T + (ROW_H - ROW_FONT) / 2;

    auto drawCell = [&](int l, int t, int r, int b)
    {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 80);
            DrawBox(l, t, r, b, 0x000000, TRUE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
            DrawBox(l, t, r, b, 0x888888, FALSE);
    };

    drawCell(cell1L, row1T, cell1R, row1B);
    drawCell(cell2L, row1T, cell2R, row1B);
    drawCell(cell3L, row1T, cell3R, row1B);

    SetFontSize(ROW_FONT);

    // ノルマ金額ラベル（常に表示）
    const char* normaLabel = "ノルマ金額";
    int labelW = GetDrawStringWidth(normaLabel, (int)strlen(normaLabel));
    DrawString(cell1L + (cell1W - labelW) / 2, cellY, normaLabel, 0xdddddd);

    // 金額（TIME_SCORE後に表示）
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
    char scoreBuf[48];
    sprintf_s(scoreBuf, "$%s", scoreStr);

    if (revealTimer_ >= TIME_SCORE)
    {
        int scoreW = GetDrawStringWidth(scoreBuf, (int)strlen(scoreBuf));
        DrawString(cell3R - 20 - scoreW, cellY, scoreBuf,
            cleared ? 0x00ff40 : 0xffffff);
    }

    // 達成/未達成（TIME_STATUS後に表示）
    if (revealTimer_ >= TIME_STATUS)
    {
        const char* normaStatus = cleared ? "達成" : "未達成";
        unsigned int normaColor = cleared ? 0x00ff40 : 0xff4040;
        int statusW = GetDrawStringWidth(normaStatus, (int)strlen(normaStatus));
        DrawString(cell2L + (cell2W - statusW) / 2, cellY, normaStatus, normaColor);
    }

    // ===== グレード（TIME_GRADE後に表示）=====
    const char* grade;
    unsigned int gradeColor;

    if (totalScore_ < QUOTA) { grade = "C"; gradeColor = 0xff6060; }
    else if (totalScore_ < GRADE_B) { grade = "B"; gradeColor = 0x60c0ff; }
    else { grade = "A"; gradeColor = 0xffff40; }

    constexpr int GRADE_LABEL_FONT = 50;
    constexpr int GRADE_FONT = 160;

    SetFontSize(GRADE_LABEL_FONT);
    const char* gradeLabel = "グレード：";
    int gradeLabelW = GetDrawStringWidth(gradeLabel, (int)strlen(gradeLabel));

    SetFontSize(GRADE_FONT);
    int gradeW = GetDrawStringWidth(grade, (int)strlen(grade));

    const int gradeY = row1B + 60;
    const int gradeStartX = (screenW - (gradeLabelW + gradeW)) / 2;

    // グレードラベルは常に表示
    SetFontSize(GRADE_LABEL_FONT);
    DrawString(gradeStartX,
        gradeY + (GRADE_FONT - GRADE_LABEL_FONT) / 2,
        gradeLabel, 0xdddddd);

    // グレード英字はTIME_GRADE後に表示
    if (revealTimer_ >= TIME_GRADE)
    {
        SetFontSize(GRADE_FONT);
        DrawString(gradeStartX + gradeLabelW, gradeY, grade, gradeColor);
    }

    // ===== 帰還する（TIME_GRADE後に表示）=====
    if (revealTimer_ >= TIME_GRADE)
    {
        constexpr int HINT_FONT = 36;
        SetFontSize(HINT_FONT);

        if (isPad)
        {
            const char* hint = "[ A ] 帰還する";
            int hintW = GetDrawStringWidth(hint, (int)strlen(hint));
            DrawString((screenW - hintW) / 2, btnT + 12, hint, 0xaaaaaa);
        }
        else
        {
            int mouseX, mouseY;
            GetMousePoint(&mouseX, &mouseY);

            bool isHovering = (mouseX >= btnL && mouseX <= btnR &&
                mouseY >= btnT && mouseY <= btnB);

            unsigned int btnColor = isHovering ? 0xffffff : 0x888888;

            const char* hint = "帰還する";
            int hintW = GetDrawStringWidth(hint, (int)strlen(hint));
            DrawString((screenW - hintW) / 2, btnT + 12, hint, btnColor);

            if (isHovering)
            {
                DrawLine(
                    (screenW - hintW) / 2, btnT + 12 + HINT_FONT,
                    (screenW + hintW) / 2, btnT + 12 + HINT_FONT,
                    0xffffff);
            }
        }
    }

    SetFontSize(prevSize);
}