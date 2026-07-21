#pragma once
#include <vector>
#include "../Core/Vector3.h"
#include "../Core/Quaternion.h"
#include "../Core/Matrix4x4.h"
#include "ActorComponent.h"

class ActorBase;

class SceneComponent : public ActorComponent
{
public:

    // コンストラクタ（オーナーを必ず渡す）
    SceneComponent(ActorBase& owner);
    ~SceneComponent(void) override;

    // ---------------------------------------------------------------
    // 親子関係
    // ---------------------------------------------------------------

    // 親コンポーネントにアタッチする
    void AttachTo(SceneComponent* parent);

    // 親からデタッチする
    void Detach(void);

    // 親を取得する
    SceneComponent* GetParent(void) const;

    // 子コンポーネントリストを取得する
    const std::vector<SceneComponent*>& GetChildren(void) const;

    // ---------------------------------------------------------------
    // ローカル Transform の設定
    // 親からの相対的な位置・回転・スケール
    // ---------------------------------------------------------------

    void SetLocalPos(const Vector3& pos);
    void SetLocalRot (const Quaternion& rot);
    void SetLocalScl(const Vector3& scl);

    // ---------------------------------------------------------------
    // ローカル Transform の取得
    // ---------------------------------------------------------------

    const Vector3& GetLocalPos(void) const;
    const Quaternion& GetLocalRot(void) const;
    const Vector3& GetLocalScl(void)    const;

    // ---------------------------------------------------------------
    // ワールド Transform の取得
    // 親子関係を考慮した最終的な位置・回転・スケール
    // ---------------------------------------------------------------

    // ワールド座標を取得する
    Vector3    GetWorldPos(void) const;

    // ワールド回転を取得する
    Quaternion GetWorldRot(void) const;

    // ワールドスケールを取得する
    Vector3    GetWorldScl(void)    const;

    // ワールド Transform 行列を取得する
    Matrix4x4  GetWorldMat(void)   const;

    // ---------------------------------------------------------------
    // ワールド Transform の設定
    // 親子関係を考慮して逆算してローカルに設定する
    // ---------------------------------------------------------------

    void SetWorldPos(const Vector3& pos);
    void SetWorldRot(const Quaternion& rot);

    // ---------------------------------------------------------------
    // 方向ベクトルの取得（ワールド空間）
    // ---------------------------------------------------------------

    Vector3 GetForward(void) const;
    Vector3 GetBack(void)    const;
    Vector3 GetUp(void)      const;
    Vector3 GetDown(void)    const;
    Vector3 GetLeft(void)    const;
    Vector3 GetRight(void)   const;

private:

    // ローカル Transform（親からの相対）
    Vector3    localPos_ = Vector3::ZERO;
    Quaternion localRot_;
    Vector3    localScl_ = Vector3::ONE;

    // 親子関係
    SceneComponent* parent_ = nullptr;
    std::vector<SceneComponent*> children_;
};

