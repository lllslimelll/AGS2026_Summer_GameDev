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
        THINK,      // 遷移判断用（互換保持）
        IDLE,       // パトロールの角で待機（1秒）
        PATROL,     // 次の角へ向かって移動
        CHASE,      // プレイヤーを追跡（障害物回避あり）
        ATTACK,     // 攻撃中（当たり判定はアニメ後半のみ）
        RETURN,     // 初期位置へ帰還
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

    // ==== 定数 ====

    // モデルのスケールと回転
    static constexpr float SCALE = 3.0f;
    static constexpr VECTOR DEFAULT_LOCAL_ROT =
    { 0.0f, 180.0f * DX_PI_F / 180.0f, 0.0f };

    // 地面判定用の線分コライダ
    static constexpr VECTOR COL_LINE_START_LOCAL_POS = { 0.0f, 120.0f,  0.0f };
    static constexpr VECTOR COL_LINE_END_LOCAL_POS = { 0.0f, -10.0f,  0.0f };

    // 本体のカプセルコライダ
    static constexpr VECTOR COL_CAPSULE_TOP_LOCAL_POS = { 0.0f, 300.0f, 0.0f };
    static constexpr VECTOR COL_CAPSULE_DOWN_LOCAL_POS = { 0.0f,  50.0f, 0.0f };
    static constexpr float  COL_CAPSULE_RADIUS = 100.0f;

    // 攻撃用の球（左手フレームに追従）
    static constexpr float ATTACK_SPHERE_RADIUS = 130.0f;

    // 攻撃球のフレーム位置からのローカルオフセット
    // x: 敵の左右方向（負 = 左）
    // y: ワールドY方向（負 = 下）
    // z: 敵の前後方向（正 = 前）
    static constexpr VECTOR ATTACK_SPHERE_OFFSET_LOCAL = { -100.0f, -150.0f, 30.0f };

    // ==== AI パラメータ ====

    // 移動速度
    static constexpr float SPEED_PATROL = 5.0f;
    static constexpr float SPEED_CHASE = 10.0f;
    static constexpr float SPEED_RETURN = 5.0f;

    // 各種距離
    static constexpr float DIST_ATTACK = 500.0f;   // 攻撃に入る距離
    static constexpr float DIST_LOSE_CHASE = 2000.0f;   // 追跡を諦めて帰る距離
    static constexpr float DIST_ARRIVE = 40.0f;   // 目的地到達とみなす距離

    // ダメージ
    static constexpr int   ATTACK_DAMAGE = 20;
    // 攻撃当たり判定の有効区間（アニメ進捗 0.0 ? 1.0）
    static constexpr float ATTACK_HIT_START = 0.5f;
    static constexpr float ATTACK_HIT_END = 1.0f;

    // パトロール
    static constexpr float PATROL_SIDE = 1000.0f;              // 正方形の一辺
    static constexpr float PATROL_HALF = PATROL_SIDE * 0.5f;   // 半辺
    static constexpr float IDLE_AT_CORNER_TIME = 1.0f;         // 角での待機秒数

    // 視野判定（ロジックベースの円錐視野）
    static constexpr float VIEW_RANGE = 800.0f;
    static constexpr float VIEW_HALF_FOV_RAD = 60.0f * DX_PI_F / 180.0f; // 合計120度
    static constexpr float EYE_HEIGHT = 50.0f;   // 視線レイの高さオフセット

    // 追跡時の障害物回避
    static constexpr float AVOID_PROBE_DIST = 200.0f;
    static constexpr float AVOID_ANGLE_RAD = 45.0f * DX_PI_F / 180.0f;
    static constexpr float AVOID_ANGLE_WIDE_RAD = 90.0f * DX_PI_F / 180.0f;

    // ==== 状態変数 ====
    STATE state_;
    float step_;             // 各ステートで使う汎用タイマー

    // パトロール用
    int   patrolCornerIdx_;  // 0..3 の角インデックス（時計回り）
    float idleTimer_;        // 角での待機タイマー

    // 攻撃用
    bool  attackHit_;        // 現在の一振りで既にヒット済みか

    // キャッシュ済みボーンフレーム
    int   attackHandFrame_;

#ifdef _DEBUG
    bool  debugDrawView_ = true; // デバッグ視野表示のトグル
#endif

    // ==== 状態遷移 ====
    void ChangeState(STATE state);
    void ChangeStateNone(void);
    void ChangeStateThink(void);
    void ChangeStateIdle(void);
    void ChangeStatePatrol(void);
    void ChangeStateChase(void);
    void ChangeStateAttack(void);
    void ChangeStateReturn(void);
    void ChangeStateEnd(void);

    // ==== 各ステート更新 ====
    void UpdateNone(void);
    void UpdateThink(void);
    void UpdateIdle(void);
    void UpdatePatrol(void);
    void UpdateChase(void);
    void UpdateAttack(void);
    void UpdateReturn(void);
    void UpdateEnd(void);

    // ==== ヘルパー ====

    // faceDir_ / moveDir_ をターゲット方向に向ける（XZ平面）
    void SetMoveDirToTarget(const VECTOR& target);

    // プレイヤーとの距離（3D）
    float DistToPlayer(void) const;

    // プレイヤーとの距離（XZ平面のみ）
    float DistToPlayerXZ(void) const;

    // 視野判定：距離チェック＋FOV円錐＋LOSレイキャスト
    bool IsPlayerInSight(void);

    // パトロールの角
    VECTOR GetPatrolCornerPos(int index) const; // インデックス (0..3) のワールド座標
    VECTOR GetCurrentPatrolTarget(void) const;  // 現在の目標コーナー
    void   AdvancePatrolCorner(void);           // 時計回りで次の角へ

    // 障害物回避を含めた追跡方向を計算
    VECTOR ComputeChaseDir(void);

    // プレイヤーカプセルとの押し戻し
    void PushBackFromPlayer(void);

    // 攻撃球の中心位置を取得（オフセット適用済み）
    VECTOR GetAttackSpherePos(void) const;

    // デバッグ描画
    void DrawDebugAI(void);
};