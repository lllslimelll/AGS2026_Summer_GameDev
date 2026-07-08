#include <DxLib.h>
#include "../Scene/SceneManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/SoundManager.h"
#include "../Camera/Camera.h"
#include "../Object/Actor/ActorBase.h"
#include "../Object/Actor/Charactor/Player.h"
#include "../Object/Actor/Charactor/Enemy/EnemyManager.h"
#include "../Object/Actor/Stage.h"
#include "../Object/Actor/Item/ItemManager.h"
#include "../Object/Actor/SkyDome.h"
#include "../Object/UI/GameUI/StatusUI.h"
#include "GameScene.h"

GameScene::GameScene(void)
	:
	stage_(nullptr),
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
	stage_->Release();
	delete stage_;

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
	stage_ = new Stage();

	itemMng_ = new ItemManager();

	player_ = std::make_unique<Player>(itemMng_, stage_);

	// カメラ
	camera_ = std::make_unique<Camera>();

	enemyManager_ = new EnemyManager(player_.get());

	skyDome_ = new SkyDome(player_->GetTransform());

	gameUIs_.emplace_back(std::make_unique<StatusUI>(*player_));

	// 初期化
	stage_->Init();
	// ステージモデルのコライダー
	const ColliderBase* stageCollider =
		stage_->GetOwnCollider(static_cast<int>(Stage::COLLIDER_TYPE::MODEL));

	// アイテム
	itemMng_->Init();
	itemMng_->AddHitCollider(stageCollider);

	player_->Init();
	player_->AddHitCollider(stageCollider);	// ステージモデルのコライダー登録

	enemyManager_->Init();
	enemyManager_->AddHitCollider(stageCollider); // ステージモデルのコライダー登録
	enemyManager_->AddHitCollider(				  // キャラモデルのコライダー登録
		player_->GetOwnCollider(static_cast<int>(CharactorBase::COLLIDER_TYPE::CAPSULE)));

	skyDome_->Init();
	
	camera_->SetFollow(&player_->GetTransform());// 追従対象の設定
	camera_->ChangeMode(Camera::MODE::FOLLOW);	 // モード変更
	camera_->AddHitCollider(stageCollider);		 // ステージモデルのコライダー登録

	player_->SetCameraTransform(&camera_->GetTransform()); // カメラのTransformをプレイヤーに渡す
	player_->SetForward(camera_->GetForward()); // カメラの前方向をプレイヤーに渡す
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
	stage_->Update();
	itemMng_->Update();
	player_->Update();
	camera_->Update();
	enemyManager_->Update();
	skyDome_->Update();

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
	
	stage_->Draw();
	itemMng_->Draw();
	enemyManager_->Draw();
	player_->Draw();

	// 描画に使用するシャドウマップの設定を解除
	SetUseShadowMap(0, -1);
	// シャドウマップの削除
	DeleteShadowMap(shadowMapHandle);

	stage_->DrawUI();

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
	MV1DrawModel(stage_->GetTransform().modelId);
	// シャドウマップへキャラクターモデルの描画
	MV1DrawModel(player_->GetTransform().modelId);

	// シャドウマップへの描画を終了
	ShadowMap_DrawEnd();

	return shadowMapHandle;
}
