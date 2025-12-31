#ifndef __ITEM_MANAGER_H__
#define __ITEM_MANAGER_H__

#include "cocos2d.h"
#include "components/item/InventoryDef.h"
#include "entt/entt.hpp"

#include <unordered_map>

class ItemManager {
public:
    static ItemManager* getInstance();

    bool loadItems(const std::string& filePath = "items/items_json/items_blocks.json", bool append = false);

    const ItemDefinition* getItemData(int id) const;

    cocos2d::SpriteFrame* getItemSprite(int id);

    const std::vector<ItemDefinition>& getAllItems() const { return _items; }

    entt::registry& getRegistry() { return _registry; }
    const entt::registry& getRegistry() const { return _registry; }

    entt::entity getEntityById(int id) const;

    std::vector<entt::entity> getItemsByType(ItemType type) const;

    std::vector<entt::entity> getItemsByTag(int tag) const;

    size_t getItemCount() const;

private:
    ItemManager() = default;
    ~ItemManager() = default;

    ItemDefinition entityToDefinition(entt::entity entity) const;

    static ItemManager* _instance;

    entt::registry _registry;

    std::unordered_map<int, entt::entity> _idToEntity;

    mutable std::vector<ItemDefinition> _items;
    mutable bool _itemsCacheDirty = true;
};

#endif

