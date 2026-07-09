#include "../Actor/Item/Item.h"
#include "Inventory.h"

Inventory::Inventory(void)
    :
    slots_{},
    selectedSlot_(0)
{
}

bool Inventory::Add(Item* item)
{
    if (item == nullptr) { return false; }

    for (int i = 0; i < SLOT_MAX; i++)
    {
        if (slots_[i] == nullptr)
        {
            slots_[i] = item;
            if (i == selectedSlot_) { item->SetSelected(true); }
            return true;
        }
    }
    return false;
}

void Inventory::RemoveSelected(void)
{
    slots_[selectedSlot_] = nullptr;
}

Item* Inventory::Get(int slot) const
{
    if (slot < 0 || slot >= SLOT_MAX) { return nullptr; }
    return slots_[slot];
}

Item* Inventory::GetSelected(void) const
{
    return slots_[selectedSlot_];
}

int Inventory::GetSelectedIndex(void) const
{
    return selectedSlot_;
}

bool Inventory::IsFull(void) const
{
    for (int i = 0; i < SLOT_MAX; i++)
    {
        if (slots_[i] == nullptr) { return false; }
    }
    return true;
}

void Inventory::SelectNext(void)
{
    int old = selectedSlot_;
    selectedSlot_ = (selectedSlot_ + 1) % SLOT_MAX;
    ApplySelection(old, selectedSlot_);
}

void Inventory::SelectPrev(void)
{
    int old = selectedSlot_;
    selectedSlot_ = (selectedSlot_ - 1 + SLOT_MAX) % SLOT_MAX;
    ApplySelection(old, selectedSlot_);
}

void Inventory::Select(int slot)
{
    if (slot < 0 || slot >= SLOT_MAX) { return; }
    if (slot == selectedSlot_) { return; }

    int old = selectedSlot_;
    selectedSlot_ = slot;
    ApplySelection(old, selectedSlot_);
}

void Inventory::ApplySelection(int oldSlot, int newSlot)
{
    if (slots_[oldSlot] != nullptr) { slots_[oldSlot]->SetSelected(false); }
    if (slots_[newSlot] != nullptr) { slots_[newSlot]->SetSelected(true); }
}