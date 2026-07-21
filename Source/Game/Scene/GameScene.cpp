#include <DxLib.h>
#include "../../Component/StaticMeshComponent.h"
#include "../Scene/SceneManager.h"
#include "../../Manager/InputManager.h"
#include "../../Manager/SoundManager.h"
#include "../../Collision/CollisionManager.h"
#include "../Camera/Camera.h"
#include "../Actor/ActorBase.h"
#include "../Actor/Charactor/Player.h"
#include "../Actor/Charactor/Enemy/EnemyManager.h"
#include "../Actor/Charactor/Enemy/EnemyGiant.h"
#include "../Actor/Charactor/Enemy/EnemyBase.h"
#include "../Actor/Stage/StageManager.h"
#include "../Actor/Stage/Planet.h"
#include "../Actor/Stage/Rocket.h"
#include "../Actor/Item/ItemManager.h"
#include "../Actor/SkyDome.h"
#include "../UI/GameUI/StatusUI.h"
#include "../UI/GameUI/GuideUI.h"
#include "../UI/GameUI/InventoryUI.h"
#include "../UI/GameUI/RocketLocatorUI.h"
#include "GameScene.h"

GameScene::GameScene(void)
    : SceneBase()
{
}

GameScene::~GameScene(void)
{
    CollisionManager::GetInstance().Clear();

    enemyMng_->Release();
    itemMng_->Release();
    stageMng_->Release();

    for (auto& actor : actors_)
    {
        actor->Release();
    }
    actors_.clear();
}

void GameScene::Load(void)
{
}

void GameScene::Init(void)
{
    stageMng_ = std::make_unique<StageManager>();
    stageMng_->Init();

    itemMng_ = std::make_unique<ItemManager>();

    player_ = SpawnActor<Player>(itemMng_.get(), *stageMng_);
    player_->Init();

    camera_ = SpawnActor<Camera>();
    camera_->SetFollowTarget(player_);
    camera_->ChangeMode(Camera::MODE::FOLLOW);
    camera_->Init();

    enemyMng_ = std::make_unique<EnemyManager>(*player_);
    enemyMng_->Init();

    skyDome_ = SpawnActor<SkyDome>(player_);
    skyDome_->Init();

    gameUIs_.emplace_back(std::make_unique<StatusUI>(*player_));
    gameUIs_.emplace_back(std::make_unique<GuideUI>(*player_));
    gameUIs_.emplace_back(std::make_unique<InventoryUI>(player_->GetInventory()));
    gameUIs_.emplace_back(std::make_unique<RocketLocatorUI>(*player_, stageMng_->GetRocket()));

    for (auto& ui : gameUIs_) ui->Load();

    itemMng_->Init();
}

void GameScene::Update(void)
{
    SetMouseDispFlag(false);

    auto& ins = InputManager::GetInstance();
    if (ins.IsTriggered(InputManager::InputCommand::PAUSE))
    {
        sceMng_.PushOverlay(SceneManager::SCENE_ID::PAUSE);
        return;
    }

    stageMng_->Update();
    itemMng_->Update();
    enemyMng_->Update();

    for (auto& actor : actors_)
        actor->Update();

    // 当たり判定（全アクターの移動が終わった後に実行し、
    // その場で押し戻すことで描画前にめり込みを解消する）
    CollisionManager::GetInstance().Update();
}

void GameScene::Draw(void)
{
    camera_->SetBeforeDraw();
    skyDome_->Draw();

    int shadowMapHandle = CreateShadowMap();
    SetUseShadowMap(0, shadowMapHandle);

    stageMng_->Draw();
    itemMng_->Draw();
    enemyMng_->Draw();

    for (auto& actor : actors_)
    {
        if (actor.get() == skyDome_) continue;
        actor->Draw();
    }

    SetUseShadowMap(0, -1);
    DeleteShadowMap(shadowMapHandle);

    for (auto& ui : gameUIs_)
        ui->Draw();
}

int GameScene::CreateShadowMap(void)
{
    Vector3 playerPos = player_->GetPos();

    int shadowMapHandle = MakeShadowMap(1024, 1024);
    SetShadowMapLightDirection(shadowMapHandle, { 0.3f, -0.7f, 0.8f });
    SetShadowMapDrawArea(shadowMapHandle,
        VGet(playerPos.x - 1000.0f, playerPos.y - 1.0f, playerPos.z - 1000.0f),
        VGet(playerPos.x + 1000.0f, playerPos.y + 1000.0f, playerPos.z + 1000.0f));

    ShadowMap_DrawSetup(shadowMapHandle);

    // TODO: StaticMeshComponent 経由で描画するように変える
    auto* planetMesh = stageMng_->GetPlanet().GetComponent<StaticMeshComponent>();
    auto* playerMesh = player_->GetComponent<StaticMeshComponent>();
    if (planetMesh) MV1DrawModel(planetMesh->GetModelId());
    if (playerMesh) MV1DrawModel(playerMesh->GetModelId());

    ShadowMap_DrawEnd();

    return shadowMapHandle;
}