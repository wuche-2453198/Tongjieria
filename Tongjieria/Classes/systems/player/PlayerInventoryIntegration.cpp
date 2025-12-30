#include "PlayerInventoryIntegration.h"
#include "systems/item/EquipmentValidator.h"
#include "systems/item/ItemManager.h"
#include "systems/player/ItemUseSystem.h"
#include "cocos2d.h"

USING_NS_CC;

namespace PlayerInventoryBridge {

// ==================== Equipment Management Implementation ====================

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
            return equipment.helmet; // Default: return helmet slot
    }
}

bool canEquipToSlot(int itemId, EquipmentSlotType slotType) {
    if (itemId == 0) {
        return false;
    }

    // Convert PlayerInventoryBridge::EquipmentSlotType to EquipSlotType
    EquipSlotType targetSlotType;
    switch (slotType) {
        case EquipmentSlotType::Helmet:
            targetSlotType = EquipSlotType::Helmet;
            break;
        case EquipmentSlotType::Chestplate:
            targetSlotType = EquipSlotType::Chestplate;
            break;
        case EquipmentSlotType::Leggings:
            targetSlotType = EquipSlotType::Leggings;
            break;
        case EquipmentSlotType::Accessory0:
        case EquipmentSlotType::Accessory1:
        case EquipmentSlotType::Accessory2:
        case EquipmentSlotType::Accessory3:
        case EquipmentSlotType::Accessory4:
        case EquipmentSlotType::Accessory5:
            targetSlotType = EquipSlotType::Accessory0; // All accessories use same validation
            break;
        default:
            return false;
    }

    // Use EquipmentValidator to check if item can be equipped
    auto* validator = EquipmentValidator::getInstance();
    return validator->canEquip(itemId, targetSlotType);
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

    // Check if inventory slot is valid
    if (inventorySlotIndex < 0 || inventorySlotIndex >= static_cast<int>(slots.size())) {
        CCLOG("equipItem: Invalid inventory slot index %d", inventorySlotIndex);
        return false;
    }

    auto& invSlot = slots[inventorySlotIndex];
    if (invSlot.itemId == 0) {
        CCLOG("equipItem: Inventory slot %d is empty", inventorySlotIndex);
        return false;
    }

    // Check if item can be equipped to this slot
    if (!canEquipToSlot(invSlot.itemId, slotType)) {
        CCLOG("equipItem: Item %d cannot be equipped to this slot", invSlot.itemId);
        return false;
    }

    // Get equipment slot
    auto& equipSlot = getEquipmentSlot(equipment, slotType);

    // If slot already has item, unequip first
    if (!equipSlot.isEmpty()) {
        CCLOG("equipItem: Slot already has item %d, unequipping first", equipSlot.itemId);
        unequipItem(registry, playerEntity, slotType);
    }

    // Equip item
    equipSlot.itemId = invSlot.itemId;
    equipSlot.prefixId = invSlot.prefixId;

    // Remove from inventory (equipment system is independent of inventory)
    // Note: Not removing from inventory for now, since Terraria uses independent equipment system
    // If removal needed, uncomment:
    // inventory->removeItem(invSlot.itemId, 1);

    CCLOG("equipItem: Equipped item %d to slot type %d", equipSlot.itemId, (int)slotType);

    // Recalculate stats
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

    // Try to put equipment back into inventory
    // This feature is not implemented yet
    auto* inventory = Inventory::getInstance();
    // Note: Not putting back into inventory for now, since Terraria uses independent equipment system
    // If needed, uncomment:
    // if (!inventory->addItem(equipSlot.itemId, 1)) {
    //     CCLOG("unequipItem: Inventory full, cannot unequip");
    //     return false;
    // }

    CCLOG("unequipItem: Unequipped item %d from slot type %d", equipSlot.itemId, (int)slotType);

    // Clear equipment slot
    equipSlot.clear();

    // Recalculate stats
    calculateEquipmentStats(registry, playerEntity);

    return true;
}

bool autoEquipItem(entt::registry& registry,
                   entt::entity playerEntity,
                   int inventorySlotIndex)
{
    // TODO: Automatically determine equipment slot based on item type
    // Need to get item info from ItemManager

    CCLOG("autoEquipItem: Auto-equip not fully implemented yet");
    return false;
}

// ==================== Hotbar Management Implementation ====================

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

    // Use ItemUseSystem to handle item usage
    return ItemUseSystem::useItem(registry, playerEntity, slot.itemId);
}

// ==================== Stats Calculation Implementation ====================

void calculateEquipmentStats(entt::registry& registry,
                             entt::entity playerEntity)
{
    if (!registry.valid(playerEntity)) {
        return;
    }

    auto& equipment = registry.get<ecs::PlayerEquipmentComponent>(playerEntity);
    auto& stats = registry.get<ecs::PlayerStatsComponent>(playerEntity);

    // Reset equipment bonuses (keep base values)
    int totalDefense = 0;
    float moveSpeedBonus = 1.0f;
    float meleeDamageBonus = 1.0f;
    float rangedDamageBonus = 1.0f;
    float magicDamageBonus = 1.0f;

    auto* itemMgr = ItemManager::getInstance();

    // Calculate armor defense from helmet
    if (!equipment.helmet.isEmpty()) {
        auto itemData = itemMgr->getItemData(equipment.helmet.itemId);
        if (itemData) {
            totalDefense += itemData->defense;
            CCLOG("  Helmet: %s (+%d defense)", itemData->name.c_str(), itemData->defense);
        }
    }

    // Calculate armor defense from chestplate
    if (!equipment.chestplate.isEmpty()) {
        auto itemData = itemMgr->getItemData(equipment.chestplate.itemId);
        if (itemData) {
            totalDefense += itemData->defense;
            CCLOG("  Chestplate: %s (+%d defense)", itemData->name.c_str(), itemData->defense);
        }
    }

    // Calculate armor defense from leggings
    if (!equipment.leggings.isEmpty()) {
        auto itemData = itemMgr->getItemData(equipment.leggings.itemId);
        if (itemData) {
            totalDefense += itemData->defense;
            CCLOG("  Leggings: %s (+%d defense)", itemData->name.c_str(), itemData->defense);
        }
    }

    // Calculate accessory bonuses
    for (int i = 0; i < ecs::PlayerEquipmentComponent::MAX_ACCESSORIES; i++) {
        if (!equipment.accessories[i].isEmpty()) {
            auto itemData = itemMgr->getItemData(equipment.accessories[i].itemId);
            if (itemData) {
                // Accessories can provide defense too
                totalDefense += itemData->defense;
                CCLOG("  Accessory %d: %s (+%d defense)", i, itemData->name.c_str(), itemData->defense);

                // TODO: Add other accessory bonuses when data is available
                // Example: moveSpeedBonus, damage bonuses, etc.
            }
        }
    }

    // Apply to player stats
    stats.defense = totalDefense;
    // stats.moveSpeed *= moveSpeedBonus;  // TODO: Apply when moveSpeed bonuses are added to items
    stats.meleeDamageBonus = meleeDamageBonus;
    stats.rangedDamageBonus = rangedDamageBonus;
    stats.magicDamageBonus = magicDamageBonus;

    CCLOG("calculateEquipmentStats: Total Defense=%d", stats.defense);
}

int checkArmorSet(entt::registry& registry,
                  entt::entity playerEntity)
{
    if (!registry.valid(playerEntity)) {
        return 0;
    }

    auto& equipment = registry.get<ecs::PlayerEquipmentComponent>(playerEntity);

    // TODO: Check armor set
    // E.g.: Copper set, Iron set, Gold set, etc.
    // Need to define set IDs and corresponding helmet, chestplate, leggings combinations

    // Example: Check if fully equipped with same type armor
    if (!equipment.helmet.isEmpty() &&
        !equipment.chestplate.isEmpty() &&
        !equipment.leggings.isEmpty()) {
        // TODO: Check if same armor set
        CCLOG("checkArmorSet: Player has full armor set");
        return 1; // Return set ID
    }

    return 0;
}

// ==================== Item Operation Implementation ====================

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
