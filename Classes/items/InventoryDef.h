// Item definitions and inventory slot structures
#ifndef __INVENTORY_DEF_H__
#define __INVENTORY_DEF_H__

#include <string>
#include <vector>

// Basic item category
enum class ItemType {
    Unknown = 0,
    Weapon,
    Tool,
    Material,
    Consumable,
    Placeable
};

// Static item definition loaded from items.json
struct ItemDefinition {
    int id = 0;
    std::string name;
    ItemType type = ItemType::Unknown;
    int maxStack = 1;
    std::string iconPath;
    int value = 0;
    std::vector<int> tags; // group ids for fuzzy recipe matching
};

// Runtime inventory slot
struct InventorySlot {
    int itemId = 0; // 0 means empty
    int count = 0;
    int prefixId = 0;
};

#endif // __INVENTORY_DEF_H__
