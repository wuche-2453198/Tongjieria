#ifndef __INVENTORY_H__
#define __INVENTORY_H__

#include "components/items/InventoryDef.h"


//固定物品栏定义 
class Inventory {
public:
    static Inventory* getInstance();

    // Initialize capacity (clears current slots)
    // Default: 40 general slots + 10个快捷键栏 + 4 armor + 4 coin + 1 trash = 59
    void init(size_t capacity = 59);

    // Add items; returns true if fully added, false if overflow
    //添加物品定义
    bool addItem(int id, int count);

    // Add items and collect overflow as drops
    //尝试把指定数量的物品装入背包
    std::vector<InventorySlot> addItemWithOverflow(int id, int count);

    // Remove items; returns true if enough items were removed
    //去除物品
    bool removeItem(int id, int count);

    // Swap two slot indices; returns false if indices invalid
    //交换物品
    bool swapSlots(size_t a, size_t b);

    // Move part/all from one slot to another; auto stack if same item
    //移动物品位置
    bool moveItem(size_t from, size_t to, int count);

    // First empty slot index, or -1 if full
    //返回第一个空栏的下标
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
