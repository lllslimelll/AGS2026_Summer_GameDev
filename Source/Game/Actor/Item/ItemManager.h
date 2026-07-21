#pragma once
#include <vector>
#include <functional>
#include "../ActorBase.h"
#include "Item.h"

class ItemManager : public ActorBase
{
public:

    // Item 生成を GameScene に委譲するための関数オブジェクト
    // GameScene が World 相当なので SpawnActor 権限をここで受け取る
    using SpawnItemFunc = std::function<Item* (const Item::ItemData&)>;

    ItemManager(SpawnItemFunc spawnFunc);
    ~ItemManager(void) override;

    void Init(void)    override;
    void Update(void)  override;
    void Draw(void)    override;
    void Release(void) override;

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

    SpawnItemFunc spawnFunc_;

    std::vector<Item*> items_;
    std::vector<Item*> flyingItems_;

    void LoadCsvData(void);
    void UpdateThrowingList(void);
    void RemoveDeliveredItems(void);
};