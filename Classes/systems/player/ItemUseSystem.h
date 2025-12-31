#ifndef __ITEM_USE_SYSTEM_H__
#define __ITEM_USE_SYSTEM_H__

#include "entt/entt.hpp"
#include "cocos2d.h"

/**
 * @file ItemUseSystem.h
 * @brief 物品使用系统
 *
 * 功能：
 * - 处理武器使用（攻击）
 * - 处理消耗品使用（药水、食物）
 * - 处理工具使用（挖掘、砍树）
 * - 处理方块放置
 */
class ItemUseSystem {
public:
    /**
     * @brief 使用物品（自动判断类型）
     * @param registry EnTT注册表
     * @param playerEntity 玩家实体
     * @param itemId 物品ID
     * @return 是否成功使用
     */
    static bool useItem(entt::registry& registry, entt::entity playerEntity, int itemId);

    /**
     * @brief 使用武器（攻击）
     * @param registry EnTT注册表
     * @param playerEntity 玩家实体
     * @param itemId 武器ID
     * @return 是否成功使用
     */
    static bool useWeapon(entt::registry& registry, entt::entity playerEntity, int itemId);

    /**
     * @brief 使用消耗品（药水、食物）
     * @param registry EnTT注册表
     * @param playerEntity 玩家实体
     * @param itemId 消耗品ID
     * @return 是否成功使用
     */
    static bool useConsumable(entt::registry& registry, entt::entity playerEntity, int itemId);

    /**
     * @brief 使用工具（挖掘、砍树）
     * @param registry EnTT注册表
     * @param playerEntity 玩家实体
     * @param itemId 工具ID
     * @return 是否成功使用
     */
    static bool useTool(entt::registry& registry, entt::entity playerEntity, int itemId);

    /**
     * @brief 放置方块
     * @param registry EnTT注册表
     * @param playerEntity 玩家实体
     * @param itemId 方块ID
     * @param worldPos 世界坐标
     * @return 是否成功放置
     */
    static bool placeBlock(entt::registry& registry, entt::entity playerEntity, int itemId, cocos2d::Vec2 worldPos);

    /**
     * @brief 根据物品ID获取对应的方块类型名称
     * @param itemId 物品ID
     * @return 方块类型名称（用于entt::hashed_string）
     */
    static const char* getBlockTypeFromItemId(int itemId);

private:
    ItemUseSystem() = default;
};

#endif // __ITEM_USE_SYSTEM_H__
