#pragma once
#include <vector>
#include <memory>
#include <DxLib.h>
#include "HitResult.h"
#include "CollisionChannel.h"

class ColliderBase;
class ActorBase;

// 当たり判定を一元管理するクラス
// Unreal の FPhysScene に相当
class CollisionManager
{
public:

    static void             CreateInstance(void);
    static CollisionManager& GetInstance(void);
    void Destroy(void);

    // シーン切り替え時に全登録をクリアする
    void Clear(void);

    // コライダーを登録する
    void Register(ColliderBase* collider);

    // コライダーの登録を解除する
    void Unregister(ColliderBase* collider);

    // 毎フレーム全ての当たり判定を実行する
    void Update(void);

    // Actor の HitResult を全て取得する
    std::vector<HitResult> GetHits(const ActorBase* actor) const;

    // BLOCK のみ取得（押し戻し用）
    std::vector<HitResult> GetBlockHits(const ActorBase* actor) const;

    // OVERLAP のみ取得（トリガー・ダメージ判定用）
    std::vector<HitResult> GetOverlapHits(const ActorBase* actor) const;

private:

    CollisionManager(void) = default;
    ~CollisionManager(void) = default;

    static CollisionManager* instance_;

    // 登録されたコライダー一覧
    std::vector<ColliderBase*> colliders_;

    // 毎フレームの判定結果（Update でクリア→生成）
    std::vector<HitResult> hitResults_;

    // 2つのコライダーの判定を実行して HitResult を生成する
    void Solve(ColliderBase* a, ColliderBase* b);

    // Capsule vs Model の判定
    void SolveCapsuleModel(ColliderBase* capsule, ColliderBase* model);

    // Sphere vs Sphere の判定
    void SolveSphereVsSphere(ColliderBase* a, ColliderBase* b);

    // Capsule vs Capsule の判定
    void SolveCapsuleVsCapsule(ColliderBase* a, ColliderBase* b);

    // 優先度方式でレスポンスを決定する
    // BLOCK > OVERLAP > IGNORE
    CollisionResponse ResolveResponse(
        const ColliderBase* a,
        const ColliderBase* b) const;
};