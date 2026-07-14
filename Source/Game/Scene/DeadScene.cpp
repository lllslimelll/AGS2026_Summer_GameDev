#include <DxLib.h>
#include "../Manager/InputManager.h"
#include "SceneManager.h"
#include "DeadScene.h"


DeadScene::DeadScene()
	:
	SceneBase(),
	menuIndex_(-1)
{
}

DeadScene::~DeadScene()
{
}

void DeadScene::Init(void)
{
}

void DeadScene::Update(void)
{
	UpdateDeadMenu();
}

void DeadScene::Draw(void)
{
	DrawDeadMenu();
}

void DeadScene::UpdateDeadMenu(void)
{
    auto& ins = InputManager::GetInstance();

    bool up = ins.IsTriggered(InputManager::InputCommand::UI_UP);
    bool down = ins.IsTriggered(InputManager::InputCommand::UI_DOWN);

    constexpr int MENU_MAX = static_cast<int>(MENU::MAX);
    if (up)   menuIndex_ = (menuIndex_ - 1 + MENU_MAX) % MENU_MAX;
    if (down) menuIndex_ = (menuIndex_ + 1) % MENU_MAX;

    // マウス選択
    int screenW, screenH;
    GetScreenState(&screenW, &screenH, nullptr);

    int mouseX, mouseY;
    GetMousePoint(&mouseX, &mouseY);

    bool mouseMoved = (mouseX != prevMouseX_ || mouseY != prevMouseY_);
    if (mouseMoved)
    {
        const char* labels[MENU_MAX] = { "リトライ", " タイトルへ" };

        constexpr int ITEM_SPAN = 110;
        constexpr int MENU_FONT = 70;
        int baseY = screenH / 2 - 20;

        int prevSize = GetFontSize();
        SetFontSize(MENU_FONT);

        int mouseHoverIndex = -1;
        for (int i = 0; i < MENU_MAX; i++)
        {
            int textW = GetDrawStringWidth(labels[i], (int)strlen(labels[i]));
            int x = (screenW - textW) / 2;
            int y = baseY + i * ITEM_SPAN;

            if (mouseX >= x - 60 && mouseX <= x + textW &&
                mouseY >= y && mouseY <= y + ITEM_SPAN)
            {
                mouseHoverIndex = i;
                break;
            }
        }

        SetFontSize(prevSize);
        menuIndex_ = mouseHoverIndex;
    }

    prevMouseX_ = mouseX;
    prevMouseY_ = mouseY;

    bool decide = ins.IsTriggered(InputManager::InputCommand::UI_DECIDE);
    if (!decide || menuIndex_ < 0) return;

    switch (static_cast<MENU>(menuIndex_))
    {
    case MENU::RETRY:
        sceMng_.ChangeScene(SceneManager::SCENE_ID::GAME);
        break;
    case MENU::TITLE:
        sceMng_.ChangeScene(SceneManager::SCENE_ID::TITLE);
        break;
    default: break;
    }
}

void DeadScene::DrawDeadMenu(void)
{
    int screenW, screenH;
    GetScreenState(&screenW, &screenH, nullptr);

    // 半透明黒オーバーレイ
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
    DrawBox(0, 0, screenW, screenH, 0x000000, TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    int prevSize = GetFontSize();

    // タイトル
    constexpr int TITLE_FONT = 140;
    SetFontSize(TITLE_FONT);
    const char* title = "調査失敗";
    int titleW = GetDrawStringWidth(title, (int)strlen(title));
    DrawString((screenW - titleW) / 2, screenH / 2 - 280, title, 0xff4040);

    // メニュー項目
    constexpr int MENU_FONT = 70;
    constexpr int MENU_MAX = static_cast<int>(MENU::MAX);
    constexpr int ITEM_SPAN = 110;
    SetFontSize(MENU_FONT);

    const char* labels[MENU_MAX] = { "リトライ", " タイトルへ" };
    int baseY = screenH / 2 - 20;

    for (int i = 0; i < MENU_MAX; i++)
    {
        bool selected = (i == menuIndex_);
        unsigned int color = selected ? 0xffff60 : 0xaaaaaa;

        int textW = GetDrawStringWidth(labels[i], (int)strlen(labels[i]));
        int x = (screenW - textW) / 2;
        int y = baseY + i * ITEM_SPAN;

        if (selected) { DrawString(x - 60, y, ">", color); }
        DrawString(x, y, labels[i], color);
    }

    SetFontSize(prevSize);
}
