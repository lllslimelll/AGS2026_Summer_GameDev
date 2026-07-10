#include <DxLib.h>
#include "../Scene/SceneManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/SoundManager.h"
#include "../Camera/Camera.h"
#include "../Object/Actor/ActorBase.h"
#include "../Object/Actor/Charactor/Player.h"
#include "../Object/Actor/Charactor/Enemy/EnemyManager.h"
#include "../Object/Actor/Stage/StageManager.h"
#include "../Object/Actor/Stage/Planet.h"
#include "../Object/Actor/Stage/Rocket.h"
#include "../Object/Actor/Item/ItemManager.h"
#include "../Object/Actor/SkyDome.h"
#include "../Object/UI/GameUI/StatusUI.h"
#include "../Object/UI/GameUI/GuideUI.h"
#include "../Object/UI/GameUI/InventoryUI.h"
#include "../Object/UI/GameUI/RocketLocatorUI.h"
#include "GameScene.h"

GameScene::GameScene(void)
	:
	stageMng_(nullptr),
	itemMng_(nullptr),
	player_(nullptr),
	camera_(nullptr),
	enemyManager_(nullptr),
	skyDome_(nullptr),
	SceneBase()
{
}

GameScene::~GameScene(void)
{
	stageMng_->Release();

	itemMng_->Release();
	delete itemMng_;

	player_->Release();

	enemyManager_->Release();
	delete enemyManager_;

	skyDome_->Release();
	delete skyDome_;
}

void GameScene::Init(void)
{
	stageMng_ = std::make_unique<StageManager>();
	stageMng_->Init();

	itemMng_ = new ItemManager();

	player_ = std::make_unique<Player>(itemMng_, *stageMng_);
	player_->Init();

	// カメラ
	camera_ = std::make_unique<Camera>();

	enemyManager_ = new EnemyManager(*player_);
	enemyManager_->Init();

	skyDome_ = new SkyDome(player_->GetTransform());

	gameUIs_.emplace_back(std::make_unique<StatusUI>(*player_));
	gameUIs_.emplace_back(std::make_unique<GuideUI>(*player_));
	gameUIs_.emplace_back(std::make_unique<InventoryUI>(player_->GetInventory()));
	gameUIs_.emplace_back(std::make_unique<RocketLocatorUI>(*player_, stageMng_->GetRocket()));
	for (auto& ui : gameUIs_)
	{
		ui->Load();
	}

	// モデルのコライダー
	const ColliderBase* planetCollider =
		stageMng_->GetPlanet().GetOwnCollider(static_cast<int>(Planet::COLLIDER_TYPE::MODEL));
	const ColliderBase* rocketCollider =
		stageMng_->GetRocket().GetOwnCollider(static_cast<int>(Rocket::COLLIDER_TYPE::MODEL));
	const ColliderBase* playerCollider =
		player_->GetOwnCollider(static_cast<int>(CharactorBase::COLLIDER_TYPE::CAPSULE));

	// アイテム
	itemMng_->Init();
	itemMng_->AddHitCollider(planetCollider);

	for (auto& e : enemyManager_->GetEnemies())
	{
		player_->AddHitCollider(
			e->GetOwnCollider(static_cast<int>(CharactorBase::COLLIDER_TYPE::CAPSULE)));
	}
	player_->AddHitCollider(planetCollider);
	player_->AddHitCollider(rocketCollider);

	enemyManager_->AddHitCollider(planetCollider); // ステージモデルのコライダー登録
	enemyManager_->AddHitCollider(playerCollider);

	skyDome_->Init();
	
	camera_->SetFollow(&player_->GetTransform());// 追従対象の設定
	camera_->ChangeMode(Camera::MODE::FOLLOW);	 // モード変更
	camera_->AddHitCollider(planetCollider);		 // ステージモデルのコライダー登録
	camera_->AddHitCollider(rocketCollider);

	player_->SetCameraTransform(&camera_->GetTransform()); // カメラのTransformをプレイヤーに渡す
}

void GameScene::Update(void)
{
	SetMouseDispFlag(false);
	auto& ins = InputManager::GetInstance();

	if (ins.IsTriggered(InputManager::InputCommand::PAUSE))
	{
		// ポーズシーンを追加
		sceMng_.PushOverlay(SceneManager::SCENE_ID::PAUSE);
		return;
	}

	// 更新
	stageMng_->Update();
	itemMng_->Update();
	player_->Update();
	camera_->Update();
	enemyManager_->Update();
	skyDome_->Update();

	player_->SetForward(camera_->GetForward()); // カメラの前方向をプレイヤーに渡す
}

void GameScene::Draw(void)
{
	// 描画前処理の適用
	camera_->SetBeforeDraw();
	skyDome_->Draw();

	// シャドウマップのハンドル
	int shadowMapHandle = CreateShadowMap();
	// 描画に使用するシャドウマップを設定
	SetUseShadowMap(0, shadowMapHandle);
	
	stageMng_->Draw();
	itemMng_->Draw();
	enemyManager_->Draw();
	player_->Draw();

	// 描画に使用するシャドウマップの設定を解除
	SetUseShadowMap(0, -1);
	// シャドウマップの削除
	DeleteShadowMap(shadowMapHandle);

	for (auto& ui : gameUIs_)
	{
		ui->Draw();
	}
}

// シャドウマップ作成
int GameScene::CreateShadowMap(void)
{
	// シャドウマップハンドルの作成
	int shadowMapHandle = MakeShadowMap(1024, 1024);
	// シャドウマップが想定するライトの方向もセット
	SetShadowMapLightDirection(shadowMapHandle, { 0.3f, -0.7f, 0.8f });
	// シャドウマップに描画する範囲を設定
	SetShadowMapDrawArea(shadowMapHandle,
		VGet(player_->GetTransform().pos.x - 1000.0f, player_->GetTransform().pos.y - 1.0f, player_->GetTransform().pos.z -1000.0f), 
		VGet(player_->GetTransform().pos.x + 1000.0f, player_->GetTransform().pos.y + 1000.0f, player_->GetTransform().pos.z + 1000.0f));

	// シャドウマップへの描画の準備
	ShadowMap_DrawSetup(shadowMapHandle);

	// シャドウマップへステージモデルの描画
	MV1DrawModel(stageMng_->GetTransform().modelId);
	// シャドウマップへキャラクターモデルの描画
	MV1DrawModel(player_->GetTransform().modelId);

	// シャドウマップへの描画を終了
	ShadowMap_DrawEnd();

	return shadowMapHandle;
}
