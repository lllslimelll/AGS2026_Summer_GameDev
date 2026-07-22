#include <DxLib.h>
#include "../../../Object/Actor/Item/Item.h"
#include "../../../Object/Inventory/Inventory.h"
#include "InventoryUI.h"

InventoryUI::InventoryUI(const Inventory& inventory)
    : GameUI(),
    inventory_(inventory),
    itemImgs_{}
{
}

InventoryUI::~InventoryUI(void)
{
}

void InventoryUI::Load(void)
{
    itemImgs_[0] = LoadGraph("Data/Image/item2.png");
    itemImgs_[1] = LoadGraph("Data/Image/item1.png");
    itemImgs_[2] = LoadGraph("Data/Image/item3.png");
}

void InventoryUI::Release(void)
{
    for (int i = 0; i < IMG_COUNT; i++)
    {
        if (itemImgs_[i] != -1)
        {
            DeleteGraph(itemImgs_[i]);
            itemImgs_[i] = -1;
        }
    }
}

void InventoryUI::Draw(void)
{
    int screenW, screenH;
    GetScreenState(&screenW, &screenH, nullptr);

    const int totalWidth = Inventory::SLOT_MAX * SLOT_SPAN - SLOT_PADDING;
    const int startX = (screenW - totalWidth) / 2 + 10;
    const int baseY = screenH - SLOT_SIZE - MARGIN_BOTTOM;
    const int selected = inventory_.GetSelectedIndex();

    // かっこの長さ(px)
    constexpr int BRACKET_LEN = 20;
    constexpr int BRACKET_W = 3;   // 線の太さ
    // 深めの青
    const unsigned int COL_NORMAL = GetColor(30, 80, 160);
    const unsigned int COL_SELECTED = GetColor(60, 160, 255);

    for (int i = 0; i < Inventory::SLOT_MAX; i++)
    {
        const int x = startX + i * SLOT_SPAN;
        const bool isSel = (i == selected);
        const int  expand = isSel ? SELECTED_EXPAND : 0;
        const int  drawX = x - expand / 2;
        const int  drawY = baseY - expand / 2;
        const int  drawSize = SLOT_SIZE + expand;
        const unsigned int col = isSel ? COL_SELECTED : COL_NORMAL;

        // 左上かっこ
        DrawBox(drawX, drawY, drawX + BRACKET_W, drawY + BRACKET_LEN, col, TRUE);
        DrawBox(drawX, drawY, drawX + BRACKET_LEN, drawY + BRACKET_W, col, TRUE);
        // 右上かっこ
        DrawBox(drawX + drawSize - BRACKET_W, drawY, drawX + drawSize, drawY + BRACKET_LEN, col, TRUE);
        DrawBox(drawX + drawSize - BRACKET_LEN, drawY, drawX + drawSize, drawY + BRACKET_W, col, TRUE);
        // 左下かっこ
        DrawBox(drawX, drawY + drawSize - BRACKET_LEN, drawX + BRACKET_W, drawY + drawSize, col, TRUE);
        DrawBox(drawX, drawY + drawSize - BRACKET_W, drawX + BRACKET_LEN, drawY + drawSize, col, TRUE);
        // 右下かっこ
        DrawBox(drawX + drawSize - BRACKET_W, drawY + drawSize - BRACKET_LEN, drawX + drawSize, drawY + drawSize, col, TRUE);
        DrawBox(drawX + drawSize - BRACKET_LEN, drawY + drawSize - BRACKET_W, drawX + drawSize, drawY + drawSize, col, TRUE);

        // アイテム画像・価格(変更なし)
        const Item* item = inventory_.Get(i);
        if (item == nullptr) { continue; }

        const int typeIdx = static_cast<int>(item->GetType());
        if (typeIdx >= 0 && typeIdx < IMG_COUNT && itemImgs_[typeIdx] != -1)
        {
            const int imgDrawSize = drawSize - IMG_MARGIN * 2;
            DrawExtendGraph(
                drawX + IMG_MARGIN, drawY + IMG_MARGIN,
                drawX + IMG_MARGIN + imgDrawSize,
                drawY + IMG_MARGIN + imgDrawSize,
                itemImgs_[typeIdx], TRUE);
        }

        const int prevSize = GetFontSize();
        SetFontSize(PRICE_FONT);
        const int price = item->GetValue();
        const int textW = GetDrawFormatStringWidth("$%d", price);
        DrawFormatString(
            drawX + (drawSize - textW) / 2,
            drawY + drawSize + 6,
            GetColor(255, 255, 255),
            "$%d", price);
        SetFontSize(prevSize);
    }
}