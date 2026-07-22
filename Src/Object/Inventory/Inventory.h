#pragma once
#include <array>

class Item;

class Inventory
{
public:

    static constexpr int SLOT_MAX = 4;

    Inventory(void);
    ~Inventory(void) = default;

    // アイテムを空きスロットに追加。成功したらtrue
    bool Add(Item* item);

    // 選択中スロットのアイテムを取り出す
    void RemoveSelected(void);

    // 指定スロットのアイテム取得
    Item* Get(int slot) const;

    // 選択中スロットのアイテム取得
    Item* GetSelected(void) const;

    // 選択中スロット番号取得
    int GetSelectedIndex(void) const;

    // 満杯か
    bool IsFull(void) const;
    // インベントリにアイテムがあるか
    bool HasAnyItem(void) const;

    // スロット選択
    void SelectNext(void);
    void SelectPrev(void);
    void Select(int slot);

private:

    // 非所有ポインタ（実体はItemManagerが管理）
    std::array<Item*, SLOT_MAX> slots_;

    int selectedSlot_;

    // 選択状態をItemに同期する
    void ApplySelection(int oldSlot, int newSlot);
};
