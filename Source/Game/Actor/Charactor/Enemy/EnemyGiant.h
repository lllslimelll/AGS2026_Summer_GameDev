#pragma once
#include <DxLib.h>
#include "../../../../Core/Vector3.h"
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

    void Init(void) override;
    void Draw(void) override;

protected:

    void UpdateProcess(void)     override;
    void UpdateProcessPost(void) override;

private:
    // コライダー定数
    static constexpr Vector3 COL_CAPSULE_TOP_LOCAL_POS = Vector3(0.0f, 160.0f, 0.0f);
    static constexpr Vector3 COL_CAPSULE_DOWN_LOCAL_POS = Vector3(0.0f, 50.0f, 0.0f);
    static constexpr float   COL_CAPSULE_RADIUS = 40.0f;

    // モデル定数
    static constexpr float SCALE = 1.0f;
    static constexpr Vector3 DEFAULT_LOCAL_ROT = Vector3(0.0f, 0.0f, 0.0f);

    // 視野パラメータ
    static constexpr float VIEW_DIST = 1000.0f;
    static constexpr float VIEW_ANGLE = 60.0f;
    static constexpr float VIEW_HALF_FOV = VIEW_ANGLE * 0.5f * 3.14159265f / 180.0f;

    // 攻撃パラメータ
    static constexpr float ATTACK_SPHERE_RADIUS = 20.0f;
    static constexpr float ATTACK_DAMAGE = 20.0f;

    // 移動・AI距離パラメータ
    static constexpr float SPEED_PATROL = 1.0f;
    static constexpr float SPEED_CHASE = 3.0f;
    static constexpr float DIST_ATTACK = 100.0f;
    static constexpr float DIST_CHASE = 500.0f;

    // 攻撃フレームインデックス
    int attackHandFrame_ = -1;

    STATE state_ = STATE::NONE;
    float step_ = 0.0f;
    bool  attackHit_ = false;

    Vector3 spawnPos_;

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

    // 更新
    void UpdateNone(void);
    void UpdateThink(void);
    void UpdateIdle(void);
    void UpdatePatrol(void);
    void UpdateChase(void);
    void UpdateAttack(void);
    void UpdateReturn(void);
    void UpdateEnd(void);

    // ヘルパー
    void  SetMoveDirToTarget(const Vector3& target);
    bool  InSearchCone(void)  const;
    float DistToPlayer(void)  const;
    void  PushBackFromPlayer(void);
};