#pragma once
#include "PrimitiveComponent.h"

// 球形状の当たり判定 Component
class SphereComponent : public PrimitiveComponent
{
public:

    // radius : 球の半径
    SphereComponent(
        ActorBase& owner,
        float radius);

    ~SphereComponent(void) override;

    // ---------------------------------------------------------------
    // 形状
    // ---------------------------------------------------------------

    SHAPE GetShape(void) const override { return SHAPE::SPHERE; }

    // ---------------------------------------------------------------
    // パラメータの取得・設定
    // ---------------------------------------------------------------

    float GetRadius(void)      const;
    void  SetRadius(float radius);

    // ---------------------------------------------------------------
    // ワールド座標での形状取得
    // ---------------------------------------------------------------

    // 球の中心のワールド座標
    Vector3 GetCenter(void) const;

    // ---------------------------------------------------------------
    // デバッグ描画
    // ---------------------------------------------------------------

    void Draw(void) override;

private:

    float radius_;
};