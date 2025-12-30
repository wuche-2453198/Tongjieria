#ifndef __PLAYER_INVENTORY_INTEGRATION_H__
#define __PLAYER_INVENTORY_INTEGRATION_H__

#include "components/player/PlayerComponents.h"
#include "systems/item/Inventory.h"
#include "systems/item/ItemManager.h"
#include "entt/entt.hpp"

/**
 * @file PlayerInventoryIntegration.h
 * @brief 玩家系统与物品系统的整合接口
 *
 * 功能：
 * - 将全局 Inventory 与玩家实体关联
 * - 处理装备/卸下装备
 * - 处理快捷栏物品使用
 * - 计算装备属性加成
 */

namespace PlayerInventoryBridge {

// ==================== 装备管理 ====================

/**
 * @brief 装备物品到指定槽位
 * @param registry EnTT注册表
 * @param playerEntity 玩家实体
 * @param inventorySlotIndex 背包槽位索引
 * @param equipmentSlotType 装备槽位类型
 * @return 是否成功装备
 */
enum class EquipmentSlotType {
    Helmet,
    Chestplate,
    Leggings,
    Accessory0,
    Accessory1,
    Accessory2,
    Accessory3,
    Accessory4,
    Accessory5
};

bool equipItem(entt::registry& registry,
               entt::entity playerEntity,
               int inventorySlotIndex,
               EquipmentSlotType slotType);

/**
 * @brief 卸下装备到背包
 * @param registry EnTT注册表
 * @param playerEntity 玩家实体
 * @param slotType 装备槽位类型
 * @return 是否成功卸下
 */
bool unequipItem(entt::registry& registry,
                 entt::entity playerEntity,
                 EquipmentSlotType slotType);

/**
 * @brief 快速装备（自动检测物品类型并装备到对应槽位）
 * @param registry EnTT注册表
 * @param playerEntity 玩家实体
 * @param inventorySlotIndex 背包槽位索引
 * @return 是否成功装备
 */
bool autoEquipItem(entt::registry& registry,
                   entt::entity playerEntity,
                   int inventorySlotIndex);

// ==================== 快捷栏管理 ====================

/**
 * @brief 使用快捷栏当前选中的物品
 * @param registry EnTT注册表
 * @param playerEntity 玩家实体
 * @return 是否成功使用
 */
bool useCurrentHotbarItem(entt::registry& registry,
                          entt::entity playerEntity);

/**
 * @brief 使用指定快捷栏槽位的物品
 * @param registry EnTT注册表
 * @param playerEntity 玩家实体
 * @param hotbarIndex 快捷栏索引（0-9）
 * @return 是否成功使用
 */
bool useHotbarItem(entt::registry& registry,
                   entt::entity playerEntity,
                   int hotbarIndex);

/**
 * @brief 获取快捷栏当前选中物品的ID
 * @param registry EnTT注册表
 * @param playerEntity 玩家实体
 * @return 物品ID，如果为空返回0
 */
int getCurrentHotbarItemId(entt::registry& registry,
                           entt::entity playerEntity);

// ==================== 属性计算 ====================

/**
 * @brief 计算装备提供的总属性加成
 * @param registry EnTT注册表
 * @param playerEntity 玩家实体
 *
 * 会自动更新 PlayerStatsComponent 中的以下属性：
 * - defense (防御力)
 * - moveSpeed (移动速度)
 * - meleeDamageBonus (近战伤害加成)
 * - rangedDamageBonus (远程伤害加成)
 * - magicDamageBonus (魔法伤害加成)
 */
void calculateEquipmentStats(entt::registry& registry,
                             entt::entity playerEntity);

/**
 * @brief 检查是否拥有套装效果
 * @param registry EnTT注册表
 * @param playerEntity 玩家实体
 * @return 套装ID，如果没有套装返回0
 */
int checkArmorSet(entt::registry& registry,
                  entt::entity playerEntity);

// ==================== 物品操作 ====================

/**
 * @brief 消耗背包中的物品
 * @param itemId 物品ID
 * @param count 数量
 * @return 是否成功消耗
 */
bool consumeItem(int itemId, int count);

/**
 * @brief 给玩家添加物品到背包
 * @param itemId 物品ID
 * @param count 数量
 * @return 是否完全添加（false表示背包满溢出）
 */
bool giveItem(int itemId, int count);

/**
 * @brief 检查背包中是否有足够数量的物品
 * @param itemId 物品ID
 * @param count 数量
 * @return 是否有足够数量
 */
bool hasItem(int itemId, int count);

// ==================== 辅助函数 ====================

/**
 * @brief 获取装备槽位的引用
 */
ecs::PlayerEquipmentComponent::EquipmentSlot& getEquipmentSlot(
    ecs::PlayerEquipmentComponent& equipment,
    EquipmentSlotType slotType);

/**
 * @brief 判断物品是否可以装备到指定槽位
 */
bool canEquipToSlot(int itemId, EquipmentSlotType slotType);

} // namespace PlayerInventoryBridge

#endif // __PLAYER_INVENTORY_INTEGRATION_H__
