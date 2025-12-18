#include "PlayerInventoryIntegration.h"
#include "cocos2d.h"

USING_NS_CC;

namespace PlayerInventoryBridge {

// ==================== 装备管理实现 ====================

ecs::PlayerEquipmentComponent::EquipmentSlot& getEquipmentSlot(
    ecs::PlayerEquipmentComponent& equipment,
    EquipmentSlotType slotType)
{
    switch (slotType) {
        case EquipmentSlotType::Helmet:
            return equipment.helmet;
        case EquipmentSlotType::Chestplate:
            return equipment.chestplate;
        case EquipmentSlotType::Leggings:
            return equipment.leggings;
        case EquipmentSlotType::Accessory0:
            return equipment.accessories[0];
        case EquipmentSlotType::Accessory1:
            return equipment.accessories[1];
        case EquipmentSlotType::Accessory2:
            return equipment.accessories[2];
        case EquipmentSlotType::Accessory3:
            return equipment.accessories[3];
        case EquipmentSlotType::Accessory4:
            return equipment.accessories[4];
        case EquipmentSlotType::Accessory5:
            return equipment.accessories[5];
        default:
            return equipment.helmet; // 默认返回头盔槽
    }
}

bool canEquipToSlot(int itemId, EquipmentSlotType slotType) {
    // TODO: 根据物品类型判断是否可以装备到指定槽位
    // 需要从 ItemManager 获取物品信息
    // 目前简单返回 true，后续需要实现物品类型检查
    return itemId != 0;
}

bool equipItem(entt::registry& registry,
               entt::entity playerEntity,
               int inventorySlotIndex,
               EquipmentSlotType slotType)
{
    if (!registry.valid(playerEntity)) {
        CCLOG("equipItem: Invalid player entity");
        return false;
    }

    auto& equipment = registry.get<ecs::PlayerEquipmentComponent>(playerEntity);
    auto* inventory = Inventory::getInstance();
    const auto& slots = inventory->getSlots();

    // 检查背包槽位是否有效
    if (inventorySlotIndex < 0 || inventorySlotIndex >= static_cast<int>(slots.size())) {
        CCLOG("equipItem: Invalid inventory slot index %d", inventorySlotIndex);
        return false;
    }

    auto& invSlot = slots[inventorySlotIndex];
    if (invSlot.itemId == 0) {
        CCLOG("equipItem: Inventory slot %d is empty", inventorySlotIndex);
        return false;
    }

    // 检查是否可以装备到该槽位
    if (!canEquipToSlot(invSlot.itemId, slotType)) {
        CCLOG("equipItem: Item %d cannot be equipped to this slot", invSlot.itemId);
        return false;
    }

    // 获取装备槽位
    auto& equipSlot = getEquipmentSlot(equipment, slotType);

    // 如果装备槽已有物品，先卸下
    if (!equipSlot.isEmpty()) {
        CCLOG("equipItem: Slot already has item %d, unequipping first", equipSlot.itemId);
        unequipItem(registry, playerEntity, slotType);
    }

    // 装备物品
    equipSlot.itemId = invSlot.itemId;
    equipSlot.prefixId = invSlot.prefixId;

    // 从背包移除（装备系统独立于背包）
    // 注意：这里暂时不从背包移除，因为泰拉瑞亚的装备是独立系统
    // 如果需要移除，取消注释：
    // inventory->removeItem(invSlot.itemId, 1);

    CCLOG("equipItem: Equipped item %d to slot type %d", equipSlot.itemId, (int)slotType);

    // 重新计算属性
    calculateEquipmentStats(registry, playerEntity);

    return true;
}

bool unequipItem(entt::registry& registry,
                 entt::entity playerEntity,
                 EquipmentSlotType slotType)
{
    if (!registry.valid(playerEntity)) {
        return false;
    }

    auto& equipment = registry.get<ecs::PlayerEquipmentComponent>(playerEntity);
    auto& equipSlot = getEquipmentSlot(equipment, slotType);

    if (equipSlot.isEmpty()) {
        CCLOG("unequipItem: Slot is already empty");
        return false;
    }

    // 尝试将装备放回背包
    auto* inventory = Inventory::getInstance();
    // 注意：这里暂时不放回背包，因为泰拉瑞亚的装备是独立系统
    // 如果需要放回，取消注释：
    // if (!inventory->addItem(equipSlot.itemId, 1)) {
    //     CCLOG("unequipItem: Inventory full, cannot unequip");
    //     return false;
    // }

    CCLOG("unequipItem: Unequipped item %d from slot type %d", equipSlot.itemId, (int)slotType);

    // 清空装备槽
    equipSlot.clear();

    // 重新计算属性
    calculateEquipmentStats(registry, playerEntity);

    return true;
}

bool autoEquipItem(entt::registry& registry,
                   entt::entity playerEntity,
                   int inventorySlotIndex)
{
    // TODO: 根据物品类型自动判断装备槽位
    // 需要从 ItemManager 获取物品信息

    CCLOG("autoEquipItem: Auto-equip not fully implemented yet");
    return false;
}

// ==================== 快捷栏管理实现 ====================

int getCurrentHotbarItemId(entt::registry& registry,
                           entt::entity playerEntity)
{
    if (!registry.valid(playerEntity)) {
        return 0;
    }

    auto& hotbar = registry.get<ecs::PlayerHotbarComponent>(playerEntity);
    int invIndex = hotbar.getCurrentInventoryIndex();

    auto* inventory = Inventory::getInstance();
    const auto& slots = inventory->getSlots();

    if (invIndex >= 0 && invIndex < static_cast<int>(slots.size())) {
        return slots[invIndex].itemId;
    }

    return 0;
}

bool useCurrentHotbarItem(entt::registry& registry,
                          entt::entity playerEntity)
{
    if (!registry.valid(playerEntity)) {
        return false;
    }

    auto& hotbar = registry.get<ecs::PlayerHotbarComponent>(playerEntity);
    return useHotbarItem(registry, playerEntity, hotbar.selectedIndex);
}

bool useHotbarItem(entt::registry& registry,
                   entt::entity playerEntity,
                   int hotbarIndex)
{
    if (!registry.valid(playerEntity)) {
        return false;
    }

    auto& hotbar = registry.get<ecs::PlayerHotbarComponent>(playerEntity);
    if (hotbarIndex < 0 || hotbarIndex >= ecs::PlayerHotbarComponent::HOTBAR_SIZE) {
        return false;
    }

    int invIndex = hotbar.slots[hotbarIndex];
    auto* inventory = Inventory::getInstance();
    const auto& slots = inventory->getSlots();

    if (invIndex < 0 || invIndex >= static_cast<int>(slots.size())) {
        return false;
    }

    auto& slot = slots[invIndex];
    if (slot.itemId == 0) {
        CCLOG("useHotbarItem: Hotbar slot %d is empty", hotbarIndex);
        return false;
    }

    CCLOG("useHotbarItem: Using item %d from hotbar slot %d", slot.itemId, hotbarIndex);

    // TODO: 根据物品类型执行不同的使用逻辑
    // - 消耗品：消耗并应用效果
    // - 武器：发起攻击
    // - 工具：开始使用工具
    // - 方块：准备放置

    return true;
}

// ==================== 属性计算实现 ====================

void calculateEquipmentStats(entt::registry& registry,
                             entt::entity playerEntity)
{
    if (!registry.valid(playerEntity)) {
        return;
    }

    auto& equipment = registry.get<ecs::PlayerEquipmentComponent>(playerEntity);
    auto& stats = registry.get<ecs::PlayerStatsComponent>(playerEntity);

    // 重置装备加成（保留基础值）
    int totalDefense = 0;
    float moveSpeedBonus = 1.0f;
    float meleeDamageBonus = 1.0f;
    float rangedDamageBonus = 1.0f;
    float magicDamageBonus = 1.0f;

    // TODO: 从 ItemManager 获取每件装备的属性
    // 这里需要实现装备属性数据库

    // 计算护甲防御
    if (!equipment.helmet.isEmpty()) {
        // totalDefense += getItemDefense(equipment.helmet.itemId);
        totalDefense += 2; // 临时值
    }
    if (!equipment.chestplate.isEmpty()) {
        totalDefense += 3; // 临时值
    }
    if (!equipment.leggings.isEmpty()) {
        totalDefense += 2; // 临时值
    }

    // 计算饰品加成
    for (int i = 0; i < ecs::PlayerEquipmentComponent::MAX_ACCESSORIES; i++) {
        if (!equipment.accessories[i].isEmpty()) {
            // TODO: 获取饰品属性
            // 例如：速度饰品、伤害饰品等
        }
    }

    // 应用到玩家属性
    stats.defense = totalDefense;
    // stats.moveSpeed *= moveSpeedBonus;
    stats.meleeDamageBonus = meleeDamageBonus;
    stats.rangedDamageBonus = rangedDamageBonus;
    stats.magicDamageBonus = magicDamageBonus;

    CCLOG("calculateEquipmentStats: Defense=%d", stats.defense);
}

int checkArmorSet(entt::registry& registry,
                  entt::entity playerEntity)
{
    if (!registry.valid(playerEntity)) {
        return 0;
    }

    auto& equipment = registry.get<ecs::PlayerEquipmentComponent>(playerEntity);

    // TODO: 检查套装
    // 例如：铜套装、铁套装、金套装等
    // 需要定义套装ID和对应的头盔、胸甲、护腿组合

    // 示例：检查是否全部装备了同类型护甲
    if (!equipment.helmet.isEmpty() &&
        !equipment.chestplate.isEmpty() &&
        !equipment.leggings.isEmpty()) {
        // TODO: 检查是否是同一套装
        CCLOG("checkArmorSet: Player has full armor set");
        return 1; // 返回套装ID
    }

    return 0;
}

// ==================== 物品操作实现 ====================

bool consumeItem(int itemId, int count) {
    auto* inventory = Inventory::getInstance();
    bool success = inventory->removeItem(itemId, count);

    if (success) {
        CCLOG("consumeItem: Consumed %d x %d", count, itemId);
    } else {
        CCLOG("consumeItem: Failed to consume %d x %d (not enough)", count, itemId);
    }

    return success;
}

bool giveItem(int itemId, int count) {
    auto* inventory = Inventory::getInstance();
    bool success = inventory->addItem(itemId, count);

    if (success) {
        CCLOG("giveItem: Gave %d x %d", count, itemId);
    } else {
        CCLOG("giveItem: Inventory full, overflow for %d x %d", count, itemId);
    }

    return success;
}

bool hasItem(int itemId, int count) {
    auto* inventory = Inventory::getInstance();
    int actualCount = inventory->getItemCount(itemId);
    return actualCount >= count;
}

} // namespace PlayerInventoryBridge
