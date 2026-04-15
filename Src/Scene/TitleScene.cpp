#include <DxLib.h>
#include "../Application.h"
#include "../Utility/AsoUtility.h"
#include "../Object/Common/AnimationController.h"
#include "../Object/Actor/SkyDome.h"
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Manager/Camera.h"
#include "../Manager/ResourceManager.h"
#include "TitleScene.h"

TitleScene::TitleScene(void)
	:
	//imgTitle_(-1),
	imgPushSpace_(-1),
	//bigPlanet_(),
	//spherePlanet_(),
	player_(),
	animController_(),
	SceneBase(),
	skyDome_()
{
}

TitleScene::~TitleScene(void)
{
}

void TitleScene::Init(void)
{

	// 画像読み込み
	//imgTitle_ = resMng_.Load(ResourceManager::SRC::TITLE).handleId_;
	
	// PushSpace画像読み込み
	imgPushSpace_ = resMng_.Load(ResourceManager::SRC::PUSH_SPACE).handleId_;

	// 定点カメラ
	sceMng_.GetCamera()->ChangeMode(Camera::MODE::FIXED_POINT);

	//// メイン惑星
	//bigPlanet_.SetModel(resMng_.Load(			// 1個 = Load()  複数 = Depulicate()
	//	ResourceManager::SRC::PIT_FALL_PLANET).handleId_);
	//bigPlanet_.scl = AsoUtility::VECTOR_ONE;
	//bigPlanet_.quaRot = Quaternion::Identity();
	//bigPlanet_.quaRotLocal = Quaternion::Identity();
	//bigPlanet_.pos = AsoUtility::VECTOR_ZERO;
	//bigPlanet_.Update();

	//// 球体惑星
	//spherePlanet_.SetModel(resMng_.Load(			// 1個 = Load()  複数 = Depulicate()
	//	ResourceManager::SRC::SPHERE_PLANET).handleId_);
	//spherePlanet_.scl = { 0.7f, 0.7f, 0.7f };
	//spherePlanet_.quaRot = Quaternion::Identity();
	//// X軸を90°回転
	//player_.quaRotLocal = Quaternion::AngleAxis(AsoUtility::Deg2RadF(90.0f), AsoUtility::AXIS_X);
	//spherePlanet_.pos = { -250.0f, -100.0f, -100.0f };
	//spherePlanet_.Update();




	//// プレイヤー
	//player_.SetModel(resMng_.Load(			// 1個 = Load()  複数 = Depulicate()
	//	ResourceManager::SRC::PLAYER).handleId_);
	//player_.scl = { 0.4f, 0.4f, 0.4f };
	//player_.quaRot = Quaternion::Identity();
	//// X軸を180°回転
	//player_.quaRotLocal = Quaternion::AngleAxis(AsoUtility::Deg2RadF(90.0f), AsoUtility::AXIS_Y);
	//player_.pos = { -250.0f, -32.0f, -105.0f };
	//player_.Update();

	//// アニメーション
	//animController_ = new AnimationController(player_.modelId);
	//animController_->Add(0, 20.0f, resMng_.Load(ResourceManager::SRC::RUN).path_);
	//animController_->Play(0, true);

	// スカイドーム
	skyDome_ = new SkyDome(empty_);
	skyDome_->Init();
}

void TitleScene::Update(void)
{
	skyDome_->Update();

	//// 惑星にX軸に毎フレーム1°ずつ回転を追加
	//spherePlanet_.quaRotLocal = Quaternion::Mult(
	//	spherePlanet_.quaRotLocal,
	//	Quaternion::AngleAxis(AsoUtility::Deg2RadF(-1.0f), AsoUtility::AXIS_Z));

	//// モデル行列を更新
	//bigPlanet_.Update();
	//spherePlanet_.Update();
	//player_.Update();

	//// プレイヤーアニメーション
	//animController_->Play(0, true);

	//// アニメーション更新
	//animController_->Update();

	// シーン遷移
	auto const& ins = InputManager::GetInstance();
	if (ins.IsTrgDown(KEY_INPUT_SPACE))
	{
		sceMng_.ChangeScene(SceneManager::SCENE_ID::GAME);
	}
}

void TitleScene::Draw(void)
{
	skyDome_->Draw();

	// 惑星描画
	MV1DrawModel(bigPlanet_.modelId);

	// 球体惑星描画
	MV1DrawModel(spherePlanet_.modelId);

	// プレイヤー描画
	MV1DrawModel(player_.modelId);

	// タイトル画像描画
	DrawRotaGraph(
		Application::SCREEN_SIZE_X / 2,
		Application::SCREEN_SIZE_Y / 2 - 50,
		1.0f,
		0.0,
		imgTitle_,
		TRUE
	);

	// PushSpace描画
	DrawRotaGraph(
		Application::SCREEN_SIZE_X / 2,
		Application::SCREEN_SIZE_Y - 120,
		1.0f,
		0.0,
		imgPushSpace_,
		TRUE
	);
}

void TitleScene::Release(void)
{
	skyDome_->Release();
	delete skyDome_;

	animController_->Release();
	delete animController_;
}
