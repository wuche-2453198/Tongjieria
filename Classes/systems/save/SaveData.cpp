#include "SaveData.h"
#include <fstream>
#include <sstream>
#include <algorithm>

USING_NS_CC;

SimpleSaveManager* SimpleSaveManager::instance = nullptr;

SimpleSaveManager* SimpleSaveManager::getInstance() {
    if (!instance) {
        instance = new SimpleSaveManager();
    }
    return instance;
}

// ==================== 文件路径辅助 ====================
std::string SimpleSaveManager::getSaveFilePath(int slot) const {
    std::string dir = FileUtils::getInstance()->getWritablePath() + "saves/";
    FileUtils::getInstance()->createDirectory(dir.c_str());
    return dir + "save_" + std::to_string(slot) + ".dat";
}

std::string SimpleSaveManager::getEnttSaveFilePath(int slot) const {
    std::string dir = FileUtils::getInstance()->getWritablePath() + "saves/";
    return dir + "entt_" + std::to_string(slot) + ".bin";
}

// ==================== 基本存档操作 ====================
bool SimpleSaveManager::saveGame(int slot, const SimpleSaveData& data) {
    std::string path = getSaveFilePath(slot);

    UserDefault* ud = UserDefault::getInstance();
    std::string prefix = "save_" + std::to_string(slot) + "_";

    // 保存基本数据
    ud->setIntegerForKey((prefix + "slot").c_str(), data.slotNumber);
    ud->setStringForKey((prefix + "name").c_str(), data.saveName.c_str());
    ud->setDoubleForKey((prefix + "time").c_str(), (double)data.saveTime);

    // 保存玩家数据
    ud->setStringForKey((prefix + "playerName").c_str(), data.playerName.c_str());
    ud->setIntegerForKey((prefix + "playerLevel").c_str(), data.playerLevel);
    ud->setIntegerForKey((prefix + "playerExp").c_str(), data.playerExp);
    ud->setFloatForKey((prefix + "playerPosX").c_str(), data.playerPos.position.x);
    ud->setFloatForKey((prefix + "playerPosY").c_str(), data.playerPos.position.y);
    ud->setIntegerForKey((prefix + "playerHealth").c_str(), data.playerHealth);
    ud->setIntegerForKey((prefix + "playerMaxHealth").c_str(), data.playerMaxHealth);
    ud->setIntegerForKey((prefix + "playerCoins").c_str(), data.playerCoins);
    ud->setIntegerForKey((prefix + "playerMana").c_str(), data.playerMana);
    ud->setIntegerForKey((prefix + "playerMaxMana").c_str(), data.playerMaxMana);

    // 保存物品数据
    std::string inventoryStr = EnttSerializer::serializeInventory(data.inventory);
    ud->setStringForKey((prefix + "inventory").c_str(), inventoryStr.c_str());

    // 标记存档存在
    ud->setBoolForKey((prefix + "exists").c_str(), true);
    ud->flush();

    CCLOG("Game saved to slot %d", slot);
    return true;
}

bool SimpleSaveManager::loadGame(int slot, SimpleSaveData& outData) {
    UserDefault* ud = UserDefault::getInstance();
    std::string prefix = "save_" + std::to_string(slot) + "_";

    if (!ud->getBoolForKey((prefix + "exists").c_str(), false)) {
        return false;
    }

    outData.slotNumber = slot;
    outData.saveName = ud->getStringForKey((prefix + "name").c_str(), "");
    outData.saveTime = (time_t)ud->getDoubleForKey((prefix + "time").c_str(), 0);

    // 加载玩家数据
    outData.playerName = ud->getStringForKey((prefix + "playerName").c_str(), "Player");
    outData.playerLevel = ud->getIntegerForKey((prefix + "playerLevel").c_str(), 1);
    outData.playerExp = ud->getIntegerForKey((prefix + "playerExp").c_str(), 0);
    outData.playerPos.position.x = ud->getFloatForKey((prefix + "playerPosX").c_str(), 0.0f);
    outData.playerPos.position.y = ud->getFloatForKey((prefix + "playerPosY").c_str(), 0.0f);
    outData.playerHealth = ud->getIntegerForKey((prefix + "playerHealth").c_str(), 100);
    outData.playerMaxHealth = ud->getIntegerForKey((prefix + "playerMaxHealth").c_str(), 100);
    outData.playerCoins = ud->getIntegerForKey((prefix + "playerCoins").c_str(), 0);
    outData.playerMana = ud->getIntegerForKey((prefix + "playerMana").c_str(), 100);
    outData.playerMaxMana = ud->getIntegerForKey((prefix + "playerMaxMana").c_str(), 100);

    // 加载物品数据
    std::string inventoryStr = ud->getStringForKey((prefix + "inventory").c_str(), "");
    outData.inventory = EnttSerializer::deserializeInventory(inventoryStr);

    CCLOG("Game loaded from slot %d", slot);
    CCLOG("Player: %s Lv.%d (HP: %d/%d)",
        outData.playerName.c_str(), outData.playerLevel,
        outData.playerHealth, outData.playerMaxHealth);
    return true;
}

bool SimpleSaveManager::deleteGame(int slot) {
    UserDefault* ud = UserDefault::getInstance();
    std::string prefix = "save_" + std::to_string(slot) + "_";

    // 清除所有相关键
    ud->setBoolForKey((prefix + "exists").c_str(), false);
    ud->flush();

    // 删除可能的 EnTT 数据文件
    std::string enttPath = getEnttSaveFilePath(slot);
    if (FileUtils::getInstance()->isFileExist(enttPath)) {
        FileUtils::getInstance()->removeFile(enttPath);
    }

    CCLOG("Game deleted from slot %d", slot);
    return true;
}

std::vector<SimpleSaveData> SimpleSaveManager::getAllSaves() {
    std::vector<SimpleSaveData> saves;

    for (int i = 1; i <= 8; i++) {
        SimpleSaveData data;
        if (loadGame(i, data)) {
            saves.push_back(data);
        }
        else {
            // 空存档槽
            data.slotNumber = i;
            data.saveName = "Empty Slot";
            data.saveTime = 0;
            data.playerLevel = 0;
            saves.push_back(data);
        }
    }

    return saves;
}

bool SimpleSaveManager::isSlotEmpty(int slot) {
    UserDefault* ud = UserDefault::getInstance();
    std::string prefix = "save_" + std::to_string(slot) + "_";
    return !ud->getBoolForKey((prefix + "exists").c_str(), false);
}

// ==================== EnttSerializer 实现 ====================
std::string EnttSerializer::serializeTransform(const ECS::Transform& transform) {
    std::stringstream ss;
    ss << transform.x << "," << transform.y << ","
        << transform.rotation << "," << transform.scaleX << "," << transform.scaleY;
    return ss.str();
}

ECS::Transform EnttSerializer::deserializeTransform(const std::string& data) {
    ECS::Transform transform;
    std::stringstream ss(data);
    std::string token;

    try {
        std::getline(ss, token, ',');
        transform.x = std::stof(token);
        std::getline(ss, token, ',');
        transform.y = std::stof(token);
        std::getline(ss, token, ',');
        transform.rotation = std::stof(token);
        std::getline(ss, token, ',');
        transform.scaleX = std::stof(token);
        std::getline(ss, token, ',');
        transform.scaleY = std::stof(token);
    }
    catch (...) {
        CCLOG("Failed to deserialize Transform: %s", data.c_str());
    }

    return transform;
}

std::string EnttSerializer::serializePlayerComponent(const ECS::PlayerComponent& player) {
    std::stringstream ss;
    ss << player.name << "|"
        << player.level << "|"
        << player.exp << "|"
        << player.health << "|"
        << player.maxHealth << "|"
        << player.coins << "|"
        << player.mana << "|"
        << player.maxMana << "|"
        << player.moveSpeed;
    return ss.str();
}

ECS::PlayerComponent EnttSerializer::deserializePlayerComponent(const std::string& data) {
    ECS::PlayerComponent player;
    std::stringstream ss(data);
    std::string token;
    int index = 0;

    while (std::getline(ss, token, '|')) {
        try {
            switch (index) {
            case 0: player.name = token; break;
            case 1: player.level = std::stoi(token); break;
            case 2: player.exp = std::stoi(token); break;
            case 3: player.health = std::stoi(token); break;
            case 4: player.maxHealth = std::stoi(token); break;
            case 5: player.coins = std::stoi(token); break;
            case 6: player.mana = std::stoi(token); break;
            case 7: player.maxMana = std::stoi(token); break;
            case 8: player.moveSpeed = std::stof(token); break;
            }
        }
        catch (...) {
            CCLOG("Failed to deserialize PlayerComponent at index %d", index);
        }
        index++;
    }

    return player;
}

std::string EnttSerializer::serializeInventory(const std::map<int, int>& inventory) {
    std::stringstream ss;
    for (const auto& item : inventory) {
        ss << item.first << ":" << item.second << ";";
    }
    return ss.str();
}

std::map<int, int> EnttSerializer::deserializeInventory(const std::string& data) {
    std::map<int, int> inventory;
    std::stringstream ss(data);
    std::string itemStr;

    while (std::getline(ss, itemStr, ';')) {
        if (itemStr.empty()) continue;

        size_t colonPos = itemStr.find(':');
        if (colonPos != std::string::npos) {
            try {
                int itemId = std::stoi(itemStr.substr(0, colonPos));
                int count = std::stoi(itemStr.substr(colonPos + 1));
                inventory[itemId] = count;
            }
            catch (...) {
                CCLOG("Failed to deserialize inventory item: %s", itemStr.c_str());
            }
        }
    }

    return inventory;
}

SimpleSaveData EnttSerializer::extractFromEntity(entt::registry& registry, entt::entity playerEntity) {
    SimpleSaveData saveData;

    if (!registry.valid(playerEntity)) {
        return saveData;
    }

    // 从 Transform 组件提取位置
    if (registry.all_of<ECS::Transform>(playerEntity)) {
        const auto& transform = registry.get<ECS::Transform>(playerEntity);
        saveData.playerPos.position.x = transform.x;
        saveData.playerPos.position.y = transform.y;
    }

    // 从 PlayerComponent 提取玩家数据
    if (registry.all_of<ECS::PlayerComponent>(playerEntity)) {
        const auto& playerComp = registry.get<ECS::PlayerComponent>(playerEntity);
        saveData.playerName = playerComp.name;
        saveData.playerLevel = playerComp.level;
        saveData.playerExp = playerComp.exp;
        saveData.playerHealth = playerComp.health;
        saveData.playerMaxHealth = playerComp.maxHealth;
        saveData.playerCoins = playerComp.coins;
        saveData.playerMana = playerComp.mana;
        saveData.playerMaxMana = playerComp.maxMana;
    }

    // 从 ItemComponent（如果有）提取物品数据
    auto view = registry.view<ECS::ItemComponent>();
    for (auto entity : view) {
        const auto& item = registry.get<ECS::ItemComponent>(entity);
        // 假设物品ID为 key
        saveData.inventory[item.itemId] = item.stackSize;
    }

    return saveData;
}

void EnttSerializer::applyToEntity(entt::registry& registry, entt::entity playerEntity,
    const SimpleSaveData& data) {
    if (!registry.valid(playerEntity)) {
        return;
    }

    // 应用位置到 Transform 组件
    if (registry.all_of<ECS::Transform>(playerEntity)) {
        auto& transform = registry.get<ECS::Transform>(playerEntity);
        transform.x = data.playerPos.position.x;
        transform.y = data.playerPos.position.y;
    }
    else {
        // 如果没有 Transform 组件，添加一个
        auto& transform = registry.emplace<ECS::Transform>(playerEntity);
        transform.x = data.playerPos.position.x;
        transform.y = data.playerPos.position.y;
    }

    // 应用数据到 PlayerComponent 组件
    if (registry.all_of<ECS::PlayerComponent>(playerEntity)) {
        auto& playerComp = registry.get<ECS::PlayerComponent>(playerEntity);
        playerComp.name = data.playerName;
        playerComp.level = data.playerLevel;
        playerComp.exp = data.playerExp;
        playerComp.health = data.playerHealth;
        playerComp.maxHealth = data.playerMaxHealth;
        playerComp.coins = data.playerCoins;
        playerComp.mana = data.playerMana;
        playerComp.maxMana = data.playerMaxMana;
    }
    else {
        // 如果没有 PlayerComponent，添加一个
        ECS::PlayerComponent playerComp;
        playerComp.name = data.playerName;
        playerComp.level = data.playerLevel;
        playerComp.exp = data.playerExp;
        playerComp.health = data.playerHealth;
        playerComp.maxHealth = data.playerMaxHealth;
        playerComp.coins = data.playerCoins;
        playerComp.mana = data.playerMana;
        playerComp.maxMana = data.playerMaxMana;
        registry.emplace<ECS::PlayerComponent>(playerEntity, playerComp);
    }
}

// ==================== 查找玩家实体 ====================
entt::entity SimpleSaveManager::findPlayerEntity(entt::registry& registry) {
    // 查找带有 PlayerComponent 的实体
    auto view = registry.view<ECS::PlayerComponent>();
    if (view.begin() != view.end()) {
        return *view.begin();
    }

    // 查找带有 TagComponent 且 tag 为 "player" 的实体
    auto tagView = registry.view<ECS::TagComponent>();
    for (auto entity : tagView) {
        const auto& tag = registry.get<ECS::TagComponent>(entity);
        if (tag.tag == "player") {
            return entity;
        }
    }

    return entt::null;
}

// ==================== EnTT 整合存档操作 ====================
bool SimpleSaveManager::saveGameWithEntt(int slot, entt::registry& registry,
    entt::entity playerEntity) {
    if (!registry.valid(playerEntity)) {
        CCLOG("Invalid player entity for save");
        return false;
    }

    // 从实体提取数据
    SimpleSaveData saveData = EnttSerializer::extractFromEntity(registry, playerEntity);

    // 设置存档基本信息
    saveData.slotNumber = slot;
    saveData.saveTime = time(nullptr);

    if (saveData.saveName.empty()) {
        saveData.saveName = "Save " + std::to_string(slot);
    }

    // 保存到文件
    bool success = saveGame(slot, saveData);

    if (success) {
        CCLOG("EnTT game saved to slot %d", slot);
    }

    return success;
}

bool SimpleSaveManager::loadGameWithEntt(int slot, entt::registry& registry,
    entt::entity playerEntity) {
    SimpleSaveData saveData;
    if (!loadGame(slot, saveData)) {
        return false;
    }

    // 应用数据到实体
    EnttSerializer::applyToEntity(registry, playerEntity, saveData);

    CCLOG("EnTT game loaded from slot %d", slot);
    return true;
}

// ==================== 快速保存/加载 ====================
bool SimpleSaveManager::quickSave(entt::registry& registry) {
    entt::entity playerEntity = findPlayerEntity(registry);
    if (playerEntity == entt::null) {
        CCLOG("No player entity found for quick save");
        return false;
    }

    // 使用槽位1进行快速保存
    return saveGameWithEntt(1, registry, playerEntity);
}

bool SimpleSaveManager::quickLoad(entt::registry& registry) {
    entt::entity playerEntity = findPlayerEntity(registry);
    if (playerEntity == entt::null) {
        CCLOG("No player entity found for quick load");
        return false;
    }

    // 从槽位1快速加载
    return loadGameWithEntt(1, registry, playerEntity);
}