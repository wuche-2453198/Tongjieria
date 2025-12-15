#include "CraftingExecutor.h"
#include "CraftingMatcher.h"
#include "Inventory.h"
#include "ItemManager.h"

using namespace cocos2d;

CraftingExecutor* CraftingExecutor::_instance = nullptr;

CraftingExecutor* CraftingExecutor::getInstance() {
    if (!_instance) {
        _instance = new CraftingExecutor();
    }
    return _instance;
}

bool CraftingExecutor::craft(const RecipeDefinition& recipe) {
    // Step 1: Second validation (prevent async issues)
    if (!CraftingMatcher::getInstance()->canCraft(recipe)) {
        CCLOG("CraftingExecutor: Cannot craft recipe %d", recipe.recipeId);
        return false;
    }

    auto inventory = Inventory::getInstance();

    // Step 2: Deduct materials (greedy algorithm)
    for (const auto& ingredient : recipe.ingredients) {
        bool success = inventory->removeItem(ingredient.itemId, ingredient.count);
        if (!success) {
            // This shouldn't happen (already validated)
            CCLOG("CraftingExecutor: Failed to remove item %d x%d",
                  ingredient.itemId, ingredient.count);
            // TODO: Rollback already deducted materials (optional implementation)
            return false;
        }
    }

    // Step 3: Add product (auto handle overflow)
    auto overflowItems = inventory->addItemWithOverflow(
        recipe.resultItemId,
        recipe.resultCount
    );

    // Step 4: Handle overflow (generate drop items)
    if (!overflowItems.empty()) {
        CCLOG("CraftingExecutor: Inventory full, %d items dropped",
              overflowItems[0].count);
        // TODO: Generate drop item entities at player position (future implementation)
        // Current simplification: log only
    }

    auto itemDef = ItemManager::getInstance()->getItemData(recipe.resultItemId);
    CCLOG("CraftingExecutor: Crafted %s x%d",
          itemDef ? itemDef->name.c_str() : "Unknown",
          recipe.resultCount);

    // Step 5: Event already dispatched by Inventory
    return true;
}

int CraftingExecutor::craftMultiple(const RecipeDefinition& recipe, int count) {
    int successCount = 0;

    for (int i = 0; i < count; ++i) {
        if (craft(recipe)) {
            successCount++;
        } else {
            break; // Not enough materials or inventory full, stop
        }
    }

    return successCount;
}

int CraftingExecutor::craftMax(const RecipeDefinition& recipe) {
    int maxCount = CraftingMatcher::getInstance()->getMaxCraftCount(recipe);
    return craftMultiple(recipe, maxCount);
}
