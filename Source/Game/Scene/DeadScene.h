#pragma once
#include "SceneBase.h"

class DeadScene : public SceneBase
{
public:

    DeadScene();
    ~DeadScene() override;

    // ÉçÅ[Éh
    void Load(void) override {};

    void Init(void)   override;
    void Update(void) override;
    void Draw(void)   override;

private:

    enum class MENU
    {
        RETRY,
        TITLE,
        MAX,
    };

    int menuIndex_;

    int prevMouseX_ = -1;
    int prevMouseY_ = -1;

    void UpdateDeadMenu(void);
    void DrawDeadMenu(void);
};