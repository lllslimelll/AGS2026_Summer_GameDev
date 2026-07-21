#include <string>
#include <fstream>
#include <DxLib.h>
#include "../../../Application.h"
#include "../../../Utility/AsoUtility.h"
#include "Item.h"
#include "ItemManager.h"

ItemManager::ItemManager(SpawnItemFunc spawnFunc)
    : ActorBase()
    , spawnFunc_(spawnFunc)
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
    // Item の解放は GameScene(actors_) が担当するため
    // ここではリストをクリアするだけ
    items_.clear();
    flyingItems_.clear();
}

Item* ItemManager::Create(const Item::ItemData& data)
{
    // Item の生成は GameScene(World 相当)に委譲する
    Item* item = spawnFunc_(data);
    if (item != nullptr)
    {
        items_.emplace_back(item);
    }
    return item;
}

Item* ItemManager::GetAimedItem(
    const Vector3& rayOrigin,
    const Vector3& rayDir,
    float          rayLength) const
{
    Vector3 rayEnd = rayOrigin + rayDir * rayLength;
    Item* aimed = nullptr;
    float   nearest = rayLength;

    for (auto item : items_)
    {
        if (!item->IsAimed(rayOrigin, rayEnd)) continue;

        float dist = Vector3::Distance(item->GetPos(), rayOrigin);
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

const std::vector<Item*>& ItemManager::GetThrowingItems(void) const
{
    return flyingItems_;
}

void ItemManager::LoadCsvData(void)
{
    std::ifstream ifs(Application::PATH_CSV + "ItemData.csv");
    if (!ifs) return;

    std::string line;
    std::vector<std::string> strSplit;
    bool isHeader = true;

    while (getline(ifs, line))
    {
        if (isHeader) { isHeader = false; continue; }
        if (line.empty() || line.find_first_not_of(",\r\n ") == std::string::npos) continue;

        strSplit = AsoUtility::Split(line, ',');

        Item::ItemData data = Item::ItemData();
        int idx = 0;

        data.id = stoi(strSplit[idx++]);
        data.type = static_cast<Item::TYPE>(stoi(strSplit[idx++]));
        data.grade = static_cast<Item::GRADE>(stoi(strSplit[idx++]));
        data.value = stoi(strSplit[idx++]);
        data.defaultPos = Vector3(
            stof(strSplit[idx++]),
            stof(strSplit[idx++]),
            stof(strSplit[idx++])
        );

        Create(data);
    }
}

void ItemManager::RemoveDeliveredItems(void)
{
    // 納品済みアイテムをリストから除外する
    // 実体の解放は GameScene の actors_ が担当するため erase のみ
    for (auto it = items_.begin(); it != items_.end();)
    {
        if ((*it)->GetState() == Item::STATE::DELIVERED)
            it = items_.erase(it);  // eraseが次のイテレータを返す
        else
            ++it;
    }
}

void ItemManager::UpdateThrowingList(void)
{
    flyingItems_.clear();
    for (auto item : items_)
    {
        if (item->GetState() == Item::STATE::THROW)
        {
            flyingItems_.push_back(item);
        }
    }
}