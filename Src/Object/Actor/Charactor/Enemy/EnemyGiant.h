#pragma once
#include <DxLib.h>
#include "EnemyBase.h"

class Player;

class EnemyGiant : public EnemyBase
{
public:

    // 状態
    enum class STATE
    {
        NONE,
        THINK,
        IDLE,
        PATROL,
        CHASE,
        ATTACK,
        RETURN,
        END,
    };

    // アニメーション種別
    enum class ANIM_TYPE
    {
        IDLE = 0,
        WALK = 1,
        RUN = 2,
        ATTACK = 3,
    };

    // コンストラクタ
    EnemyGiant(const EnemyBase::EnemyData& data, Player& player);

    // デストラクタ
    ~EnemyGiant(void) override;

protected:

    void InitLoad(void)      override;
    void InitTransform(void) override;
    void InitCollider(void)  override;
    void InitAnimation(void) override;
    void InitPost(void)      override;

    void UpdateProcess(void)     override;
    void UpdateProcessPost(void) override;

    void Draw(void) override;

private:
    int attackHandFrame_ = 0;
    // スケール
    static constexpr float SCALE = 1.0f;

    // モデルローカル回転
    static constexpr VECTOR DEFAULT_LOCAL_ROT =
    { 0.0f, 180.0f * DX_PI_F / 180.0f, 0.0f };

    // 地面衝突用線分
    static constexpr VECTOR COL_LINE_START_LOCAL_POS = { 0.0f, 120.0f,  0.0f };
    static constexpr VECTOR COL_LINE_END_LOCAL_POS = { 0.0f, -10.0f,  0.0f };

    // カプセルコライダ
    static constexpr VECTOR COL_CAPSULE_TOP_LOCAL_POS = { 0.0f, 160.0f, 0.0f };
    static constexpr VECTOR COL_CAPSULE_DOWN_LOCAL_POS = { 0.0f,  50.0f, 0.0f };
    static constexpr float  COL_CAPSULE_RADIUS = 40.0f;

    // 視野コライダ（視野モデル）用
    static constexpr VECTOR VIEW_RANGE_SCL = { 10.0f, 5.0f, 3.0f };
    static constexpr float  VIEW_RANGE_ROT_X = 26.0f * DX_PI_F / 180.0f;
    static constexpr float  VIEW_RANGE_LOCAL_ROT_X = 90.0f * DX_PI_F / 180.0f;
    static constexpr int    VIEW_RANGE_SYNC_FRAME = 6;  // 頭部フレーム番号

    // 攻撃用球体コライダ（手フレームに追従）
    static constexpr float ATTACK_SPHERE_RADIUS = 20.0f;
 

    // AI距離パラメータ
    static constexpr float SPEED_PATROL = 1.0f;
    static constexpr float SPEED_CHASE = 3.0f;
    static constexpr float DIST_ATTACK = 100.0f;  // 攻撃距離
    static constexpr float DIST_CHASE = 500.0f;  // チェイス解除距離
    static constexpr float ATTACK_DAMAGE = 20;
    VECTOR spawnPos_ = AsoUtility::VECTOR_ZERO;
    // 状態
    STATE state_;

    // 更新ステップ（タイマー）
    float step_;

    // 視野用トランスフォーム
    Transform viewRangeTransform_;

    // 攻撃ヒット済みフラグ（1回の攻撃で1回だけダメージ）
    bool attackHit_;

    // 状態遷移
    void ChangeState(STATE state);
    void ChangeStateNone(void);
    void ChangeStateThink(void);
    void ChangeStateIdle(void);
    void ChangeStatePatrol(void);
    void ChangeStateChase(void);
    void ChangeStateAttack(void);
    void ChangeStateReturn(void);
    void ChangeStateEnd(void);

    // 更新系
    void UpdateNone(void);
    void UpdateThink(void);
    void UpdateIdle(void);
    void UpdatePatrol(void);
    void UpdateChase(void);
    void UpdateAttack(void);
    void UpdateReturn(void);
    void UpdateEnd(void);

    // 巡回方向設定
    void SetMoveDirToTarget(const VECTOR& target);

    // 索敵（視野モデル）
    bool InSearchConeModel(void);

    // プレイヤーとの距離
    float DistToPlayer(void) const;

    // 押し戻し処理（敵とプレイヤーのカプセル同士）
    void PushBackFromPlayer(void);
};