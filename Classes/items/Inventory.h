#ifndef __INVENTORY_H__
#define __INVENTORY_H__

#include "InventoryDef.h"

#include <vector>

// Simple inventory container with fixed capacity
class Inventory {
public:
    static Inventory* getInstance();

    // Initialize capacity (clears current slots)
    void init(size_t capacity = 50);

    // Add items; returns true if fully added, false if overflow
    bool addItem(int id, int count);

    // Add items and collect overflow as drops
    std::vector<InventorySlot> addItemWithOverflow(int id, int count);

    // Remove items; returns true if enough items were removed
    bool removeItem(int id, int count);

    // Swap two slot indices; returns false if indices invalid
    bool swapSlots(size_t a, size_t b);

    // Move part/all from one slot to another; auto stack if same item
    bool moveItem(size_t from, size_t to, int count);

    // First empty slot index, or -1 if full
    int getFirstEmptySlot() const;

    // Total count of an item across slots
    int getItemCount(int id) const;

    const std::vector<InventorySlot>& getSlots() const { return _slots; }

    void clear();

private:
    Inventory() = default;
    ~Inventory() = default;

    static Inventory* _instance;

    std::vector<InventorySlot> _slots;

    void dispatchInventoryChanged();
    int addItemInternal(int id, int count);
};

#endif // __INVENTORY_H__
