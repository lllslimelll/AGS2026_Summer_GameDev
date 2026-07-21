#pragma once
#include "../ActorBase.h"

class Planet;
class Rocket;

class StageManager : public ActorBase
{
public:

    StageManager(Planet* planet, Rocket* rocket);
    ~StageManager(void) override;

    void Init(void)    override {}
    void Update(void)  override {}
    void Draw(void)    override {}
    void Release(void) override {}

    Rocket& GetRocket(void);
    Planet& GetPlanet(void);

private:

    Planet* planet_;
    Rocket* rocket_;
};