#pragma once
#include "../ActorBase.h"

class Rocket : public ActorBase
{
public:

    // 衝突判定種別
    enum class COLLIDER_TYPE
    {
        MODEL = 0,
        MAX,
    };

    static constexpr int QUOTA = 5000;
    ~Rocket(void) override;

    void Update(void) override;
    void Draw(void)   override;

    // 納品
    void AddDelivery(int value);

    // 累計納品額
    int GetTotalDelivered(void) const;

    // ノルマクリアしたか
    bool IsQuotaCleared(void) const;

    // 座標取得
    const VECTOR& GetPos(void) const;

protected:

    virtual void InitLoad(void)      override;
    virtual void InitTransform(void) override;
    virtual void InitCollider(void)  override;
    virtual void InitAnimation(void) override;
    virtual void InitPost(void)      override;

private:



    int totalDelivered_ = 0;
};