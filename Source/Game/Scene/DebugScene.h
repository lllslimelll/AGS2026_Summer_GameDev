#pragma once
#include <vector>
#include <memory>
#include <DxLib.h>
#include "SceneBase.h"

class Camera;
class Planet;
class Rocket;
class StageManager;

class DebugScene : public SceneBase
{
public:

    DebugScene(void);
    ~DebugScene(void) override;

    void Load(void)   override {}
    void Init(void)   override;
    void Update(void) override;
    void Draw(void)   override;

private:

    std::unique_ptr<Camera> camera_;

    Planet* planet_ = nullptr;
    Rocket* rocket_ = nullptr;
    StageManager* stageMng_ = nullptr;

    // デバッグポイント（DxLib の描画関数に渡すため VECTOR で保持）
    std::vector<VECTOR> points_;

    void PlaceDebugPoint(void);
    void SavePoints(void);
};