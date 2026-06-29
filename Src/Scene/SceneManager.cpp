#include <chrono>
#include <DxLib.h>
#include <EffekseerForDXLib.h>
#include "../Common/Fader.h"
#include "TitleScene.h"
#include "GameScene.h"
#include "PauseScene.h"
#include "ResultScene.h"
#include "DebugScene.h"
#include "../Manager/Camera.h"
#include "../Manager/ResourceManager.h"
#include "SceneManager.h"
#include "../Manager/SoundManager.h"

SceneManager* SceneManager::instance_ = nullptr;

void SceneManager::CreateInstance()
{
	if (instance_ == nullptr)
	{
		instance_ = new SceneManager();
	}
	instance_->Init();
}

SceneManager& SceneManager::GetInstance(void)
{
	return *instance_;
}

void SceneManager::Init(void)
{

	sceneId_ = SCENE_ID::TITLE;
	waitSceneId_ = SCENE_ID::NONE;

	// フェード機能の初期化
	fader_ = new Fader();
	fader_->Init();

	// カメラ
	camera_ = new Camera();
	camera_->Init();

	// 画面遷移中判定
	isSceneChanging_ = false;

	// デルタタイム
	preTime_ = std::chrono::system_clock::now();

	// 3D用の設定
	Init3D();

	// 初期シーンの設定
	DoChangeScene(SCENE_ID::TITLE);

}

void SceneManager::Init3D(void)
{

	// 背景色設定
	SetBackgroundColor(
		BACKGROUND_COLOR_R, 
		BACKGROUND_COLOR_G, 
		BACKGROUND_COLOR_B);

	// Zバッファを有効にする
	SetUseZBuffer3D(true);

	// Zバッファへの書き込みを有効にする
	SetWriteZBuffer3D(true);

	// バックカリングを有効にする
	SetUseBackCulling(true);

	// ライトの設定
	SetUseLighting(true);
	
	// ライトの設定
	//ChangeLightTypeDir({ 0.3f, -0.7f, 0.8f });

	// フォグ設定
	SetFogEnable(false);
	SetFogColor(5, 5, 5);
	SetFogStartEnd(10000.0f, 20000.0f);

	// マウスカーソルを非表示にする
	SetMouseDispFlag(FALSE);
}

void SceneManager::Update(void)
{

	if (scenes_.empty()) { return; }

	// デルタタイム
	auto nowTime = std::chrono::system_clock::now();
	deltaTime_ = static_cast<float>(
		std::chrono::duration_cast<std::chrono::nanoseconds>(nowTime - preTime_).count() / 1000000000.0);
	preTime_ = nowTime;

	// フェード機能の更新
	fader_->Update();
	if (isSceneChanging_)
	{
		// フェード状態の切替処理
		Fade();
	}
	else
	{
		// スタック末尾のみ更新
		scenes_.back()->Update();
	}

	// カメラ更新
	camera_->Update();

}

void SceneManager::Draw(void)
{
	
	// 描画先グラフィック領域の指定
	// (３Ｄ描画で使用するカメラの設定などがリセットされる)
	SetDrawScreen(DX_SCREEN_BACK);

	// 画面を初期化
	ClearDrawScreen();

	// カメラ設定
	camera_->SetBeforeDraw();

	// Effekseerにより再生中のエフェクトを更新する。
	UpdateEffekseer3D();

	// スタック内のシーンを描画
	for (auto& scene : scenes_)
	{
		scene->Draw();
	}

	// カメラ描画
	camera_->DrawDebug();

	// Effekseerにより再生中のエフェクトを描画する。
	DrawEffekseer3D();
	
	// 暗転・明転
	fader_->Draw();

}

void SceneManager::Destroy(void)
{
	// シーンリストの破棄
	scenes_.clear();

	// フェード機能の解放
	delete fader_;

	camera_->Release();
	delete camera_;


	// インスタンスのメモリ解放
	delete instance_;

}

// シーン遷移
void SceneManager::ChangeScene(SCENE_ID nextId)
{

	// フェード処理が終わってからシーンを変える場合もあるため、
	// 遷移先シーンをメンバ変数に保持
	waitSceneId_ = nextId;

	// フェードアウト(暗転)を開始する
	fader_->SetFade(Fader::STATE::FADE_OUT);
	isSceneChanging_ = true;

}

// シーン遷移実行
void SceneManager::DoChangeScene(SCENE_ID sceneId)
{

	// リソースの解放
	ResourceManager::GetInstance().Release();

	// シーンを変更する
	sceneId_ = sceneId;

	// シーンリストの破棄
	scenes_.clear();

	// シーンの生成
	switch (sceneId_)
	{
	case SCENE_ID::TITLE:
		scenes_.push_back(std::make_unique<TitleScene>());
		SetMouseDispFlag(true);
		SoundManager::GetInstance().StopWalk();
		break;
	case SCENE_ID::GAME:
		scenes_.push_back(std::make_unique<GameScene>());
		SoundManager::GetInstance().StopWalk();
		SetMouseDispFlag(false);
		break;
	case SCENE_ID::DEBUG:
		scenes_.push_back(std::make_unique<DebugScene>());
		break;
	}

	// シーン末尾の初期化
	scenes_.back()->Init();

	// デルタタイムリセット
	ResetDeltaTime();

	waitSceneId_ = SCENE_ID::NONE;
}

void SceneManager::PushScene(SCENE_ID sceneId)
{ 
	// シーンの変更
	sceneId_ = sceneId;

	// シーンの生成
	switch (sceneId_)
	{
	case SCENE_ID::PAUSE:
		scenes_.push_back(std::make_unique<PauseScene>());
		break;
	case SCENE_ID::RESULT:
		scenes_.push_back(std::make_unique<ResultScene>());
		SoundManager::GetInstance().StopWalk();
		SetMouseDispFlag(true);
		break;
	default:
		break;
	}

	// シーン末尾の初期化
	scenes_.back()->Init();
}



void SceneManager::PopScene()
{
	if (scenes_.size() > 1)
	{
		scenes_.pop_back();
	}
}


SceneManager::SCENE_ID SceneManager::GetSceneID(void)
{
	return sceneId_;
}

float SceneManager::GetDeltaTime(void) const
{
	return 1.0f / 60.0f;
	//return deltaTime_;
}

Camera* SceneManager::GetCamera(void) const
{
	return camera_;
}

SceneManager::SceneManager(void)
{

	sceneId_ = SCENE_ID::NONE;
	waitSceneId_ = SCENE_ID::NONE;

	fader_ = nullptr;

	isSceneChanging_ = false;

	// デルタタイム
	deltaTime_ = 1.0f / 60.0f;

	camera_ = nullptr;

}

void SceneManager::ResetDeltaTime(void)
{
	deltaTime_ = 0.016f;
	preTime_ = std::chrono::system_clock::now();
}

void SceneManager::Fade(void)
{

	Fader::STATE fState = fader_->GetState();
	switch (fState)
	{
	case Fader::STATE::FADE_IN:
		// 明転中
		if (fader_->IsEnd())
		{
			// 明転が終了したら、フェード処理終了
			fader_->SetFade(Fader::STATE::NONE);
			isSceneChanging_ = false;
		}
		break;
	case Fader::STATE::FADE_OUT:
		// 暗転中
		if (fader_->IsEnd())
		{
			// 完全に暗転してからシーン遷移
			DoChangeScene(waitSceneId_);
			// 暗転から明転へ
			fader_->SetFade(Fader::STATE::FADE_IN);
		}
		break;
	}

}

