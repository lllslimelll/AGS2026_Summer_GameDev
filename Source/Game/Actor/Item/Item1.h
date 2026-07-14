#pragma once
#include <DxLib.h>
#include "Item.h"

class Item1 : public Item
{
public:

    Item1(const ItemData& data);
    ~Item1(void) override;
};

