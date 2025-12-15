#include "CraftingMatcher.h"
#include "RecipeManager.h"
#include "StationDetector.h"
#include "Inventory.h"
#include <algorithm>
#include <climits>

using namespace cocos2d;

CraftingMatcher* CraftingMatcher::_instance = nullptr;

CraftingMatcher* CraftingMatcher::getInstance() {
    if (!_instance) {
        _instance = new CraftingMatcher();
    }
    return _instance;
}

std::vector<const RecipeDefinition*> CraftingMatcher::getAvailableRecipes() {
    auto allRecipes = RecipeManager::getInstance()->getAllRecipes();
    std::vector<const RecipeDefinition*> result;

    // Step 1: Filter by environment
    for (const auto& recipe : allRecipes) {
        if (checkStation(recipe)) {
            result.push_back(&recipe);
        }
    }

    // Step 2: Sort by availability (craftable recipes first)
    std::sort(result.begin(), result.end(),
        [this](const RecipeDefinition* a, const RecipeDefinition* b) {
            bool canCraftA = this->canCraft(*a);
            bool canCraftB = this->canCraft(*b);
            if (canCraftA != canCraftB) return canCraftA > canCraftB;
            // Secondary sort: by result item ID
            return a->resultItemId < b->resultItemId;
        });

    return result;
}

bool CraftingMatcher::checkStation(const RecipeDefinition& recipe) {
    auto detector = StationDetector::getInstance();
    return detector->hasStation(recipe.requiredStation);
}

bool CraftingMatcher::checkMaterials(const RecipeDefinition& recipe) {
    auto inventory = Inventory::getInstance();

    for (const auto& ingredient : recipe.ingredients) {
        int owned = inventory->getItemCount(ingredient.itemId);
        if (owned < ingredient.count) {
            return false;
        }
    }
    return true;
}

bool CraftingMatcher::canCraft(const RecipeDefinition& recipe) {
    return checkStation(recipe) && checkMaterials(recipe);
}

int CraftingMatcher::getMaxCraftCount(const RecipeDefinition& recipe) {
    if (!checkStation(recipe)) return 0;

    auto inventory = Inventory::getInstance();
    int maxCount = INT_MAX;

    // Calculate maximum times based on each material
    for (const auto& ingredient : recipe.ingredients) {
        int owned = inventory->getItemCount(ingredient.itemId);
        int possible = owned / ingredient.count;
        maxCount = std::min(maxCount, possible);
    }

    return maxCount == INT_MAX ? 0 : maxCount;
}

CraftingStatus CraftingMatcher::getRecipeStatus(const RecipeDefinition& recipe) {
    if (!checkStation(recipe)) {
        return CraftingStatus::MissingStation;
    }
    if (!checkMaterials(recipe)) {
        return CraftingStatus::MissingMaterial;
    }
    // Check if inventory has space (simplified: just check for one empty slot)
    if (Inventory::getInstance()->getFirstEmptySlot() < 0) {
        return CraftingStatus::InventoryFull;
    }
    return CraftingStatus::Available;
}

std::vector<std::pair<int, int>> CraftingMatcher::getMissingMaterials(
    const RecipeDefinition& recipe) {
    std::vector<std::pair<int, int>> missing;
    auto inventory = Inventory::getInstance();

    for (const auto& ingredient : recipe.ingredients) {
        int owned = inventory->getItemCount(ingredient.itemId);
        if (owned < ingredient.count) {
            missing.push_back({ingredient.itemId, ingredient.count - owned});
        }
    }
    return missing;
}
