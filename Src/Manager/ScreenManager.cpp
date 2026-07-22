#include <DxLib.h>
#include "../Application.h"
#include "ScreenManager.h"

ScreenManager* ScreenManager::instance_ = nullptr;

void ScreenManager::CreateInstance(void)
{
	if (instance_ == nullptr)
	{
		instance_ = new ScreenManager();
	}

	instance_->Init();
}

ScreenManager& ScreenManager::GetInstance(void)
{
	if (instance_ == nullptr)
	{
		ScreenManager::CreateInstance();
	}

	return *instance_;
}

void ScreenManager::Init()
{
	// スクリーン作成
	// ------------------------------
	// メインスクリーン
	mainScreen_ = MakeScreen(
		Application::SCREEN_SIZE_X,
		Application::SCREEN_SIZE_Y,
		true);

	// ピンポンバッファ用
	pingPongScreen_[0] = MakeScreen(
		Application::SCREEN_SIZE_X,
		Application::SCREEN_SIZE_Y,
		true);

	pingPongScreen_[1] = MakeScreen(
		Application::SCREEN_SIZE_X,
		Application::SCREEN_SIZE_Y,
		true);

	// UIスクリーン(仮)
	guideUIScreen_ = MakeScreen(
	 Application::SCREEN_SIZE_X,
	 Application::SCREEN_SIZE_Y,
	 true);
}

void ScreenManager::Destroy()
{
	// スクリーンの破棄
	DeleteGraph(mainScreen_);
	DeleteGraph(guideUIScreen_);
	DeleteGraph(pingPongScreen_[0]);
	DeleteGraph(pingPongScreen_[1]);

	// インスタンス破棄
	delete instance_;
}

int ScreenManager::GetMainScreen() const
{
	return mainScreen_;
}

int ScreenManager::GetGuideUIScreen() const
{
	return guideUIScreen_;
}

int ScreenManager::GetPingPongScreen(int index) const
{
	return pingPongScreen_[index];
}

ScreenManager::ScreenManager()
{
}
