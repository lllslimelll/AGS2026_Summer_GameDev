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
        IDLE,       // ランダム点に到達した後の短い待機
        WANDER,     // wanderCenter_ 周辺のランダム点へ徘徊
        CHASE,      // プレイヤー追跡
        ATTACK,     // 攻撃（当たり判定はアニメ後半のみ）
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
    static constexpr float DIST_ARRIVE = 80.0f;

    static constexpr int   ATTACK_DAMAGE = 20;
    static constexpr float ATTACK_HIT_START = 0.5f;
    static constexpr float ATTACK_HIT_END = 1.0f;

    // ==== 定数：徘徊 ====

    static constexpr float WANDER_RADIUS = 800.0f;  // wanderCenter_ からの徘徊半径
    static constexpr float WANDER_MIN_DIST = 200.0f;  // 目標が近すぎるのを防ぐ下限
    static constexpr int   WANDER_MAX_TRIES = 16;     // 到達可能な目標を探す試行回数
    static constexpr float IDLE_TIME_MIN = 0.5f;
    static constexpr float IDLE_TIME_MAX = 2.0f;

    // ==== 定数：視野判定 ====

    static constexpr float VIEW_RANGE = 1200.0f;
    static constexpr float VIEW_HALF_FOV_RAD = 60.0f * DX_PI_F / 180.0f;
    static constexpr float EYE_HEIGHT = 50.0f;

    // ==== 定数：障害物回避 ====

    static constexpr float AVOID_PROBE_DIST = 200.0f;
    static constexpr float AVOID_ANGLE_RAD = 45.0f * DX_PI_F / 180.0f;
    static constexpr float AVOID_ANGLE_WIDE_RAD = 90.0f * DX_PI_F / 180.0f;

    // 一度選んだ回避方向を保持する秒数（フレーム間の振動防止）
    static constexpr float AVOID_COMMIT_SEC = 0.4f;

    // 押し戻し量がこの閾値以上のフレームだけ「壁に接触」と判定
    static constexpr float WALL_PUSH_EPSILON = 0.5f;

    // 1秒あたりに回れる最大角（震え防止のローパス）
    static constexpr float MAX_TURN_RAD_PER_SEC = 6.0f;

    // スタック検知
    static constexpr float STUCK_MOVE_MIN = 1.0f;  // 1フレームの実移動下限
    static constexpr float STUCK_TIMEOUT_SEC = 1.5f;  // これを超えたら次目標へ

    // ==== 状態変数：ステート機械 ====

    STATE state_;
    float idleTimer_;

    // 攻撃用
    bool  attackHit_;
    int   attackHandFrame_;

    // ==== 状態変数：徘徊 ====

    // 徘徊の「中心」。起動時は defaultPos_、見失った時は見失った位置に更新される
    VECTOR wanderCenter_;

    // 現在の徘徊目標地点
    VECTOR wanderTarget_;

    // 前回選んだ徘徊角度（フォールバック時に逆方向を試すために保持）
    float  lastWanderTheta_;

    // インスタンス固有の乱数エンジン（複数体でシードが被らないようにする）
    std::mt19937 rng_;

    // ==== 状態変数：ステアリング ====

    VECTOR wallNormalXZ_;     // 押し戻しから推定した壁の外向き法線（XZ平面）
    VECTOR prevMoveDir_;      // 角速度制限用の前フレーム方向
    int    lastAvoidChoice_;  // -1:左, 0:前, +1:右
    float  avoidCommitTimer_; // ヒステリシスタイマー

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

    // wanderCenter_ 周辺から到達可能なランダム目標を選び wanderTarget_ に格納
    void PickNextWanderTarget(void);

    // 3本のレイで通路幅を考慮した到達可能チェック
    bool IsReachable(const VECTOR& cand) const;

    // 2点間が遮蔽なく直行できるか（1本レイ）
    bool IsPathClear(const VECTOR& from, const VECTOR& to) const;

    // 徘徊中心を更新（見失った時に呼ぶ）
    void SetWanderCenter(const VECTOR& center);

    // ==== ヘルパー：乱数 ====

    float RandFloat(float minVal, float maxVal); // [minVal, maxVal] の一様乱数
    float RandAngle(void);                       // [0, 2π) の角度

    // ==== ヘルパー：移動と方向 ====

    void   SetMoveDirToTarget(const VECTOR& target);
    float  DistToPlayer(void) const;
    float  DistToPlayerXZ(void) const;
    bool   IsPlayerInSight(void);

    // 追跡方向の計算（レイキャスト＋ヒステリシス）
    VECTOR ComputeChaseDir(void);

    // 徘徊目標方向の計算（ComputeChaseDir の徘徊版）
    VECTOR ComputeWanderDir(void);

    void   PushBackFromPlayer(void);
    VECTOR GetAttackSpherePos(void) const;

    // ==== ヘルパー：ステアリングフィルタ ====

    // 押し戻し後の位置から壁法線を推定して wallNormalXZ_ に格納
    void UpdateWallNormal(void);

    // moveDir_ の壁食い込み成分を射影で除去：v_slide = v - (v・n)n
    void ApplyWallSlide(void);

    // moveDir_ の1フレーム回転量を MAX_TURN_RAD_PER_SEC で制限
    void LimitTurnRate(void);

    // ==== ヘルパー：スタック検知 ====

    bool IsStuck(void);
    void ResetSteering(void);

    // デバッグ描画
    void DrawDebugAI(void);
};