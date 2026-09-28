#include <DxLib.h>
#include <EffekseerForDXLib.h>
#include "Manager/InputManager.h"
#include "Manager/SettingsManager.h"
#include "Manager/ResourceManager.h"
#include "Scene/SceneManager.h"
#include "Manager/SoundManager.h"
#include "Manager/ScreenManager.h"
#include "Common/FpsController.h"
#include "Application.h"

Application* Application::instance_ = nullptr;

const std::string Application::PATH_CSV = "Data/Csv/";
const std::string Application::PATH_EFFECT = "Data/Effect/";
const std::string Application::PATH_IMAGE = "Data/Image/";
const std::string Application::PATH_MODEL = "Data/Model/";
const std::string Application::PATH_SHADER = "Data/Shader/";
const std::string Application::PATH_SOUND = "Data/Sound/";

// インスタンス生成
void Application::CreateInstance(void)
{
	if (instance_ == nullptr)
	{
		instance_ = new Application();
	}

	instance_->Init();
}

// インスタンス取得
Application& Application::GetInstance(void)
{
	return *instance_;
}

void Application::Init(void)
{
	// アプリケーションの初期設定
	SetWindowText("惑星探査");

	// ウィンドウサイズ・色深度設定
	SetGraphMode(SCREEN_SIZE_X, SCREEN_SIZE_Y, SCREEN_COLOR_DEPTH);
	// ウィンドウモード
	ChangeWindowMode(true);

	// FPS制御初期化
	fpsController_ = new FpsController(FRAME_RATE);

	// DxLibの初期化
	SetUseDirect3DVersion(DX_DIRECT3D_11);
	isInitFail_ = false;
	if (DxLib_Init() == -1)
	{
		isInitFail_ = true;
		return;
	}

	// Effekseerの初期化
	InitEffekseer();

	// 乱数のシード値を設定する
	DATEDATA date;

	// 現在時刻を取得する
	GetDateTime(&date);

	// 乱数の初期値を設定する
	// 設定する数値によって、ランダムの出方が変わる
	SRand(date.Year + date.Mon + date.Day + date.Hour + date.Min + date.Sec);

	// 入力制御初期化
	SetUseDirectInputFlag(true);
	InputManager::CreateInstance();

	// リソース管理初期化
	ResourceManager::CreateInstance();

	// スクリーン管理初期化
	ScreenManager::GetInstance().Init();

	// サウンド管理初期化
	SoundManager::CreateInstance();

	// 設定管理初期化
	SettingsManager::CreateInstance();

	// シーン管理初期化
	SceneManager::CreateInstance();
}

void Application::Run(void)
{
	// インスタンス取得
	InputManager& inputManager = InputManager::GetInstance();
	SceneManager& sceneManager = SceneManager::GetInstance();

	// ゲームループ
	while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_0) == 0)
	{
		// 更新
		inputManager.Update();
		sceneManager.Update();

		// 描画
		sceneManager.Draw();

#ifdef _DEBUG

		// 平均FPS描画
		fpsController_->Draw();

#endif // _DEBUG

		// 裏画面と表画面の内容を交換する
		ScreenFlip();

		// 理想FPS経過待ち
		fpsController_->Wait();
	}
}

void Application::Destroy(void)
{
	// 各種マネージャークラスの解放
	InputManager::GetInstance().Destroy();
	ResourceManager::GetInstance().Destroy();
	SceneManager::GetInstance().Destroy();
	SoundManager::GetInstance().Destroy();
	ScreenManager::GetInstance().Destroy();

	// FPS制御クラスの解放
	delete fpsController_;

	// Effekseerを終了
	Effkseer_End();

	// DxLibを終了
	if (DxLib_End() == -1)
	{
		isReleaseFail_ = true;
	}

	// インスタンスの解放
	delete instance_;
}

bool Application::IsInitFail(void) const
{
	return isInitFail_;
}

bool Application::IsReleaseFail(void) const
{
	return isReleaseFail_;
}

Application::Application(void)
	:
	fpsController_(nullptr),
	isInitFail_(false),
	isReleaseFail_(false)
{
}

void Application::InitEffekseer(void)
{
	if (Effekseer_Init(8000) == -1)
	{
		DxLib_End();
	}

	SetChangeScreenModeGraphicsSystemResetFlag(FALSE);

	Effekseer_SetGraphicsDeviceLostCallbackFunctions();
}
