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

void ScreenManager::Init(void)
{
    mainScreen_ = MakeScreen(
        Application::SCREEN_SIZE_X,
        Application::SCREEN_SIZE_Y,
        true);

    pingPongScreen_[0] = MakeScreen(
        Application::SCREEN_SIZE_X,
        Application::SCREEN_SIZE_Y,
        true);

    pingPongScreen_[1] = MakeScreen(
        Application::SCREEN_SIZE_X,
        Application::SCREEN_SIZE_Y,
        true);
}

void ScreenManager::Destroy(void)
{
    DeleteGraph(mainScreen_);
    DeleteGraph(pingPongScreen_[0]);
    DeleteGraph(pingPongScreen_[1]);

    delete instance_;
    instance_ = nullptr;
}

int ScreenManager::GetMainScreen(void) const
{
    return mainScreen_;
}

int ScreenManager::GetPingPongScreen(int index) const
{
    return pingPongScreen_[index];
}

ScreenManager::ScreenManager(void)
    : mainScreen_(-1), pingPongScreen_{ -1, -1 }
{
}