#ifndef __ITEM_MANAGER_H__
#define __ITEM_MANAGER_H__

#include "cocos2d.h"
#include "InventoryDef.h"

#include <unordered_map>

// Singleton responsible for loading and serving item definitions and icons
class ItemManager {
public:
    static ItemManager* getInstance();

    // Load definitions from json file (default: items.json in Resources)
    // If append=true, keep existing items and merge; otherwise clear first.
    bool loadItems(const std::string& filePath = "items.json", bool append = false);

    // Get definition by item id; returns nullptr if not found
    const ItemDefinition* getItemData(int id) const;

    // Convenience: fetch sprite frame for the item icon if present
    cocos2d::SpriteFrame* getItemSprite(int id);

    const std::vector<ItemDefinition>& getAllItems() const { return _items; }

private:
    ItemManager() = default;
    ~ItemManager() = default;

    static ItemManager* _instance;

    std::vector<ItemDefinition> _items;
    std::unordered_map<int, size_t> _idToIndex;
};

#endif // __ITEM_MANAGER_H__
