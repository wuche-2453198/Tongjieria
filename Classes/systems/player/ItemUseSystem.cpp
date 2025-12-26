#include "ItemUseSystem.h"
#include "components/player/PlayerComponents.h"
#include "systems/items/ItemManager.h"
#include "systems/items/Inventory.h"

USING_NS_CC;

bool ItemUseSystem::useItem(entt::registry& registry, entt::entity playerEntity, int itemId) {
    if (itemId == 0) {
        return false;
    }

    auto* itemMgr = ItemManager::getInstance();
    auto itemData = itemMgr->getItemData(itemId);

    if (!itemData) {
        CCLOG("ItemUseSystem::useItem: Item %d not found", itemId);
        return false;
    }

    // Determine item type and call appropriate handler
    switch (itemData->type) {
        case ItemType::Equipment:
            // Equipment items - could be weapons or tools
            // Check tags to determine if it's a weapon
            for (int tag : itemData->tags) {
                if (tag >= 201 && tag <= 203) {  // 201=Sword, 202=Axe, 203=Pick
                    if (tag == 201) {
                        return useWeapon(registry, playerEntity, itemId);
                    } else {
                        return useTool(registry, playerEntity, itemId);
                    }
                }
            }
            return false;

        case ItemType::Consumables:
            return useConsumable(registry, playerEntity, itemId);

        case ItemType::Placeables:
            // TODO: Need target position for placement
            CCLOG("ItemUseSystem::useItem: Placeable items need target position");
            return false;

        default:
            CCLOG("ItemUseSystem::useItem: Unknown item type for item %d", itemId);
            return false;
    }
}

bool ItemUseSystem::useWeapon(entt::registry& registry, entt::entity playerEntity, int itemId) {
    if (!registry.valid(playerEntity)) {
        return false;
    }

    auto* itemData = ItemManager::getInstance()->getItemData(itemId);
    if (!itemData) {
        return false;
    }

    auto& combat = registry.get<ecs::PlayerCombatComponent>(playerEntity);
    auto& stats = registry.get<ecs::PlayerStatsComponent>(playerEntity);

    // Check attack cooldown
    if (combat.isAttacking || combat.attackCooldown > 0.0f) {
        CCLOG("ItemUseSystem::useWeapon: Attack on cooldown");
        return false;
    }

    // Start attack
    combat.isAttacking = true;
    combat.attackCooldown = 0.5f;  // 0.5 second cooldown

    // Calculate damage (base damage + player stats)
    int damage = itemData->defense;  // TODO: Use proper damage field when added
    int totalDamage = static_cast<int>(damage * stats.meleeDamageBonus);

    CCLOG("ItemUseSystem::useWeapon: Using weapon %s (damage: %d)",
          itemData->name.c_str(), totalDamage);

    // TODO: Create attack hitbox
    // TODO: Check for enemies in range
    // TODO: Apply damage to enemies
    // TODO: Play attack animation

    return true;
}

bool ItemUseSystem::useConsumable(entt::registry& registry, entt::entity playerEntity, int itemId) {
    if (!registry.valid(playerEntity)) {
        return false;
    }

    auto* itemData = ItemManager::getInstance()->getItemData(itemId);
    if (!itemData) {
        return false;
    }

    auto& stats = registry.get<ecs::PlayerStatsComponent>(playerEntity);

    // TODO: Get heal values from item data when available
    // For now, use placeholder values based on item name
    int healHP = 0;
    int healMP = 0;

    if (itemData->name.find("Potion") != std::string::npos) {
        healHP = 50;  // Placeholder
    }

    // Apply healing
    if (healHP > 0) {
        stats.currentHealth = std::min(stats.currentHealth + static_cast<float>(healHP), stats.maxHealth);
        CCLOG("ItemUseSystem::useConsumable: Used %s, healed %d HP (now %.0f/%.0f)",
              itemData->name.c_str(), healHP, stats.currentHealth, stats.maxHealth);
    }

    if (healMP > 0) {
        stats.currentMana = std::min(stats.currentMana + static_cast<float>(healMP), stats.maxMana);
        CCLOG("ItemUseSystem::useConsumable: Restored %d MP (now %.0f/%.0f)",
              healMP, stats.currentMana, stats.maxMana);
    }

    // Consume the item from inventory
    auto* inventory = Inventory::getInstance();
    inventory->removeItem(itemId, 1);

    // TODO: Play use sound
    // TODO: Play use animation

    return true;
}

bool ItemUseSystem::useTool(entt::registry& registry, entt::entity playerEntity, int itemId) {
    if (!registry.valid(playerEntity)) {
        return false;
    }

    auto* itemData = ItemManager::getInstance()->getItemData(itemId);
    if (!itemData) {
        return false;
    }

    CCLOG("ItemUseSystem::useTool: Using tool %s", itemData->name.c_str());

    // TODO: Implement tool usage
    // - Axe: chop trees
    // - Pickaxe: mine blocks
    // - Check for target block in range
    // - Apply mining/chopping damage to block
    // - Drop resources when block breaks

    return true;
}

bool ItemUseSystem::placeBlock(entt::registry& registry, entt::entity playerEntity, int itemId, Vec2 worldPos) {
    if (!registry.valid(playerEntity)) {
        return false;
    }

    auto* itemData = ItemManager::getInstance()->getItemData(itemId);
    if (!itemData) {
        return false;
    }

    if (itemData->type != ItemType::Placeables) {
        CCLOG("ItemUseSystem::placeBlock: Item %d is not placeable", itemId);
        return false;
    }

    CCLOG("ItemUseSystem::placeBlock: Placing %s at (%.1f, %.1f)",
          itemData->name.c_str(), worldPos.x, worldPos.y);

    // TODO: Implement block placement
    // - Check if position is valid (not blocked)
    // - Check if player is in range
    // - Place block in world
    // - Consume item from inventory
    // - Play placement sound

    return true;
}
