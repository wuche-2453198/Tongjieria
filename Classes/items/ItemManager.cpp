#include "ItemManager.h"
#include "json/document.h"
#include "json/rapidjson.h"

using namespace cocos2d;

ItemManager* ItemManager::_instance = nullptr;

ItemManager* ItemManager::getInstance() {
    if (!_instance) {
        _instance = new ItemManager();
        // Try multiple known packs; succeeds as long as any file loads.
        bool loaded = false;
        const char* files[] = {"items.json", "items_blocks.json", "items_machines.json", "items_seeds.json"};
        for (auto file : files) {
            if (_instance->loadItems(file, loaded)) {
                loaded = true;
            }
        }
        if (!loaded) {
            CCLOG("ItemManager: no items loaded; check Resources/*.json existence.");
        }
    }
    return _instance;
}

bool ItemManager::loadItems(const std::string& filePath, bool append) {
    if (!append) {
        _items.clear();
        _idToIndex.clear();
    }

    std::string jsonContent = FileUtils::getInstance()->getStringFromFile(filePath);
    if (jsonContent.empty()) {
        CCLOG("ItemManager: %s not found or empty.", filePath.c_str());
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

        ItemDefinition def;
        def.id = itemJson.HasMember("id") ? itemJson["id"].GetInt() : 0;
        def.name = itemJson.HasMember("name") ? itemJson["name"].GetString() : "";
        def.type = static_cast<ItemType>(itemJson.HasMember("type") ? itemJson["type"].GetInt() : 0);
        def.maxStack = itemJson.HasMember("maxStack") ? itemJson["maxStack"].GetInt() : 1;
        def.iconPath = itemJson.HasMember("icon") ? itemJson["icon"].GetString() : "";
        def.value = itemJson.HasMember("value") ? itemJson["value"].GetInt() : 0;

        if (itemJson.HasMember("tags") && itemJson["tags"].IsArray()) {
            for (const auto& tag : itemJson["tags"].GetArray()) {
                if (tag.IsInt()) def.tags.push_back(tag.GetInt());
            }
        }

        if (def.id != 0) {
            _idToIndex[def.id] = _items.size();
            _items.push_back(def);
        }
    }

    CCLOG("ItemManager: Loaded %zu items total after %s", _items.size(), filePath.c_str());
    return true;
}

const ItemDefinition* ItemManager::getItemData(int id) const {
    auto it = _idToIndex.find(id);
    if (it == _idToIndex.end()) return nullptr;
    return &_items[it->second];
}

cocos2d::SpriteFrame* ItemManager::getItemSprite(int id) {
    const auto* def = getItemData(id);
    if (!def) return nullptr;

    auto cache = SpriteFrameCache::getInstance();
    auto frame = cache->getSpriteFrameByName(def->iconPath);
    if (frame) return frame;

    // Fallback: create a frame from file if not in cache
    auto sprite = Sprite::create(def->iconPath);
    return sprite ? sprite->getSpriteFrame() : nullptr;
}
