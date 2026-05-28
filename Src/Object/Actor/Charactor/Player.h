#pragma once
#include <array>
#include "CharactorBase.h"

class ItemManager;  
class Item;

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

	Player(ItemManager* itemMng);

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
	static constexpr float SPEED_MOVE = 5.0f;

	// 移動速度（ダッシュ）
	static constexpr float SPEED_DASH = 10.0f;

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

	// アイテム管理（拾得・投擲）
	ItemManager* itemMgr_;

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
};

