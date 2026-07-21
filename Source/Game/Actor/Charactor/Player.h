#pragma once
#include <functional>
#include "CharactorBase.h"
#include "../../Inventory/Inventory.h"

class ItemManager;
class Item;
class StageManager;
class CapsuleComponent;

class Player : public CharactorBase
{
public:

    // 状態
    enum class STATE { IDLE, DEAD, END };

    // アニメーション種別
    enum class ANIM_TYPE { IDLE, RUN, FAST_RUN, JUMP, DEAD };

    // UI へ渡すガイド情報
    struct GUIDE_INFO
    {
        bool canPickUp;        // アイテムを拾える状態か
        bool isAimingRocket;   // ロケットに照準が当たっているか
        bool hasSelectedItem;  // 選択中スロットにアイテムがあるか
        bool isIdle;           // IDLE 状態か
        bool isPad;            // パッド使用中か
        bool hasAnyItem;       // インベントリに 1 つでもアイテムがあるか
        int  totalDelivered;
        int  quota;
    };

    // 最大 HP
    static constexpr int   MAX_HP = 100;
    // 最大酸素量
    static constexpr float MAX_OXYGEN = 200.0f;

    Player(ItemManager* itemMng, StageManager& stageMng);
    ~Player(void) override;

    void Init(void) override;
    void Draw(void) override;

    // HP 取得
    int   GetHp(void)     const;
    // 酸素量取得
    float GetOxygen(void) const;

    // カメラの前方向を設定（移動用・XZ 平面に投影される）
    void SetCameraForward(const Vector3& forward);

    // カメラのレイ（位置と向き）を設定（照準・インタラクト用）
    void SetCameraRay(const Vector3& origin, const Vector3& dir);

    // インベントリ取得
    const Inventory& GetInventory(void) const;

    // UI ガイド情報取得
    GUIDE_INFO GetGuideInfo(void) const;

    // 被ダメージ
    void OnDamaged(int amount);

private:

    // ---- 移動速度 ----
    static constexpr float SPEED_MOVE = 15.0f;
    static constexpr float SPEED_DASH = 30.0f;

    // ---- カプセルコライダー定数 ----
    static constexpr Vector3 COL_CAPSULE_TOP_LOCAL_POS = Vector3(0.0f, 110.0f, 0.0f);
    static constexpr Vector3 COL_CAPSULE_DOWN_LOCAL_POS = Vector3(0.0f, 50.0f, 0.0f);
    static constexpr Vector3 COL_CAPSULE_TOP_JUMP_LOCAL_POS = Vector3(0.0f, 160.0f, 0.0f);
    static constexpr Vector3 COL_CAPSULE_DOWN_JUMP_LOCAL_POS = Vector3(0.0f, 80.0f, 0.0f);
    static constexpr float   COL_CAPSULE_RADIUS = 20.0f;

    // ---- ジャンプ定数 ----
    static constexpr float POW_JUMP_INIT = 1200.0f;
    static constexpr float POW_JUMP_KEEP = 90.0f;
    static constexpr float TIME_JUMP_INPUT = 0.5f;

    // ---- 酸素・HP 定数 ----
    static constexpr float OXYGEN_DASH_RATE = 5.0f;
    static constexpr float SUFFOCATE_INTERVAL = 1.0f;
    static constexpr int   SUFFOCATE_DAMAGE = MAX_HP / 100;

    // ---- メンバ ----

    STATE state_ = STATE::IDLE;

    // カプセルコライダーへの参照（CollisionReserve でサイズ変更に使う）
    CapsuleComponent* capsule_ = nullptr;

    StageManager& stageMng_;
    ItemManager* itemMgr_;

    Inventory inventory_;

    // GameScene から注入されるカメラ前方向（XZ 平面上に投影済み）
    Vector3 cameraForward_ = Vector3::FORWARD;

    // カメラのレイ（照準・インタラクト用、投影なしの生の向き）
    Vector3 cameraRayOrigin_ = Vector3::ZERO;
    Vector3 cameraRayDir_ = Vector3::FORWARD;

    bool  isBoost_ = false;
    Item* aimedItem_ = nullptr;
    float crosshairRadius_ = 5.0f;

    int   hp_ = MAX_HP;
    float oxygen_ = MAX_OXYGEN;
    float suffocateTimer_ = 0.0f;

    // ---- 状態遷移 ----
    void ChangeState(STATE state);
    void ChangeStateIdle(void);
    void ChangeStateDead(void);
    void ChangeStateEnd(void);

    // ---- 更新 ----
    void UpdateIdle(void);
    void UpdateDead(void);
    void UpdateItem(void);
    void UpdateAimedItem(void);
    void UpdateFollowItem(void);

    // ---- アクション ----
    void ProcessPickUp(void);
    void ProcessThrow(void);
    void ProcessDrop(void);
    void ProcessDelivery(void);
    void ProcessMove(void);
    void ProcessJump(void);

    // ---- CharactorBase オーバーライド ----
    void UpdateProcess(void)     override;
    void UpdateProcessPost(void) override;
    void CollisionReserve(void)  override;

    // ---- ユーティリティ ----
    bool CanPickUp(void)       const;
    bool IsAimingRocket(void)  const;
    void ChangeSelectedSlot(void);
    void DrawFrameRecursive(int modelId, int frameIdx);

    void UpdateOxygenAndHp(void);
};