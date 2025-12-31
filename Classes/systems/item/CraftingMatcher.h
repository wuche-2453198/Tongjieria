#ifndef __CRAFTING_MATCHER_H__
#define __CRAFTING_MATCHER_H__

#include "components/item/CraftingDef.h"
#include "cocos2d.h"
#include <vector>

// Crafting matcher singleton
// Filters craftable recipes based on current environment and inventory materials
class CraftingMatcher {
public:
    static CraftingMatcher* getInstance();

    // Get all available recipes (sorted by availability)
    std::vector<const RecipeDefinition*> getAvailableRecipes();

    // Check if single recipe can be crafted
    bool canCraft(const RecipeDefinition& recipe);

    // Get recipe crafting status
    CraftingStatus getRecipeStatus(const RecipeDefinition& recipe);

    // Calculate maximum craft count for recipe
    int getMaxCraftCount(const RecipeDefinition& recipe);

    // Get missing materials list (itemId, missingCount)
    std::vector<std::pair<int, int>> getMissingMaterials(const RecipeDefinition& recipe);

private:
    CraftingMatcher() = default;
    ~CraftingMatcher() = default;

    static CraftingMatcher* _instance;

    bool checkStation(const RecipeDefinition& recipe);
    bool checkMaterials(const RecipeDefinition& recipe);
};

#endif // __CRAFTING_MATCHER_H__
