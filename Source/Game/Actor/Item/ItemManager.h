#pragma once
#include <vector>
#include "Item.h"

class ItemManager
{
public:

    ItemManager(void);
    ~ItemManager(void);

    void Init(void);
    void Update(void);
    void Draw(void);
    void Release(void);

    // アイテム生成
    Item* Create(const Item::ItemData& data);

    // 照準に当たっているアイテムを返す
    Item* GetAimedItem(
        const Vector3& rayOrigin,
        const Vector3& rayDir,
        float          rayLength) const;

    // 投擲中アイテムの一覧
    const std::vector<Item*>& GetThrowingItems(void) const;

    // 全アイテムの一覧
    const std::vector<Item*>& GetAllItems(void) const;

private:

    std::vector<Item*> items_;
    std::vector<Item*> flyingItems_;

    void LoadCsvData(void);
    void UpdateThrowingList(void);
    void RemoveDeliveredItems(void);
};