#include "ItemManager.h"
#include "json/document.h"
#include "json/rapidjson.h"

using namespace cocos2d;

ItemManager* ItemManager::_instance = nullptr;

namespace {
ItemType convertToItemType(int rawType) {
    switch (rawType) {
        case 1:
        case 2:
            return ItemType::Equipment;
        case 3:
            return ItemType::Materials;
        case 4:
            return ItemType::Consumables;
        case 5:
            return ItemType::Placeables;
        default:
            return ItemType::Unknown;
    }
}

ItemType convertToItemType(const std::string& str) {
    if (str == "Equipment") return ItemType::Equipment;
    if (str == "Materials") return ItemType::Materials;
    if (str == "Consumables") return ItemType::Consumables;
    if (str == "Placeables") return ItemType::Placeables;
    return ItemType::Unknown;
}

EquipType convertToEquipType(const std::string& str) {
    if (str == "helmet") return EquipType::Helmet;
    if (str == "chestplate") return EquipType::Chestplate;
    if (str == "leggings") return EquipType::Leggings;
    if (str == "accessory") return EquipType::Accessory;
    if (str == "pickaxe") return EquipType::Pickaxe;
    if (str == "weapon") return EquipType::Weapon;
    if (str == "sword") return EquipType::Sword;
    return EquipType::None;
}

const std::vector<std::string> kItemFiles = {
    "items/items_json/items_equipment.json",
    "items/items_json/items_placeables.json",
    "items/items_json/items_materials.json",
    "items/items_json/items_consumables.json",
};
}

ItemManager* ItemManager::getInstance() {
    if (!_instance) {
        _instance = new ItemManager();
        bool loaded = false;
        for (const auto& file : kItemFiles) {
            if (_instance->loadItems(file, loaded)) {
                loaded = true;
            }
        }
        if (!loaded) {
            CCLOG("ItemManager: No items loaded, check Resources/items/items_json/*.json");
        }
    }
    return _instance;
}

bool ItemManager::loadItems(const std::string& filePath, bool append) {
    if (!append) {
        _registry.clear();
        _idToEntity.clear();
        _itemsCacheDirty = true;
    }

    std::string jsonContent = FileUtils::getInstance()->getStringFromFile(filePath);
    if (jsonContent.empty()) {
        CCLOG("ItemManager: %s not found or empty", filePath.c_str());
        return false;
    }

    rapidjson::Document doc;
    doc.Parse(jsonContent.c_str());
    if (doc.HasParseError() || !doc.IsArray()) {
        CCLOG("ItemManager: Failed to parse %s", filePath.c_str());
        return false;
    }

    for (const auto& itemJson : doc.GetArray()) {
        if (!itemJson.IsObject()) continue;

        int id = itemJson.HasMember("id") ? itemJson["id"].GetInt() : 0;
        if (id == 0) continue;

        std::string name = itemJson.HasMember("name") ? itemJson["name"].GetString() : "";

        // Parse type field (supports both int and string)
        ItemType type = ItemType::Unknown;
        if (itemJson.HasMember("type")) {
            if (itemJson["type"].IsInt()) {
                type = convertToItemType(itemJson["type"].GetInt());
            } else if (itemJson["type"].IsString()) {
                type = convertToItemType(itemJson["type"].GetString());
            }
        }

        int maxStack = itemJson.HasMember("maxStack") ? itemJson["maxStack"].GetInt() : 1;
        std::string iconPath = itemJson.HasMember("icon") ? itemJson["icon"].GetString() : "";
        int value = itemJson.HasMember("value") ? itemJson["value"].GetInt() : 0;

        std::vector<int> tags;
        if (itemJson.HasMember("tags") && itemJson["tags"].IsArray()) {
            for (const auto& tag : itemJson["tags"].GetArray()) {
                if (tag.IsInt()) tags.push_back(tag.GetInt());
            }
        }

        // Read equipment-specific fields
        EquipType equipType = EquipType::None;
        if (itemJson.HasMember("equipType") && itemJson["equipType"].IsString()) {
            equipType = convertToEquipType(itemJson["equipType"].GetString());
        }

        int defense = 0;
        if (itemJson.HasMember("defense") && itemJson["defense"].IsInt()) {
            defense = itemJson["defense"].GetInt();
        }

        int damage = 0;
        if (itemJson.HasMember("damage") && itemJson["damage"].IsInt()) {
            damage = itemJson["damage"].GetInt();
        }

        // Read consumable-specific fields
        int healAmount = 0;
        if (itemJson.HasMember("healAmount") && itemJson["healAmount"].IsInt()) {
            healAmount = itemJson["healAmount"].GetInt();
        }

        std::string useAnimation = "";
        if (itemJson.HasMember("useAnimation") && itemJson["useAnimation"].IsString()) {
            useAnimation = itemJson["useAnimation"].GetString();
        }

        auto entity = _registry.create();
        _registry.emplace<ItemId>(entity, id);
        _registry.emplace<ItemName>(entity, name);
        _registry.emplace<ItemTypeComponent>(entity, type);
        _registry.emplace<StackLimit>(entity, maxStack);
        _registry.emplace<ItemIcon>(entity, iconPath);
        _registry.emplace<ItemValue>(entity, value);
        _registry.emplace<ItemTags>(entity, tags);
        _registry.emplace<EquipTypeComponent>(entity, equipType);
        _registry.emplace<DefenseComponent>(entity, defense);
        _registry.emplace<DamageComponent>(entity, damage);
        _registry.emplace<HealAmountComponent>(entity, healAmount);
        _registry.emplace<UseAnimationComponent>(entity, useAnimation);

        _idToEntity[id] = entity;
    }

    _itemsCacheDirty = true;
    CCLOG("ItemManager: Loaded %s, total %zu items", filePath.c_str(), _idToEntity.size());
    return true;
}

entt::entity ItemManager::getEntityById(int id) const {
    auto it = _idToEntity.find(id);
    if (it != _idToEntity.end()) {
        return it->second;
    }
    return entt::null;
}

const ItemDefinition* ItemManager::getItemData(int id) const {
    if (_itemsCacheDirty) {
        _items.clear();
        auto view = _registry.view<ItemId>();
        for (auto entity : view) {
            _items.push_back(entityToDefinition(entity));
        }
        _itemsCacheDirty = false;
    }

    for (const auto& item : _items) {
        if (item.id == id) {
            return &item;
        }
    }
    return nullptr;
}

ItemDefinition ItemManager::entityToDefinition(entt::entity entity) const {
    ItemDefinition def;

    if (auto* idComp = _registry.try_get<ItemId>(entity)) {
        def.id = idComp->value;
    }
    if (auto* nameComp = _registry.try_get<ItemName>(entity)) {
        def.name = nameComp->value;
    }
    if (auto* typeComp = _registry.try_get<ItemTypeComponent>(entity)) {
        def.type = typeComp->type;
    }
    if (auto* stackComp = _registry.try_get<StackLimit>(entity)) {
        def.maxStack = stackComp->maxStack;
    }
    if (auto* iconComp = _registry.try_get<ItemIcon>(entity)) {
        def.iconPath = iconComp->path;
    }
    if (auto* valueComp = _registry.try_get<ItemValue>(entity)) {
        def.value = valueComp->value;
    }
    if (auto* tagsComp = _registry.try_get<ItemTags>(entity)) {
        def.tags = tagsComp->tags;
    }
    if (auto* equipTypeComp = _registry.try_get<EquipTypeComponent>(entity)) {
        def.equipType = equipTypeComp->equipType;
    }
    if (auto* defenseComp = _registry.try_get<DefenseComponent>(entity)) {
        def.defense = defenseComp->defense;
    }
    if (auto* damageComp = _registry.try_get<DamageComponent>(entity)) {
        def.damage = damageComp->damage;
    }
    if (auto* healComp = _registry.try_get<HealAmountComponent>(entity)) {
        def.healAmount = healComp->healAmount;
    }
    if (auto* animComp = _registry.try_get<UseAnimationComponent>(entity)) {
        def.useAnimation = animComp->animation;
    }

    return def;
}

cocos2d::SpriteFrame* ItemManager::getItemSprite(int id) {
    auto entity = getEntityById(id);
    if (entity == entt::null) return nullptr;

    auto* iconComp = _registry.try_get<ItemIcon>(entity);
    if (!iconComp || iconComp->path.empty()) return nullptr;

    auto cache = SpriteFrameCache::getInstance();
    auto frame = cache->getSpriteFrameByName(iconComp->path);
    if (frame) return frame;

    auto sprite = Sprite::create(iconComp->path);
    return sprite ? sprite->getSpriteFrame() : nullptr;
}

std::vector<entt::entity> ItemManager::getItemsByType(ItemType type) const {
    std::vector<entt::entity> result;
    auto view = _registry.view<ItemTypeComponent>();

    for (auto entity : view) {
        const auto& typeComp = view.get<ItemTypeComponent>(entity);
        if (typeComp.type == type) {
            result.push_back(entity);
        }
    }

    return result;
}

std::vector<entt::entity> ItemManager::getItemsByTag(int tag) const {
    std::vector<entt::entity> result;
    auto view = _registry.view<ItemTags>();

    for (auto entity : view) {
        const auto& tagsComp = view.get<ItemTags>(entity);
        for (int t : tagsComp.tags) {
            if (t == tag) {
                result.push_back(entity);
                break;
            }
        }
    }

    return result;
}

size_t ItemManager::getItemCount() const {
    return _idToEntity.size();
}
