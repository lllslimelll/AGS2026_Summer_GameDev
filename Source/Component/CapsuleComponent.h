#pragma once
#include <DxLib.h>
#include "PrimitiveComponent.h"

// カプセル形状の当たり判定 Component
class CapsuleComponent : public PrimitiveComponent
{
public:

    // radius    : カプセルの半径
    // halfHeight : カプセルの半分の高さ（中心から上端まで）
    CapsuleComponent(
        ActorBase& owner,
        float radius,
        float halfHeight);

    ~CapsuleComponent(void) override;

    // ---------------------------------------------------------------
    // 形状
    // ---------------------------------------------------------------

    SHAPE GetShape(void) const override { return SHAPE::CAPSULE; }

    // ---------------------------------------------------------------
    // パラメータの取得・設定
    // ---------------------------------------------------------------

    float GetRadius(void)     const;
    float GetHalfHeight(void) const;

    void SetRadius(float radius);
    void SetHalfHeight(float halfHeight);

    // ---------------------------------------------------------------
    // ワールド座標での形状取得
    // ---------------------------------------------------------------

    // カプセル上端のワールド座標
    Vector3 GetTopPos(void)    const;

    // カプセル下端のワールド座標
    Vector3 GetBottomPos(void) const;

    // カプセル中心のワールド座標
    Vector3 GetCenter(void)    const;

    // ---------------------------------------------------------------
    // デバッグ描画
    // ---------------------------------------------------------------

    void Draw(void) override;

private:

    float radius_;
    float halfHeight_;
};