#pragma once
#include <vector>
#include <DxLib.h>
#include "Item.h"

class ColliderBase;

class ItemManager
{
public:

	// コンストラクタ
	ItemManager(void);

	// デストラクタ
	~ItemManager(void);

	// 初期化
	void Init(void);

	// 更新
	void Update(void);

	// 描画
	void Draw(void);

	// 解放
	void Release(void);

	// アイテム生成
	Item* Create(const Item::ItemData& data);

	// 衝突対象となるコライダを登録
	// EnemyManagerと同じ流儀：Init()後に呼ぶ
	void AddHitCollider(const ColliderBase* collider);

	// 照準に当たっているアイテムを返す
	// 壁で遮られている場合は nullptr
	// rayOrigin : レイの始点（カメラ位置）
	// rayDir    : レイの方向（カメラ前方、正規化済み）
	// rayLength : レイの長さ（= 拾える最大距離）
	Item* GetAimedItem(
		const VECTOR& rayOrigin,
		const VECTOR& rayDir,
		float rayLength) const;

	// 投擲中アイテムの一覧（EnemyManagerの命中判定に使う）
	const std::vector<Item*>& GetThrowingItems(void) const;

	// 全アイテムの一覧
	const std::vector<Item*>& GetAllItems(void) const;

private:

	// 全アイテム
	std::vector<Item*> items_;

	// 投擲中アイテム（毎フレームUpdateFlyingListで再構築）
	std::vector<Item*> flyingItems_;

	// 遮蔽チェック用コライダー（GetAimedItemのステージ判定に使う）
	std::vector<const ColliderBase*> hitColliders_;

	// CSVからアイテムデータを読み込んで生成
	void LoadCsvData(void);

	// 投擲中リストを毎フレーム更新
	void UpdateThrowingList(void);

	// 納品済みアイテムを削除
	void RemoveDeliveredItems(void);
};

