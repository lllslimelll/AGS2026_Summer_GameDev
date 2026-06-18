#pragma once
#include <array>
#include "CharactorBase.h"

class ItemManager;  
class Item;
class Stage;

class Player : public CharactorBase
{
public:

	enum class ANIM_TYPE
	{
		IDLE,
		RUN,
		FAST_RUN,
		JUMP,
	};

	Player(ItemManager* itemMng, Stage* stage_);

	~Player(void) override;

	// 描画
	void Draw(void) override;

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
	static constexpr float SPEED_MOVE = 2.5f;

	// 移動速度（ダッシュ）
	static constexpr float SPEED_DASH = 5.0f;

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

	// インベントリ
	static constexpr int INVENTORY_MAX = 5;

	// ステージ
	Stage* stage_;

	// アイテム管理（拾得・投擲）
	ItemManager* itemMgr_;

	// ブーストフラグ
	bool isBoost_;

	// 現在照準に当たっているアイテム
	Item* aimedItem_;
	// インベントリ
	std::array<Item*, INVENTORY_MAX> inventory_;


	// 現在選択中のインベントリスロット
	int selectedSlot_;

	// インベントトリ内アイテムの画像
	int inventoryItemImgs_[INVENTORY_MAX];

	// 照準の現在半径（補間用）
	float crosshairRadius_;

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

	// インベントリに追加
	void AddInventory(Item* item);
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
	static constexpr int   MAX_HP = 100;
	static constexpr float MAX_OXYGEN = 300.0f;  // 秒
	static constexpr float OXYGEN_DASH_RATE = 2.0f; 	// 酸素消費倍率（ブースト中）
	static constexpr float SUFFOCATE_INTERVAL = 0.4f;   // ダメージ周期
	static constexpr int   SUFFOCATE_DAMAGE = MAX_HP / 100;  // 1

	int   hp_;
	float oxygen_;
	float suffocateTimer_;  // 酸素切れ後の経過時間
	bool  isDead_;

	// 更新
	void UpdateOxygenAndHp(void);

	// ダメージ
	void TakeDamage(int amount);

	// ゲームオーバー処理
	void OnDeath(void);

	// UI描画
	void DrawStatusUI(void);



	// 死亡メニュー
	enum class DEATH_MENU
	{
		RETRY,
		TITLE,
		MAX,
	};
	int deathMenuIndex_ = 0;  // 0=リトライ, 1=タイトル

	// 死亡メニュー処理
	void UpdateDeathMenu(void);
	void DrawDeathMenu(void);


	float fullInventoryMsgTimer_ = 0;

	// UIバー画像
	int hpBarImg_;
	int o2BarImg_;
};

