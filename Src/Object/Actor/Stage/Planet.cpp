#include "../../../Manager/ResourceManager.h"
#include "../../../Scene/SceneManager.h"
#include "../../../Camera/Camera.h"
#include "../../Common/Transform.h"
#include "../../../Utility/AsoUtility.h"
#include "../../Collider/ColliderModel.h"
#include "Planet.h"

Planet::~Planet(void)
{
}

void Planet::Update(void)
{
    transform_.Update();

    // カメラ座標をVSに送る
    Camera& cam = SceneManager::GetInstance().GetCamera();
    VECTOR camPos = cam.GetPos(); // 要確認: Camera側のgetter名
    material_->SetConstBufVS(0, {camPos.x, camPos.y, camPos.z, 0.0f});
}

void Planet::Draw(void)
{
    renderer_->Draw();
}

void Planet::InitLoad(void)
{
    transform_.SetModel(resMng_.Load(
        ResourceManager::SRC::MAIN_STAGE).handleId_);
}

void Planet::InitTransform(void)
{
    transform_.scl = { 2.0f, 2.0f, 2.0f };
    transform_.pos = { 0.0f, 0.0f, 0.0f };
    transform_.Update();
}

void Planet::InitCollider(void)
{
    MV1SetupCollInfo(transform_.modelId);

    ColliderModel* colModel =
        new ColliderModel(ColliderBase::TAG::PLANET, &transform_);

    for (const std::string& name : EXCLUDE_FRAME_NAMES)
    {
        colModel->AddExcludeFrameIds(name);
    }

    for (const std::string& name : TARGET_FRAME_NAMES)
    {
        colModel->AddTargetFrameIds(name);
    }

    colModel->SetOccluder(true);

    ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::MODEL), colModel);
}

void Planet::InitAnimation(void)
{
}

void Planet::InitPost(void)
{
    // モデル描画用
    // マテリアル
    material_ = std::make_unique<ModelMaterial>(
        "StageVS.vso", 1,
        "StagePS.pso", 0);

    Camera& cam = SceneManager::GetInstance().GetCamera();
    VECTOR camPos = cam.GetPos();
    material_->AddConstBufVS({ camPos.x, camPos.y, camPos.z, 0.0f });

    // レンダラー
    renderer_ = std::make_unique<ModelRenderer>(
        *material_, transform_.modelId);
}