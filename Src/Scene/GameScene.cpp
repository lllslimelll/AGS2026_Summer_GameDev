#include <DxLib.h>
#include "../Manager/SceneManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/Camera.h"
#include "../Object/Actor/ActorBase.h"
#include "../Object/Actor/Charactor/Player.h"
#include "../Object/Actor/Charactor/Enemy/EnemyManager.h"
#include "../Object/Actor/Stage.h"
#include "../Object/Actor/SkyDome.h"
#include "GameScene.h"

GameScene::GameScene(void)
	:
	stage_(nullptr),
	player_(nullptr),
	enemyManager_(nullptr),
	skyDome_(nullptr),
	allActor_(),
	SceneBase()
{
}

GameScene::~GameScene(void)
{
}

void GameScene::Init(void)
{
	stage_ = new Stage();

	player_ = new Player();

	enemyManager_ = new EnemyManager(player_);

	skyDome_ = new SkyDome(player_->GetTransform());

	// 初期化
	stage_->Init();
	// ステージモデルのコライダー
	const ColliderBase* stageCollider =
		stage_->GetOwnCollider(static_cast<int>(Stage::COLLIDER_TYPE::MODEL));

	player_->Init();
	player_->AddHitCollider(stageCollider);	// ステージモデルのコライダー登録

	enemyManager_->Init();
	enemyManager_->AddHitCollider(stageCollider); // ステージモデルのコライダー登録
	enemyManager_->AddHitCollider(				  // キャラモデルのコライダー登録
		player_->GetOwnCollider(static_cast<int>(CharactorBase::COLLIDER_TYPE::CAPSULE)));

	skyDome_->Init();

	Camera* camera = sceMng_.GetCamera();
	camera->SetFollow(&player_->GetTransform());
	camera->ChangeMode(Camera::MODE::FOLLOW);
	camera->AddHitCollider(stageCollider); // ステージモデルのコライダー登録
}

void GameScene::Update(void)
{

	// シーン遷移
	auto const& ins = InputManager::GetInstance();
	if (ins.IsTrgDown(KEY_INPUT_SPACE))
	{
		sceMng_.ChangeScene(SceneManager::SCENE_ID::TITLE);
	}

	// 更新
	stage_->Update();
	player_->Update();
	enemyManager_->Update();
	skyDome_->Update();

}

void GameScene::Draw(void)
{
	skyDome_->Draw();
	stage_->Draw();
	player_->Draw();
	enemyManager_->Draw();
	// 影描画
	DrawShadow();
}

void GameScene::Release(void)
{
	stage_->Release();
	delete stage_;

	player_->Release();
	delete player_;

	enemyManager_->Release();
	delete enemyManager_;

	skyDome_->Release();
	delete skyDome_;
}

// リアルシャドウ描画
void GameScene::DrawShadow(void)
{
	// シャドウマップハンドルの作成
	int ShadowMapHandle = MakeShadowMap(1024, 1024);
	// シャドウマップが想定するライトの方向もセット
	SetShadowMapLightDirection(ShadowMapHandle, { 0.3f, -0.7f, 0.8f });
	// シャドウマップに描画する範囲を設定
	SetShadowMapDrawArea(ShadowMapHandle, 
		VGet(player_->GetTransform().pos.x - 1000.0f, player_->GetTransform().pos.y - 1.0f, player_->GetTransform().pos.z -1000.0f), 
		VGet(player_->GetTransform().pos.x + 1000.0f, player_->GetTransform().pos.y + 1000.0f, player_->GetTransform().pos.z + 1000.0f));

	// シャドウマップへの描画の準備
	ShadowMap_DrawSetup(ShadowMapHandle);

	// シャドウマップへステージモデルの描画
	MV1DrawModel(stage_->GetTransform().modelId);
	// シャドウマップへキャラクターモデルの描画
	MV1DrawModel(player_->GetTransform().modelId);

	// シャドウマップへの描画を終了
	ShadowMap_DrawEnd();

	// 描画に使用するシャドウマップを設定
	SetUseShadowMap(0, ShadowMapHandle);

	// ステージモデルの描画
	MV1DrawModel(stage_->GetTransform().modelId);
	// キャラクターモデルの描画
	MV1DrawModel(player_->GetTransform().modelId);

	// 描画に使用するシャドウマップの設定を解除
	SetUseShadowMap(0, -1);

	// シャドウマップの削除
	DeleteShadowMap(ShadowMapHandle);
}
