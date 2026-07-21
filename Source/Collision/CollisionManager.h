// Collision/CollisionManager.h
#pragma once
#include <vector>
#include <DxLib.h>
#include "HitResult.h"
#include "CollisionChannel.h"
#include "CollisionQueryParams.h"
class PrimitiveComponent;
class ActorBase;

class CollisionManager
{
public:

    static void              CreateInstance(void);
    static CollisionManager& GetInstance(void);
    void Destroy(void);

    // シーン切り替え時に全登録をクリア
    void Clear(void);

    // PrimitiveComponent を登録する
    void Register(PrimitiveComponent* component);

    // PrimitiveComponent の登録を解除する
    void Unregister(PrimitiveComponent* component);

    // 毎フレーム全ての当たり判定を実行する
    // 結果を OnHit / OnOverlap で通知する
    void Update(void);

    // ---------------------------------------------------------------
    // レイキャスト
    // ---------------------------------------------------------------
    HitResult LineTrace(
        const VECTOR& start,
        const VECTOR& end,
        CollisionChannel channel,
        const CollisionQueryParams& params = CollisionQueryParams()) const;

    // ---------------------------------------------------------------
    // スウィープ（球）
    // ---------------------------------------------------------------
    HitResult SweepSphere(
        const VECTOR& start,
        const VECTOR& end,
        float radius,
        CollisionChannel channel,
        const CollisionQueryParams& params = CollisionQueryParams()) const;

private:

    CollisionManager(void) = default;
    ~CollisionManager(void) = default;

    static CollisionManager* instance_;

    // 登録された PrimitiveComponent 一覧
    std::vector<PrimitiveComponent*> components_;

    // 毎フレームの判定結果
    std::vector<HitResult> hitResults_;

    // 2つの Component の判定を実行する
    void Solve(PrimitiveComponent* a, PrimitiveComponent* b);

    // Capsule vs StaticMesh の判定
    void SolveCapsuleStaticMesh(
        PrimitiveComponent* capsule,
        PrimitiveComponent* mesh);

    // Sphere vs Sphere の判定
    void SolveSphereVsSphere(
        PrimitiveComponent* a,
        PrimitiveComponent* b);

    // Capsule vs Capsule の判定
    void SolveCapsuleVsCapsule(
        PrimitiveComponent* a,
        PrimitiveComponent* b);

    // 優先度方式でレスポンスを決定する
    CollisionResponse ResolveResponse(
        const PrimitiveComponent* a,
        const PrimitiveComponent* b) const;

    // LineTrace / SweepSphere の対象か
    bool IsTargetChannel(
        const PrimitiveComponent* component,
        CollisionChannel channel,
        const CollisionQueryParams& params) const;

    // HitResult を生成して OnHit / OnOverlap に通知する
    void NotifyHit(
        PrimitiveComponent* a,
        PrimitiveComponent* b,
        const HitResult& hit);
};