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
        // ドロップ状態のみ対象
        if (item->GetState() != Item::STATE::DROPPED) continue;

        VECTOR itemPos = item->GetTransform().pos;

        // --- 条件① レイとアイテムのスフィア交差 ---
        bool hit = AsoUtility::IsHitSphereCapsule(
            itemPos, 30.0f, //座標・半径
            rayOrigin, rayEnd,
            0.0f  // 半径0 = 線分として扱う
        );
        if (!hit) continue;

        // --- 条件② ステージに遮られていないか ---
        // ①をパスしたものだけに適用（重い処理を後に置く）
        bool blocked = false;
        for (const auto& c : hitColliders_)
        {
            if (c->GetShape() != ColliderBase::SHAPE::MODEL) continue;

            const ColliderModel* model =
                static_cast<const ColliderModel*>(c);

            MV1_COLL_RESULT_POLY result =
                model->GetNearestHitPolyLine(rayOrigin, itemPos);

            if (result.HitFlag > 0)
            {
                blocked = true;
                break;
            }
        }
        if (blocked) continue;

        // --- 両条件クリア：最も近いものを採用 ---
        float dist = VSize(VSub(itemPos, rayOrigin));
        if (aimed == nullptr || dist < nearest)
        {
            aimed = item;
            nearest = dist;
        }
    }

    return aimed;
}

const std::vector<Item*>& ItemManager::GetAllItems(void) const
{
    return items_;
}

void ItemManager::LoadCsvData(void)
{
}
