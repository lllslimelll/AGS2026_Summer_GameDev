#include <cmath>
#include <DxLib.h>
#include "../../Application.h"
#include "../../Common/AnimationController.h"
#include "../Actor/SkyDome.h"
#include "../../Manager/InputManager.h"
#include "../../Manager/ResourceManager.h"
#include "../../Manager/SoundManager.h"
#include "../Camera/Camera.h"
#include "../../Utility/Math.h"
#include "SceneManager.h"
#include "TitleScene.h"

// メニューラベル（MENU の順に対応）
static const char* MENU_LABELS[] =
{
    "探査を開始",
    "オプション",
    "終了"
};

// 水平の点線（DxLib に点線 API はないので短い破線を並べる）
static void DrawDottedLineH(int x1, int x2, int y, unsigned int color,
    int dashLen, int gapLen, int thickness)
{
    for (int x = x1; x < x2; x += dashLen + gapLen)
    {
        const int ex = (x + dashLen < x2) ? x + dashLen : x2;
        DrawBox(x, y, ex, y + thickness, color, TRUE);
    }
}

static constexpr unsigned int COLOR_SELECT = 0xFF2222;
static constexpr unsigned int COLOR_UNSELECT = 0xFFFFFF;
static constexpr unsigned int COLOR_TITLE = 0xFFFFFF;
static constexpr int          LINE_WIDTH = 500;
static constexpr int          LINE_PAD_Y = 130;
static constexpr unsigned int COLOR_LINE = 0xFFFFFF;

TitleScene::TitleScene(void)
    : SceneBase()
{
}

TitleScene::~TitleScene(void)
{
    if (skyDome_) skyDome_->Release();
    if (planetModelId_ >= 0) MV1DeleteModel(planetModelId_);
}

void TitleScene::Init(void)
{
    SoundManager::GetInstance().PlayBgmTitle();

    // ---- 球体惑星 ----
    planetModelId_ = resMng_.Load(ResourceManager::SRC::MAIN_STAGE).handleId_;

    // 初期姿勢：Y / X / Z 軸まわりに傾ける
    planetRot_ = Quaternion::AngleAxis(Math::ToRadian(50.0f), Vector3::UP)
        * Quaternion::AngleAxis(Math::ToRadian(-20.0f), Vector3::RIGHT)
        * Quaternion::AngleAxis(Math::ToRadian(30.0f), Vector3::FORWARD);

    // DxLib にモデル行列を設定
    // TODO: Matrix4x4::Compose -> MV1SetMatrix に移行する（現状はオイラー変換で代替）
    MV1SetScale(planetModelId_, VGet(0.7f, 0.7f, 0.7f));
    MV1SetRotationXYZ(planetModelId_, planetRot_.ToEuler().ToVECTOR());
    MV1SetPosition(planetModelId_, planetPos_.ToVECTOR());

    // ---- カメラ（固定点）----
    camera_ = std::make_unique<Camera>();
    camera_->Init();
    camera_->ChangeMode(Camera::MODE::FIXED_POINT);

    // ---- スカイドーム（タイトルは追従対象なし = STAY モード）----
    skyDome_ = std::make_unique<SkyDome>(nullptr);
    skyDome_->Init();
}

void TitleScene::Update(void)
{
    camera_->Update();
    skyDome_->Update();

    // 惑星を Y 軸まわりにゆっくり回転させる
    planetRot_ = planetRot_
        * Quaternion::AngleAxis(Math::ToRadian(-0.05f), Vector3::UP);

    // DxLib にモデル行列を更新
    MV1SetRotationXYZ(planetModelId_, planetRot_.ToEuler().ToVECTOR());

    UpdateInput();
}

void TitleScene::UpdateInput(void)
{
    auto const& ins = InputManager::GetInstance();

    // キーボード・パッドによるメニュー選択
    if (ins.IsTriggered(InputManager::InputCommand::UI_UP))
    {
        selectIndex_ = (selectIndex_ - 1 + MENU_COUNT) % MENU_COUNT;
    }
    if (ins.IsTriggered(InputManager::InputCommand::UI_DOWN))
    {
        selectIndex_ = (selectIndex_ + 1) % MENU_COUNT;
    }

    // マウスホバー判定（マウスが動いた時だけ更新）
    int mouseX, mouseY;
    GetMousePoint(&mouseX, &mouseY);
    bool mouseMoved = (mouseX != prevMouseX_ || mouseY != prevMouseY_);

    if (mouseMoved)
    {
        int hoverIndex = -1;
        const int halfW = LINE_WIDTH / 2;
        for (int i = 0; i < MENU_COUNT; ++i)
        {
            int itemY = MENU_Y_START + i * MENU_LINE_HEIGHT;
            if (mouseX >= MENU_CENTER_X - halfW && mouseX <= MENU_CENTER_X + halfW &&
                mouseY >= itemY && mouseY <= itemY + MENU_LINE_HEIGHT)
            {
                hoverIndex = i;
                break;
            }
        }
        selectIndex_ = hoverIndex;
    }

    prevMouseX_ = mouseX;
    prevMouseY_ = mouseY;

    // 決定
    if (ins.IsTriggered(InputManager::InputCommand::UI_DECIDE) && selectIndex_ != -1)
    {
        switch (static_cast<MENU>(selectIndex_))
        {
        case MENU::GAME_START:
            sceMng_.ChangeScene(SceneManager::SCENE_ID::GAME);
            SoundManager::GetInstance().StopBGMTitle();
            break;
        case MENU::QUIT_GAME:
            PostQuitMessage(0);
            break;
        default:
            break;
        }
    }
}

void TitleScene::Draw(void)
{
    camera_->SetBeforeDraw();

    // スカイドーム
    skyDome_->Draw();

    // 球体惑星
    MV1DrawModel(planetModelId_);

    DrawTitle();
    DrawMenu();
}

void TitleScene::DrawTitle(void) const
{
    SetFontSize(TITLE_FONT_SIZE);
    DrawFormatString(TITLE_X, 190, 0xFF8C00, "惑星探査");
}

void TitleScene::DrawMenu(void) const
{
    // メニューを囲む上下の点線
    const int lastItemY = MENU_Y_START + (MENU_COUNT - 1) * MENU_LINE_HEIGHT;
    const int frameTop = MENU_Y_START - LINE_PAD_Y;
    const int frameBottom = lastItemY + MENU_FONT_SIZE + LINE_PAD_Y;

    DrawDottedLineH(MENU_CENTER_X - LINE_WIDTH / 2, MENU_CENTER_X + LINE_WIDTH / 2,
        frameTop, COLOR_LINE, 2, 3, 1);
    DrawDottedLineH(MENU_CENTER_X - LINE_WIDTH / 2, MENU_CENTER_X + LINE_WIDTH / 2,
        frameBottom, COLOR_LINE, 2, 3, 1);

    SetFontSize(MENU_FONT_SIZE);

    const int triH = MENU_FONT_SIZE;
    const int triW = MENU_FONT_SIZE / 2;
    const int gap = MENU_FONT_SIZE / 3;

    for (int i = 0; i < MENU_COUNT; ++i)
    {
        const bool isSelected = (i == selectIndex_);
        const int  y = MENU_Y_START + i * MENU_LINE_HEIGHT;

        const int len = static_cast<int>(std::strlen(MENU_LABELS[i]));
        const int textW = GetDrawStringWidth(MENU_LABELS[i], len);
        const int textX = MENU_CENTER_X - textW / 2;

        const unsigned int color = isSelected ? 0xFFFFFF : 0xFF8C00;
        DrawFormatString(textX, y, color, MENU_LABELS[i]);

        if (!isSelected) continue;

        const int cy = y + triH / 2;

        // 左：右向き三角
        const int lTip = textX - gap;
        const int lBase = lTip - triW;
        DrawTriangle(lBase, cy - triH / 2, lBase, cy + triH / 2, lTip, cy, 0xFFFFFF, TRUE);

        // 右：左向き三角
        const int rTip = textX + textW + gap;
        const int rBase = rTip + triW;
        DrawTriangle(rBase, cy - triH / 2, rBase, cy + triH / 2, rTip, cy, 0xFFFFFF, TRUE);
    }
}