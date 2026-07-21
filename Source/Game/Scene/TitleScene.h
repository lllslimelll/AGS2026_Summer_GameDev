#pragma once
#include <memory>
#include "../../Core/Vector3.h"
#include "../../Core/Quaternion.h"
#include "SceneBase.h"

class Camera;
class SkyDome;

class TitleScene : public SceneBase
{
public:

    TitleScene(void);
    ~TitleScene(void) override;

    void Load(void)   override {}
    void Init(void)   override;
    void Update(void) override;
    void Draw(void)   override;

private:

    // メニュー項目
    enum class MENU
    {
        GAME_START = 0,
        OPTION,
        QUIT_GAME,
        MAX
    };

    // レイアウト定数
    static constexpr int   MENU_COUNT = static_cast<int>(MENU::MAX);
    static constexpr int   MENU_CENTER_X = 430;
    static constexpr int   TITLE_X = 100;
    static constexpr int   MENU_Y_START = 630;
    static constexpr int   MENU_FONT_SIZE = 55;
    static constexpr int   TITLE_FONT_SIZE = 170;
    static constexpr int   MENU_LINE_HEIGHT = 80;
    static constexpr float PLANET_ROT_SPEED = 1.0f;

    int selectIndex_ = -1;
    int prevMouseX_ = -1;
    int prevMouseY_ = -1;

    std::unique_ptr<Camera>   camera_;
    std::unique_ptr<SkyDome>  skyDome_;

    int imgTitle_ = -1;
    int imgPushSpace_ = -1;

    // 球体惑星（モデル ID と姿勢を直接保持）
    int       planetModelId_ = -1;
    Vector3   planetPos_ = Vector3(900.0f, -600.0f, 300.0f);
    Quaternion planetRot_ = Quaternion::Identity();

    void UpdateInput(void);
    void DrawTitle(void) const;
    void DrawMenu(void)  const;
};