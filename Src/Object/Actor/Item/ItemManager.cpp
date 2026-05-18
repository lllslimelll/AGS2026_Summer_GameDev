#include <string>
#include <fstream>
#include <DxLib.h>
#include "../../../Application.h"
#include "../../../Utility/AsoUtility.h"
#include "../../Collider/ColliderBase.h"
#include "../../Collider/ColliderModel.h"
#include "Item.h"
#include "ItemManager.h"

ItemManager::ItemManager(void)
{
}

ItemManager::~ItemManager(void)
{
}

void ItemManager::Init(void)
{
	LoadCsvData();
}

void ItemManager::Update(void)
{
	for (auto& item : items_){
		item->Update();
	}
}

void ItemManager::Draw(void)
{
	for (auto& item : items_) {
		item->Draw();
	}
}

void ItemManager::Release(void)
{
	for (auto& item : items_)
	{
		item->Release();
		delete item;
	}
	// アイテムとコライダーをクリア
	items_.clear();
	hitColliders_.clear();
}

Item* ItemManager::Create(const Item::GRADE grade, const VECTOR& pos)
{
	Item* item = new Item(grade, pos);

    if(item != nullptr)
            {
        // アイテムの初期化
        item->Init();
        items_.emplace_back(item);
	}
	return item;
}

void ItemManager::AddHitCollider(const ColliderBase* collider)
{
	// GetAimedItemの遮蔽チェック用コライダを追加
	hitColliders_.emplace_back(collider);

	// 衝突判定用コライダをアイテムに追加
	for (auto& item : items_)
	{
		item->AddHitCollider(collider);
	}
}

Item* ItemManager::GetAimedItem(
    const VECTOR& rayOrigin,   // カメラ位置
    const VECTOR& rayDir,      // カメラ前方（正規化済み）
    float rayLength) const     // レイの長さ＝拾える最大距離
{
	VECTOR rayEnd = VAdd(rayOrigin, VScale(rayDir, rayLength));

	Item* aimed = nullptr;
	float  nearest = rayLength;

	for (auto item : items_)
	{
		// アイテムがレイに当たってるか確認
		if (!item->IsAimedBy(rayOrigin, rayEnd)) continue;

		// アイテムとカメラの距離を計算
		float dist = VSize(VSub(item->GetTransform().pos, rayOrigin));
		
		if (aimed == nullptr || dist < nearest)
		{
			aimed = item;
			nearest = dist;
		}
	}
	// アイテムを返す
	return aimed;
}

const std::vector<Item*>& ItemManager::GetAllItems(void) const
{
    return items_;
}

void ItemManager::LoadCsvData(void)
{
    Create(Item::GRADE::HIGH, {300.0f, 2360.0f, 0.0f});
}
