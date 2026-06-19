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
	for (auto& item : items_)
	{
		item->Update();
	}

	// 納品済みアイテムを掃除
	RemoveDeliveredItems();
}

void ItemManager::Draw(void)
{
	for (auto& item : items_)
	{
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

Item* ItemManager::Create(const Item::ItemData& data)
{
	Item* item = nullptr;

	// 種別事にアイテム生成
	switch (data.type)
	{
	case Item::TYPE::type1:
		item = new Item(data);
		break;
	case Item::TYPE::type2:
		item = new Item(data);
		break;
	case Item::TYPE::type3:
		item = new Item(data);
		break;
	default:
		break;
	}

	if (item != nullptr)
	{
		item->Init(); // 初期化
		items_.emplace_back(item); // アイテムリストに追加
	}

	// アイテムを返す
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
    // ファイルの読み込み
	std::ifstream ifs = std::ifstream(Application::PATH_CSV + "ItemData.csv");
	// エラー発生
	if (!ifs) return;

	// ファイルを1行ずつ読み込む
	std::string line; // 1行の文字情報
	std::vector<std::string> strSplit;  // 1行を1文字の動的配列に分割

	bool isHeader = true;

	while (getline(ifs, line)) 
	{
		if (isHeader) 
		{
			isHeader = false;
			continue;
		}

		// 空行スキップ
		if (line.empty() || line.find_first_not_of(",\r\n ") == std::string::npos) continue;

		// 1行をカンマ区切りで分割
		strSplit = AsoUtility::Split(line, ',');

		Item* item = nullptr;

		// 構造体に合わせて読み込みデータを格納
		Item::ItemData data = Item::ItemData();
		int idx = 0;
		// ID
		data.id = stoi(strSplit[idx++]);
		// 種別
		data.type = static_cast<Item::TYPE>(stoi(strSplit[idx++]));
		// グレード
		data.grade = static_cast<Item::GRADE>(stoi(strSplit[idx++]));
		// 価値
		data.value = stoi(strSplit[idx++]);
		// 初期座標
		data.defaultPos =
		{
			stof(strSplit[idx++]), // X
			stof(strSplit[idx++]), // Y
			stof(strSplit[idx++])  // Z
		};

		// アイテム生成
		Create(data);
	}

	ifs.close();
}

void ItemManager::RemoveDeliveredItems(void)
{
	auto it = items_.begin();
	while (it != items_.end())
	{
		if ((*it)->GetState() == Item::STATE::DELIVERED)
		{
			(*it)->Release();
			delete* it;
			it = items_.erase(it);  // eraseは次のイテレータを返す
		}
		else
		{
			++it;
		}
	}
}
