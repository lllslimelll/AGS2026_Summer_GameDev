#include "Planet.h"
#include "Rocket.h"
#include "StageManager.h"

StageManager::StageManager(Planet* planet, Rocket* rocket)
    : ActorBase()
    , planet_(planet)
    , rocket_(rocket)
{
}

StageManager::~StageManager(void)
{
}

Rocket& StageManager::GetRocket(void)
{
    return *rocket_;
}

Planet& StageManager::GetPlanet(void)
{
    return *planet_;
}