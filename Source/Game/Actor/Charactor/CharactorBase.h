#pragma once
#include <map>
#include <functional>
#include <memory>
#include "../ActorBase.h"
class AnimationController;
class CapsuleComponent;

class CharactorBase : public ActorBase
{
public:

    CharactorBase(void);
    virtual ~CharactorBase(void) override;

    void Init(void)    override;
    void Update(void)  override;
    void Draw(void)    override;
    void Release(void) override;

    // 当たり判定の通知
    void OnHit(const HitResult& hit)     override;
    void OnOverlap(const HitResult& hit) override;

protected:

    // アニメーション
    AnimationController* animCtrl_ = nullptr;

    // 状態
    int stateBase_ = -1;

    // 状態遷移時の初期処理
    std::map<int, std::function<void(void)>> stateChanges_;

    // 毎フレームの更新処理
    std::function<void(void)> stateUpdate_;

    // 向き
    Vector3 faceDir_ = Vector3::FORWARD;
    Vector3 moveDir_ = Vector3::ZERO;

    // 移動
    float   moveSpeed_ = 0.0f;
    Vector3 movePow_ = Vector3::ZERO;

    // ジャンプ
    Vector3 jumpPow_ = Vector3::ZERO;
    bool    isJump_ = false;
    float   stepJump_ = 0.0f;

    // 移動前座標
    Vector3 prevPos_ = Vector3::ZERO;

    // 丸影
    int imgShadow_ = -1;

    // 定数
    static constexpr float MAX_FALL_SPEED = 5.0f;
    static constexpr int   CNT_TRY_COLLISION = 20;
    static constexpr float COLLISION_BACK_DIS = 1.0f;

    // 状態遷移
    virtual void ChangeState(int state);

    // 移動方向に応じた回転
    void Rotate(void);

    // 重力計算
    void CalcGravityPow(void);

    // 衝突判定
    virtual void CollisionReserve(void) {}
    void Collision(void);

    // 更新系
    virtual void UpdateProcess(void) = 0;
    virtual void UpdateProcessPost(void) = 0;

    // 接地しているか
    bool isGrounded_ = false;

    // ワールド（ステージ・ロケット等）からの押し戻し処理
    void PushBackFromWorld(const HitResult& hit);
};