#pragma once
#include "SceneComponent.h"
#include "../Collision/CollisionProfile.h"
#include "../Collision/CollisionProfileType.h"
#include "../Collision/HitResult.h"

class ActorBase;

// 当たり判定を持つ Component の基底
// Unreal の UPrimitiveComponent に相当
class PrimitiveComponent : public SceneComponent
{
public:

    PrimitiveComponent(ActorBase& owner);
    ~PrimitiveComponent(void) override;

    // ---------------------------------------------------------------
    // ライフサイクル
    // CollisionManager への自動登録・解除
    // ---------------------------------------------------------------

    // 初期化時に CollisionManager に自動登録
    void Init(void)    override;

    // デバッグ描画
    void Draw(void) override {};

    // 解放時に CollisionManager から自動解除
    void Release(void) override;

    // ---------------------------------------------------------------
    // 形状
    // ---------------------------------------------------------------

    // 形状の種類
    enum class SHAPE
    {
        NONE,
        CAPSULE,
        SPHERE,
        STATIC_MESH,
        LINE,
    };

    virtual SHAPE GetShape(void) const { return SHAPE::NONE; }

    // ---------------------------------------------------------------
    // コリジョンプロファイル
    // ---------------------------------------------------------------

    // プロファイルを設定する
    void SetProfile(CollisionProfileType type);

    // プロファイルを取得する
    const CollisionProfile& GetProfile(void) const;

    // チャンネルを取得する
    CollisionChannel GetChannel(void) const;

    // 有効フラグ
    bool IsValid(void) const;
    // 有効フラグを設定する
    void SetValid(bool valid);

private:

    // コリジョンプロファイル
    CollisionProfile profile_;

    // 有効フラグ
    bool isValid_ = true;
};