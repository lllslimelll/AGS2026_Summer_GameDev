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
    // ステージ（Planet/Rocket を先に World に配置してから StageManager に渡す）
    auto* planet = SpawnActor<Planet>();
    planet->Init();
    auto* rocket = SpawnActor<Rocket>();
    rocket->Init();
    stageMng_ = SpawnActor<StageManager>(planet, rocket);

    // アイテム
    // Item 生成を SpawnActor 経由で行うためラムダを渡す
    itemMng_ = SpawnActor<ItemManager>(
        [this](const Item::ItemData& data) -> Item*
        {
            auto* item = SpawnActor<Item>(data);
            item->Init();
            return item;
        });

    // プレイヤー
    player_ = SpawnActor<Player>(itemMng_, *stageMng_);
    player_->Init();

    // カメラ
    camera_ = SpawnActor<Camera>();
    camera_->SetFollowTarget(player_);
    camera_->ChangeMode(Camera::MODE::FOLLOW);
    camera_->Init();

    // 敵
    // Enemy 生成を SpawnActor 経由で行うためラムダを渡す
    enemyMng_ = SpawnActor<EnemyManager>(
        *player_,
        [this](const EnemyBase::EnemyData& data) -> EnemyBase*
        {
            EnemyBase* enemy = nullptr;
            switch (data.type)
            {
            case EnemyBase::TYPE::GIANT:
                enemy = SpawnActor<EnemyGiant>(data, *player_);
                break;
            default: break;
            }
            if (enemy) enemy->Init();
            return enemy;
        });
    enemyMng_->Init();

    // スカイドーム（プレイヤーに追従）
    skyDome_ = SpawnActor<SkyDome>(player_);
    skyDome_->Init();

    // UI
    gameUIs_.emplace_back(std::make_unique<StatusUI>(*player_));
    gameUIs_.emplace_back(std::make_unique<GuideUI>(*player_));
    gameUIs_.emplace_back(std::make_unique<InventoryUI>(player_->GetInventory()));
    gameUIs_.emplace_back(std::make_unique<RocketLocatorUI>(*player_, stageMng_->GetRocket()));

    for (auto& ui : gameUIs_)
    {
        ui->Load();
    }

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

    // 当たり判定
    CollisionManager::GetInstance().Update();

    // 全 Actor を更新
    // Camera::UpdateFollow 内で player_->SetCameraForward() が呼ばれるため
    // カメラを先に更新する
    for (auto& actor : actors_)
    {
        actor->Update();
    }
}

void GameScene::Draw(void)
{
    camera_->SetBeforeDraw();

    // SkyDome は最初に描画（深度バッファに書き込まない）
    skyDome_->Draw();

    int shadowMapHandle = CreateShadowMap();
    SetUseShadowMap(0, shadowMapHandle);

    // 全 Actor を描画（SkyDome 以外）
    for (auto& actor : actors_)
    {
        if (actor.get() == skyDome_) continue;
        actor->Draw();
    }

    SetUseShadowMap(0, -1);
    DeleteShadowMap(shadowMapHandle);

    // UI 描画
    for (auto& ui : gameUIs_)
    {
        ui->Draw();
    }
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