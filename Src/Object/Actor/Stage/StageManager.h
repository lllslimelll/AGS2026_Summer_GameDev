#pragma once
#include <memory>
#include <DxLib.h>

class Planet;
class Rocket;
class ColliderBase;

class StageManager
{
public:

    StageManager(void);
    ~StageManager(void);

    void Init(void);
    void Update(void);
    void Draw(void);
    void DrawUI(void);
    void Release(void);

    // ロケット取得
    Rocket& GetRocket(void);

    // 惑星取得
    Planet& GetPlanet(void);

    // Transform取得（シャドウマップ用）
    const Transform& GetTransform(void) const;

private:

    std::unique_ptr<Planet> planet_;
    std::unique_ptr<Rocket> rocket_;
};