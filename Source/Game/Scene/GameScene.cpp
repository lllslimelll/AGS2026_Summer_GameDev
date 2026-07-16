#include <DxLib.h>
#include "../Scene/SceneManager.h"
#include "../../Manager/InputManager.h"
#include "../../Manager/SoundManager.h"
#include "../../Collision/CollisionManager.h"
#include "../Camera/Camera.h"
#include "../Actor/ActorBase.h"
#include "../Actor/Charactor/Player.h"
#include "../Actor/Charactor/Enemy/EnemyManager.h"
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
	// シーン終了時に CollisionManager をクリア
	CollisionManager::GetInstance().Clear();

	stageMng_->Release();

	itemMng_->Release();
	delete itemMng_;

	player_->Release();

	enemyManager_->Release();
	delete enemyManager_;

	skyDome_->Release();
	delete skyDome_;
}

void GameScene::Load(void)
{
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

	// アイテム
	itemMng_->Init();

	skyDome_->Init();
	
	camera_->SetFollow(&player_->GetTransform());// 追従対象の設定
	camera_->ChangeMode(Camera::MODE::FOLLOW);	 // モード変更

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

	// 当たり判定を全て実行
	CollisionManager::GetInstance().Update();

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
