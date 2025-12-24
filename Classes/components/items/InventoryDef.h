#ifndef __INVENTORY_DEF_H__
#define __INVENTORY_DEF_H__

#include <string>
#include <vector>



enum class ItemType {
    Unknown = 0,
    Equipment,
    Materials,
    Placeables,
    Consumables
};

struct ItemId {
    int value;
    ItemId(int id = 0) : value(id) {}
};

struct ItemName {
    std::string value;
    ItemName(const std::string& n = "") : value(n) {}
};

struct ItemTypeComponent {
    ItemType type;
    ItemTypeComponent(ItemType t = ItemType::Unknown) : type(t) {}
};

struct StackLimit {
    int maxStack;
    StackLimit(int max = 1) : maxStack(max) {}
};

struct ItemIcon {
    std::string path;
    ItemIcon(const std::string& p = "") : path(p) {}
};

struct ItemValue {
    int value;
    ItemValue(int v = 0) : value(v) {}
};

struct ItemTags {
    std::vector<int> tags;
    ItemTags(const std::vector<int>& t = {}) : tags(t) {}
};

struct ItemDefinition {
    int id = 0;
    std::string name;
    ItemType type = ItemType::Unknown;
    int maxStack = 1;
    std::string iconPath;
    int value = 0;
    std::vector<int> tags;
};


struct InventorySlot {
    int itemId = 0;
    int count = 0;
    int prefixId = 0;
};

#endif
