#pragma once
#include "GameUI.h"

class Inventory;

class InventoryUI : public GameUI
{
public:
    explicit InventoryUI(const Inventory& inventory);
    ~InventoryUI(void) override;

    void Load(void);
    void Draw(void) override;
    void Release(void);

private:

    const Inventory& inventory_;

    static constexpr int SLOT_SIZE = 130;
    static constexpr int SLOT_PADDING = 70;
    static constexpr int SLOT_SPAN = SLOT_SIZE + SLOT_PADDING;
    static constexpr int MARGIN_BOTTOM = 66;
    static constexpr int SELECTED_EXPAND = 30;
    static constexpr int PRICE_FONT = 30;
    static constexpr int IMG_MARGIN = 10;
    static constexpr int IMG_COUNT = 3;

    int itemImgs_[IMG_COUNT];
};