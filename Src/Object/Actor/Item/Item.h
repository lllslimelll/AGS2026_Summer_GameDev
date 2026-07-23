#pragma once
#include <functional>
#include <DxLib.h>
#include "../ActorBase.h"

class Item : public ActorBase
{
public:
	void SetCameraPos(const VECTOR& pos);
	// 衝突判定種別
	enum class COLLIDER_TYPE
	{
		GROUND_LINE,
		SPHERE,
		MAX,
	};

	// 種別
	enum class TYPE {
		type1,
		type2, 
		type3
	};

	// 値段のグレード
	enum class GRADE {
		LOW, 
		MID, 
		HIGH
	};

	// アイテムデータ
	struct ItemData{
		int id;			   // 識別ID
		Item::TYPE type;   // 種別
		Item::GRADE grade; // グレード
		int value;		   // 価値
		VECTOR defaultPos;    // 初期座標
		float scale;
	};

	// 状態
	enum class STATE {
		NONE,
		DROPPED, // 落ちている
		HELD,   // インベントリ内
		THROW, // 投擲
		DELIVERED, // 納品
	};

	// 拾える距離
	static constexpr float RANGE_PICKUP = 180.0f;

	static constexpr float RANGE_PICKUP_ROCKET = 500.0f;

	// 投擲時の初速
	static constexpr float THROW_SPEED = 40.0f;

	// グレード別の基本価値
	static constexpr int VALUE_LOW = 100;
	static constexpr int VALUE_MID = 500;
	static constexpr int VALUE_HIGH = 1000;

	// コンストラクタ
	Item(const ItemData& data);
	~Item() override; // デストラクタ

	// 更新
	void Update(void) override;
	// 描画
	void Draw(void) override;

	// 拾われた
	void OnPickedUp(void);
	// 投擲された
	void OnThrow(const VECTOR& throwDir);
	// 納品された
	void OnDelivered(void);
	// 敵に命中した
	void OnHitEnemy(void);

	// 取得
	TYPE GetType(void) const;  // 種別
	int   GetValue(void) const; // 値段
	GRADE GetGrade(void) const; // グレード
	STATE GetState(void) const; // 状態

	// 照準に当たってるか
	bool IsAimed(const VECTOR& rayOrigin,
				 const VECTOR& rayEnd) const;

	// アイテム取得可能か
	void SetAimed(bool aimed);
	// 選択されてるアイテムか
	void SetSelected(bool selected);

	// 保持中の追従アイテムを設定
	void SetHeldPos(const VECTOR& pos);

	// 納品したか否かを設定
	void SetNouhin(bool nouhin);

	void OnDrop(const VECTOR& pos);

protected:

	// リソースロード
	void InitLoad(void) override;
	// 大きさ、回転、座標の初期化
	void InitTransform(void) override;
	// 衝突判定の初期化
	void InitCollider(void) override;
	// アニメーションの初期化
	void InitAnimation(void) override;
	// 初期化後の個別処理
	void InitPost(void) override;

private:

	VECTOR camPos_;
	// 種別
	TYPE type_;
	// グレード
	GRADE grade_;
	// 初期位置
	const VECTOR defaultPos_;
	const float  scale_;
	// 価値
	int value_;

	// 状態
	STATE state_;

	// 投擲中の移動量
	VECTOR throwPow_;

	// アイテムを拾える状態か
	bool isAimed_;
	// 選択されてるアイテムか
	bool isSelected_;

	// アイテム価値のビルボード画像
	int valueBillImg_[3];


	// 納品されたか
	bool isNouhinn_ = false;

	// 状態管理
	std::map<STATE, std::function<void(void)>> stateChanges_;
	// 状態別更新
	std::function<void(void)> stateUpdate_;
	// 状態別描画
	std::function<void(void)> stateDraw_;

	// 状態遷移
	void ChangeState(STATE state);
	void ChangeDropped(void); // 落ちている状態
	void ChangeHeld(void);    // インベントリ内状態
	void ChangeThrow(void);  // 空中状態
	void ChangeDelivered(void); // 納品状態

	// 状態別更新
	void UpdateDropped(void); 
	void UpdateHeld(void);
	void UpdateThrow(void);
	void UpdateDelivered(void);

	// 状態別描画
	void DrawDropped(void);
	void DrawHeld(void);
	void DrawThrow(void);
	void DrawDelelivered(void);

	// 投擲中の弾道計算
	void UpdateThrowMove(void);

	// ビルボード描画（金額表示）
	void DrawBillboard(void) const;
	
};

