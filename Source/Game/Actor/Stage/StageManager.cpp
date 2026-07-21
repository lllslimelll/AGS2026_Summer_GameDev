#include "Planet.h"
#include "Rocket.h"
#include "StageManager.h"

StageManager::StageManager(void)
{
}

StageManager::~StageManager(void) {}

void StageManager::Init(void)
{
    planet_ = std::make_unique<Planet>();
    planet_->Init();
    rocket_ = std::make_unique<Rocket>();
    rocket_->Init();
}

void StageManager::Update(void)
{
    planet_->Update();
    rocket_->Update();
}

void StageManager::Draw(void)
{
    planet_->Draw();
    rocket_->Draw();
}

void StageManager::Release(void)
{
    if (planet_) planet_->Release();
    if (rocket_) rocket_->Release();
}

Rocket& StageManager::GetRocket(void) { return *rocket_; }
Planet& StageManager::GetPlanet(void) { return *planet_; }