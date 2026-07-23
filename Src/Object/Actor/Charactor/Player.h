#pragma once
#include <functional>
#include "CharactorBase.h"
#include "../../Inventory/Inventory.h"

class ItemManager;  
class Item;
class StageManager;

class Player : public CharactorBase
{
public:

	// 状態
	enum class STATE
	{
		IDLE,
		DEAD,
		END,
	};

	enum class ANIM_TYPE
	{
		IDLE,
		RUN,
		FAST_RUN,
		JUMP,
		DEAD
	};

	struct GUIDE_INFO
	{
		bool canPickUp;       // アイテムを拾える状態か
		bool isAimingRocket;  // ロケットに照準が当たってるか
		bool hasSelectedItem; // 選択中スロットにアイテムがあるか
		bool isIdle;          // IDLE状態か（置くボタン表示用）
		bool isPad;           // パッド使用中か
		bool hasAnyItem;	  // インベントリに1つでもアイテムがあるか
		int totalDelivered;
		int quota;
	};

	// 最大HP
	static constexpr int   MAX_HP = 100;
	// 最大酸素量
	static constexpr float MAX_OXYGEN = 200.0f;

	Player(ItemManager* itemMng, StageManager& stage_);

	~Player(void) override;

	// 描画
	void Draw(void) override;

	// HP取得
	int  GetHp(void) const;
	// 酸素量取得
	float GetOxygen(void) const;

	// カメラTransformを設定
	void SetCameraTransform(const Transform* cameraTransform);
	void SetForward(const VECTOR forward);

	// インベントリ取得
	const Inventory& GetInventory(void) const;

	GUIDE_INFO GetGuideInfo(void) const;


	// 被ダメージ
	void OnDamaged(int amount);


protected:
	
	// リソースロード
	virtual void InitLoad(void) override;
	
	// 大きさ、回転、座標の初期化
	virtual void InitTransform(void) override;

	// 衝突判定の初期化
	virtual void InitCollider(void) override;

	// アニメーションの初期化
	virtual void InitAnimation(void) override;

	// 初期化後の個別処理
	virtual void InitPost(void) override;

private:

	// 移動速度（通常）
	static constexpr float SPEED_MOVE = 7.0f;

	// 移動速度（ダッシュ）
	static constexpr float SPEED_DASH = 20.0f;

	// 衝突判定用線分開始
	static constexpr VECTOR COL_LINE_START_LOCAL_POS = { 0.0f, 80.0f, 0.0f };
	// 衝突判定用線分終了
	static constexpr VECTOR COL_LINE_END_LOCAL_POS = { 0.0f, -2.0f, 0.0f };

	// 衝突判定用線分開始（ジャンプ時）
	static constexpr VECTOR COL_LINE_JUMP_START_LOCAL_POS = { 0.0f, 130.f, 0.0f };

	// 衝突判定用線分終了（ジャンプ時）
	static constexpr VECTOR COL_LINE_JUMP_END_LOCAL_POS = { 0.0f, 50.0f, 0.0f };

	// 衝突判定用カプセル上部球体（ジャンプ時）
	static constexpr VECTOR COL_CAPSULE_TOP_JUMP_LOCAL_POS =
	{ 0.0f, 160.f, 0.0f };
	// 衝突判定用カプセル下部球体（ジャンプ時）
	static constexpr VECTOR COL_CAPSULE_DOWN_JUMP_LOCAL_POS =
	{ 0.0f, 80.0f, 0.0f };

	// ジャンプ力
	static constexpr float POW_JUMP_INIT = 1200.0f;
	// 持続ジャンプ力
	static constexpr float POW_JUMP_KEEP = 90.0f;
	// ジャンプ受付時間
	static constexpr float TIME_JUMP_INPUT = 0.5f;

	// 衝突判定用カプセル上部球体
	static constexpr VECTOR COL_CAPSULE_TOP_LOCAL_POS = { 0.0f, 110.0f, 0.0f };
	// 衝突判定用カプセル下部球体
	static constexpr VECTOR COL_CAPSULE_DOWN_LOCAL_POS = { 0.0f, 50.0f, 0.0f };

	// 衝突判定用カプセル
	static constexpr float COL_CAPSULE_RADIUS = 20.0f;

	// 状態
	STATE state_;

	// ステージ
	StageManager& stageMng_;

	// アイテム管理（拾得・投擲）
	ItemManager* itemMgr_;

	// インベントリ
	Inventory inventory_;

	// カメラTransform
	const Transform* cameraTransform_;
	VECTOR cameraForward_;

	// ブーストフラグ
	bool isBoost_;

	// 現在照準に当たっているアイテム
	Item* aimedItem_;

	// 照準の現在半径（補間用）
	float crosshairRadius_;

	// 状態遷移
	void ChangeState(STATE state);
	void ChangeStateIdle(void);
	void ChangeStateDead(void);
	void ChangeStateEnd(void);

	// 更新系
	void UpdateIdle(void);
	void UpdateDead(void);

	// アイテム関連更新
	void UpdateItem(void);

	// 照準に当たっているアイテムを取得して isAimed_ を更新
	void UpdateAimedItem(void);

	// 選択中のアイテムをプレイヤーに追従させる
	void UpdateFollowItem(void);

	// 拾う処理
	void ProcessPickUp(void);
	// 投擲処理
	void ProcessThrow(void);
	// ドロップ処理
	void ProcessDrop(void);

	void DrawFrameRecursive(int modelId, int frameIdx);

	// 選択中のインベントリスロットの変更
	void ChangeSelectedSlot();
	// インベントリが満杯か否か
	bool CanPickUp(void) const;

	// 移動処理
	void ProcessMove(void);
	void ProcessJump(void);

	// 更新系
	virtual void UpdateProcess(void) override;
	virtual void UpdateProcessPost(void) override;

	// 衝突判定
	void CollisionReserve(void) override;

	// 納品処理
	void ProcessDelivery(void);

	// ロケット照準判定
	bool IsAimingRoket(void) const;

	// ===== HP・酸素 =====
	static constexpr float OXYGEN_DASH_RATE = 3.0f; 	// 酸素消費倍率（ブースト中）
	static constexpr float SUFFOCATE_INTERVAL = 0.5f;   // ダメージ周期
	static constexpr int   SUFFOCATE_DAMAGE = MAX_HP / 100;  // 1

	int   hp_;
	float oxygen_;
	float suffocateTimer_;  // 酸素切れ後の経過時間

	// 更新
	void UpdateOxygenAndHp(void);
};

