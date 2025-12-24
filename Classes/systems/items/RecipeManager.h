#ifndef __RECIPE_MANAGER_H__
#define __RECIPE_MANAGER_H__

#include "components/items/CraftingDef.h"
#include "cocos2d.h"
#include <unordered_map>

// Recipe manager singleton
// Loads recipes from recipes.json and provides query interface
class RecipeManager {
public:
    static RecipeManager* getInstance();

    // Load recipes from JSON file
    bool loadRecipes(const std::string& filePath = "items/items_json/recipes.json");

    // Query interface
    const RecipeDefinition* getRecipeById(int recipeId) const;
    const std::vector<RecipeDefinition>& getAllRecipes() const;
    std::vector<const RecipeDefinition*> getRecipesByStation(StationType station) const;
    std::vector<const RecipeDefinition*> getRecipesByResultItem(int itemId) const;

    size_t getRecipeCount() const;

private:
    RecipeManager() = default;
    ~RecipeManager() = default;

    static RecipeManager* _instance;

    std::vector<RecipeDefinition> _recipes;
    std::unordered_map<int, size_t> _recipeIdToIndex; // Fast lookup by ID

    // Helper to parse station name from JSON
    StationType parseStationType(const std::string& stationName);
};

#endif // __RECIPE_MANAGER_H__
