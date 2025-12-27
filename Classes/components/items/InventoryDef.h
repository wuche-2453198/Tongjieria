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

// Equipment slot types for equipment panel
enum class EquipSlotType {
    None = -1,
    Helmet = 0,      // 头盔
    Chestplate = 1,  // 胸甲
    Leggings = 2,    // 护腿
    Accessory0 = 3,  // 饰品槽 1
    Accessory1 = 4,  // 饰品槽 2
    Accessory2 = 5,  // 饰品槽 3
    Accessory3 = 6   // 饰品槽 4
};

// Equipment types that items can be
enum class EquipType {
    None = 0,        // 非装备
    Helmet,          // 头盔
    Chestplate,      // 胸甲
    Leggings,        // 护腿
    Accessory,       // 饰品
    Pickaxe,         // 镐子
    Weapon           // 武器
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

// Equipment type component for ECS
struct EquipTypeComponent {
    EquipType equipType;
    EquipTypeComponent(EquipType et = EquipType::None) : equipType(et) {}
};

// Defense component for armor
struct DefenseComponent {
    int defense;
    DefenseComponent(int def = 0) : defense(def) {}
};

// Consumable item components
struct HealAmountComponent {
    int healAmount;
    HealAmountComponent(int heal = 0) : healAmount(heal) {}
};

struct UseAnimationComponent {
    std::string animation;
    UseAnimationComponent(const std::string& anim = "") : animation(anim) {}
};

struct ItemDefinition {
    int id = 0;
    std::string name;
    ItemType type = ItemType::Unknown;
    int maxStack = 1;
    std::string iconPath;
    int value = 0;
    std::vector<int> tags;
    EquipType equipType = EquipType::None;  // Equipment type (None if not equipment)
    int defense = 0;                         // Defense value for armor

    // Consumable item properties
    int healAmount = 0;                      // Health restored when consumed
    std::string useAnimation = "";           // Animation to play: "eat" or "drink"
};


struct InventorySlot {
    int itemId = 0;
    int count = 0;
    int prefixId = 0;
};

#endif
