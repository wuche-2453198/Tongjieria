#pragma once
#ifndef __SAVE_DATA_H__
#define __SAVE_DATA_H__

#include "cocos2d.h"
#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <entt/entt.hpp>
#include "components/ECS/ECSComponents.h"
#include <components/core/TransformComponent.h>
#include <components/player/PlayerComponents.h>

USING_NS_CC;

// ==================== 核心存档数据结构 ====================
struct SimpleSaveData {
    int slotNumber = 0;
    std::string saveName = "";
    time_t saveTime = 0;

    // 玩家核心数据
    std::string playerName = "Player";
    int playerLevel = 1;
    int playerExp = 0;
    ecs::TransformComponent playerPos;
    int playerHealth = 100;
    int playerMaxHealth = 100;
    int playerCoins = 0;
    int playerMana = 100;
    int playerMaxMana = 100;

    // 物品数据：物品ID -> 数量
    std::map<int, int> inventory;

    // 世界状态数据（扩展用）
    std::map<std::string, std::string> worldState;

    SimpleSaveData() = default;

    bool isEmpty() const {
        return slotNumber == 0 || saveTime == 0;
    }

    std::string getDisplayText() const {
        if (isEmpty()) return "Empty Slot " + std::to_string(slotNumber);

        char timeStr[64];
        struct tm* timeinfo = localtime(&saveTime);
        strftime(timeStr, sizeof(timeStr), "%m/%d %H:%M", timeinfo);

        return saveName + "\n" +
            std::string(timeStr) + "\n" +
            "Lv." + std::to_string(playerLevel) +
            " HP:" + std::to_string(playerHealth) + "/" + std::to_string(playerMaxHealth);
    }
};

// ==================== EnTT 序列化助手 ====================
class EnttSerializer {
public:
    // 从 EnTT 实体提取存档数据
    static SimpleSaveData extractFromEntity(entt::registry& registry, entt::entity playerEntity);

    // 将存档数据应用到 EnTT 实体
    static void applyToEntity(entt::registry& registry, entt::entity playerEntity,
        const SimpleSaveData& data);

    // 序列化 Transform 组件
    static std::string serializeTransform(const ECS::Transform& transform);
    static ECS::Transform deserializeTransform(const std::string& data);

    // 序列化 PlayerComponent 组件
    static std::string serializePlayerComponent(const ECS::PlayerComponent& player);
    static ECS::PlayerComponent deserializePlayerComponent(const std::string& data);

    // 序列化物品数据
    static std::string serializeInventory(const std::map<int, int>& inventory);
    static std::map<int, int> deserializeInventory(const std::string& data);
};

// ==================== 存档管理器（支持 EnTT）====================
class SimpleSaveManager {
public:
    static SimpleSaveManager* getInstance();

    // 基本存档操作
    bool saveGame(int slot, const SimpleSaveData& data);
    bool loadGame(int slot, SimpleSaveData& outData);
    bool deleteGame(int slot);
    std::vector<SimpleSaveData> getAllSaves();
    bool isSlotEmpty(int slot);

    // EnTT 整合存档操作
    bool saveGameWithEntt(int slot, entt::registry& registry, entt::entity playerEntity);
    bool loadGameWithEntt(int slot, entt::registry& registry, entt::entity playerEntity);

    // 快速保存/加载（自动查找玩家实体）
    bool quickSave(entt::registry& registry);
    bool quickLoad(entt::registry& registry);

private:
    SimpleSaveManager() = default;
    ~SimpleSaveManager() = default;
    static SimpleSaveManager* instance;

    std::string getSaveFilePath(int slot) const;
    std::string getEnttSaveFilePath(int slot) const;

    // 自动查找玩家实体
    entt::entity findPlayerEntity(entt::registry& registry);
};

#endif // __SAVE_DATA_H__