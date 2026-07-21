#pragma once
#include "../ActorBase.h"

class StaticMeshComponent;

class Rocket : public ActorBase
{
public:

    static constexpr int QUOTA = 5000;
    ~Rocket(void) override;

    void Init(void)   override;
    void Update(void) override;
    void Draw(void)   override;

    // ”[•i
    void AddDelivery(int value);

    // —ÝŒv”[•iŠz
    int GetTotalDelivered(void) const;

    // ƒmƒ‹ƒ}ƒNƒŠƒA‚µ‚½‚©
    bool IsQuotaCleared(void) const;

    // ƒ‚ƒfƒ‹IDŽæ“¾
    int  GetModelId(void) const;

private:

    StaticMeshComponent* mesh_ = nullptr;
    int totalDelivered_ = 0;
};