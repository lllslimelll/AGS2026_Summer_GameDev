#pragma once
#include <DxLib.h>
#include <random>
#include "EnemyBase.h"

class Player;

class EnemyGiant : public EnemyBase
{
public:

    // 状態
    enum class STATE
    {
        NONE,
        IDLE,       // 短い待機
        WANDER,     // ランダム方向へランダム時間だけ歩く
        CHASE,      // プレイヤー追跡
        ATTACK,     // 攻撃
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

    EnemyGiant(const EnemyBase::EnemyData& data, Player& player);
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

    // ==== 定数：外観 ====

    static constexpr float  SCALE = 3.0f;
    static constexpr VECTOR DEFAULT_LOCAL_ROT =
    { 0.0f, 180.0f * DX_PI_F / 180.0f, 0.0f };

    static constexpr VECTOR COL_LINE_START_LOCAL_POS = { 0.0f, 120.0f,  0.0f };
    static constexpr VECTOR COL_LINE_END_LOCAL_POS = { 0.0f, -10.0f,  0.0f };

    static constexpr VECTOR COL_CAPSULE_TOP_LOCAL_POS = { 0.0f, 300.0f, 0.0f };
    static constexpr VECTOR COL_CAPSULE_DOWN_LOCAL_POS = { 0.0f,  50.0f, 0.0f };
    static constexpr float  COL_CAPSULE_RADIUS = 100.0f;

    static constexpr float  ATTACK_SPHERE_RADIUS = 130.0f;
    static constexpr VECTOR ATTACK_SPHERE_OFFSET_LOCAL = { -100.0f, -150.0f, 30.0f };

    // ==== 定数：AI パラメータ ====

    static constexpr float SPEED_WANDER = 5.0f;
    static constexpr float SPEED_CHASE = 10.0f;

    static constexpr float DIST_ATTACK = 500.0f;
    static constexpr float DIST_LOSE_CHASE = 1500.0f;

    static constexpr int   ATTACK_DAMAGE = 20;
    static constexpr float ATTACK_HIT_START = 0.5f;
    static constexpr float ATTACK_HIT_END = 1.0f;

    // ==== 定数：徘徊（方向＋時間ベース）====

    // 1回の徘徊で歩く時間の範囲（秒）
    static constexpr float WANDER_TIME_MIN = 1.5f;
    static constexpr float WANDER_TIME_MAX = 4.0f;

    // 待機時間の範囲（秒）
    static constexpr float IDLE_TIME_MIN = 0.5f;
    static constexpr float IDLE_TIME_MAX = 2.0f;

    // 前方障害物チェックのレイ距離
    static constexpr float WANDER_PROBE_DIST = 250.0f;

    // 前方が塞がれた時に方向を左右に振る刻み（度）
    static constexpr float WANDER_TURN_STEP_DEG = 15.0f;

    // 左右に振る最大ステップ数（15度 × 12 = ±180度まで試す）
    static constexpr int   WANDER_TURN_MAX_STEP = 12;

    // ==== 定数：視野判定 ====

    static constexpr float VIEW_RANGE = 1200.0f;
    static constexpr float VIEW_HALF_FOV_RAD = 60.0f * DX_PI_F / 180.0f;
    static constexpr float EYE_HEIGHT = 50.0f;

    // ==== 定数：CHASE 用の障害物回避 ====

    static constexpr float AVOID_PROBE_DIST = 200.0f;
    static constexpr float AVOID_ANGLE_RAD = 45.0f * DX_PI_F / 180.0f;
    static constexpr float AVOID_ANGLE_WIDE_RAD = 90.0f * DX_PI_F / 180.0f;
    static constexpr float AVOID_COMMIT_SEC = 0.4f;

    // ==== 定数：ステアリング共通 ====

    static constexpr float WALL_PUSH_EPSILON = 0.5f;
    static constexpr float MAX_TURN_RAD_PER_SEC = 6.0f;

    static constexpr float STUCK_MOVE_MIN = 1.0f;
    static constexpr float STUCK_TIMEOUT_SEC = 1.5f;

    // ==== 状態変数：ステート機械 ====

    STATE state_;

    bool  isAware_;

    // 攻撃用
    bool  attackHit_;
    int   attackHandFrame_;

    // ==== 状態変数：徘徊 ====

    // 現在の徘徊方向（XZ 単位ベクトル）
    VECTOR wanderDir_;

    // 現在の徘徊時間の残り（秒）
    float  wanderTimer_;

    // 待機時間の残り（秒）
    float  idleTimer_;

    // インスタンス固有の乱数エンジン
    std::mt19937 rng_;

    // ==== 状態変数：ステアリング ====

    VECTOR wallNormalXZ_;      // 押し戻しから推定した壁法線
    VECTOR prevMoveDir_;       // 角速度制限用の前フレーム方向
    int    lastAvoidChoice_;   // CHASE ヒステリシス用
    float  avoidCommitTimer_;

    float  stuckTimer_;
    VECTOR stuckLastPos_;

#ifdef _DEBUG
    bool debugDrawView_ = true;
#endif

    // ==== 状態遷移 ====

    void ChangeState(STATE state);
    void ChangeStateNone(void);
    void ChangeStateIdle(void);
    void ChangeStateWander(void);
    void ChangeStateChase(void);
    void ChangeStateAttack(void);
    void ChangeStateEnd(void);

    // ==== 各ステート更新 ====

    void UpdateNone(void);
    void UpdateIdle(void);
    void UpdateWander(void);
    void UpdateChase(void);
    void UpdateAttack(void);
    void UpdateEnd(void);

    // ==== ヘルパー：徘徊 ====

    // ランダムな方向と歩行時間を設定する
    void PickNewWanderDirection(void);

    // 指定方向が前方に障害物なく通れるか（1本レイ）
    bool IsDirectionClear(const VECTOR& dir) const;

    // 現在の wanderDir_ を左右に振って空いている方向を探し設定する。
    // 見つかれば true、全滅なら false。
    bool ReorientToClearDirection(void);

    // ==== ヘルパー：乱数 ====

    float RandFloat(float minVal, float maxVal);
    float RandAngle(void); // [0, 2π)

    // ==== ヘルパー：移動と方向 ====

    void   SetMoveDirToTarget(const VECTOR& target);
    float  DistToPlayer(void) const;
    float  DistToPlayerXZ(void) const;
    bool   IsPlayerInSight(void);

    // 追跡方向の計算（レイキャスト＋ヒステリシス）
    VECTOR ComputeChaseDir(void);

    void   PushBackFromPlayer(void);
    VECTOR GetAttackSpherePos(void) const;

    // ==== ヘルパー：ステアリングフィルタ ====

    void UpdateWallNormal(void);
    void ApplyWallSlide(void);
    void LimitTurnRate(void);

    // ==== ヘルパー：スタック検知 ====

    bool IsStuck(void);
    void ResetSteering(void);

    void DrawDebugAI(void);
};