#pragma once
#include <memory>

class Planet;
class Rocket;

class StageManager
{
public:

    StageManager(void);
    ~StageManager(void);

    void Init(void);
    void Update(void);
    void Draw(void);
    void Release(void);

    Rocket& GetRocket(void);
    Planet& GetPlanet(void);

private:

    std::unique_ptr<Planet> planet_;
    std::unique_ptr<Rocket> rocket_;
};