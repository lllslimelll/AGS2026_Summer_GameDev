#include <fstream>
#include <DxLib.h>
#include "../../Core/Vector2.h"
#include "../../Manager/InputManager.h"
#include "../../Component/StaticMeshComponent.h"
#include "SceneManager.h"
#include "../Camera/Camera.h"
#include "../Actor/Stage/Planet.h"
#include "../Actor/Stage/Rocket.h"
#include "../Actor/Stage/StageManager.h"
#include "DebugScene.h"

DebugScene::DebugScene(void)
    : SceneBase()
{
}

DebugScene::~DebugScene(void)
{
    if (stageMng_ != nullptr)
    {
        stageMng_->Release();
        delete stageMng_;
    }
}

void DebugScene::Init(void)
{
    stageMng_ = new StageManager();
    stageMng_->Init();

    // カメラ（FREE モードで自由移動）
    camera_ = std::make_unique<Camera>();
    camera_->Init();
    camera_->ChangeMode(Camera::MODE::FREE);
}

void DebugScene::Update(void)
{
    stageMng_->Update();
    camera_->Update();
    PlaceDebugPoint();
}

void DebugScene::Draw(void)
{
    camera_->SetBeforeDraw();
    stageMng_->Draw();

    int y = 20;
    for (const auto& point : points_)
    {
        DrawSphere3D(point, 30.0f, 16,
            GetColor(255, 0, 0), GetColor(255, 0, 0), false);
        DrawFormatString(20, y, 0x000000,
            "%.2f, %.2f, %.2f", point.x, point.y, point.z);
        y += 20;
    }
}

void DebugScene::PlaceDebugPoint(void)
{
    const auto& ins = InputManager::GetInstance();

    if (ins.IsTriggered(InputManager::InputCommand::SET_POINT))
    {
        int mouseX, mouseY;
        GetMousePoint(&mouseX, &mouseY);
        VECTOR screenPos = { static_cast<float>(mouseX),
                             static_cast<float>(mouseY),
                             1.0f };
        VECTOR worldPos = ConvScreenPosToWorldPos(screenPos);

        auto* mesh = stageMng_->GetPlanet().GetComponent<StaticMeshComponent>();
        if (mesh == nullptr) return;

        MV1_COLL_RESULT_POLY hit = MV1CollCheck_Line(
            mesh->GetModelId(), -1,
            camera_->GetPos().ToVECTOR(),
            worldPos);

        if (hit.HitFlag)
        {
            points_.push_back(hit.HitPosition);
        }
    }

    if (ins.IsTriggered(InputManager::InputCommand::DELETE_POINT))
    {
        if (!points_.empty()) points_.pop_back();
    }

    if (ins.IsTriggered(InputManager::InputCommand::SAVE_POINT))
    {
        SavePoints();
    }
}

void DebugScene::SavePoints(void)
{
    std::ofstream ofs("Data/Csv/PointSave.txt");
    if (!ofs) return;

    for (const VECTOR& p : points_)
    {
        ofs << p.x << " " << p.y << " " << p.z << "\n";
    }
}