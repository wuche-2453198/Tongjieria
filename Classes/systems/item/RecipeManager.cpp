#include "RecipeManager.h"
#include "json/document.h"
#include "json/rapidjson.h"

using namespace cocos2d;

RecipeManager* RecipeManager::_instance = nullptr;

RecipeManager* RecipeManager::getInstance() {
    if (!_instance) {
        _instance = new RecipeManager();
        _instance->loadRecipes();
    }
    return _instance;
}

bool RecipeManager::loadRecipes(const std::string& filePath) {
    std::string content = FileUtils::getInstance()->getStringFromFile(filePath);
    if (content.empty()) {
        CCLOG("RecipeManager: Failed to load %s", filePath.c_str());
        return false;
    }

    rapidjson::Document doc;
    doc.Parse(content.c_str());
    if (doc.HasParseError() || !doc.HasMember("recipes")) {
        CCLOG("RecipeManager: JSON parse error in %s", filePath.c_str());
        return false;
    }

    const auto& recipesArray = doc["recipes"];
    if (!recipesArray.IsArray()) {
        CCLOG("RecipeManager: 'recipes' is not an array");
        return false;
    }

    int autoId = 1;

    for (const auto& recipeJson : recipesArray.GetArray()) {
        if (!recipeJson.IsObject()) continue;

        RecipeDefinition recipe;
        recipe.recipeId = autoId++;

        // Parse Result
        if (recipeJson.HasMember("Result") && recipeJson["Result"].IsObject()) {
            const auto& result = recipeJson["Result"];
            if (result.HasMember("ItemID")) {
                recipe.resultItemId = result["ItemID"].GetInt();
            }
            if (result.HasMember("Count")) {
                recipe.resultCount = result["Count"].GetInt();
            }
        }

        // Parse Station
        if (recipeJson.HasMember("Station") && recipeJson["Station"].IsString()) {
            std::string stationName = recipeJson["Station"].GetString();
            recipe.requiredStation = parseStationType(stationName);
        }

        // Parse Ingredients
        if (recipeJson.HasMember("Ingredients") && recipeJson["Ingredients"].IsArray()) {
            for (const auto& ing : recipeJson["Ingredients"].GetArray()) {
                if (!ing.IsObject()) continue;

                RecipeIngredient ingredient;
                if (ing.HasMember("ID")) {
                    ingredient.itemId = ing["ID"].GetInt();
                }
                if (ing.HasMember("Count")) {
                    ingredient.count = ing["Count"].GetInt();
                }

                if (ingredient.itemId != 0) {
                    recipe.ingredients.push_back(ingredient);
                }
            }
        }

        // Only add valid recipes
        if (recipe.resultItemId != 0 && !recipe.ingredients.empty()) {
            _recipes.push_back(recipe);
            _recipeIdToIndex[recipe.recipeId] = _recipes.size() - 1;
        }
    }

    CCLOG("RecipeManager: Loaded %zu recipes from %s", _recipes.size(), filePath.c_str());
    return true;
}

StationType RecipeManager::parseStationType(const std::string& name) {
    if (name == "Hand") return StationType::Hand;
    if (name == "Workbench") return StationType::Workbench;
    if (name == "Furnace") return StationType::Furnace;
    if (name == "Anvil") return StationType::Anvil;
    return StationType::Hand;
}

const RecipeDefinition* RecipeManager::getRecipeById(int recipeId) const {
    auto it = _recipeIdToIndex.find(recipeId);
    if (it != _recipeIdToIndex.end()) {
        return &_recipes[it->second];
    }
    return nullptr;
}

const std::vector<RecipeDefinition>& RecipeManager::getAllRecipes() const {
    return _recipes;
}

std::vector<const RecipeDefinition*> RecipeManager::getRecipesByStation(StationType station) const {
    std::vector<const RecipeDefinition*> result;
    for (const auto& recipe : _recipes) {
        if (recipe.requiredStation == station) {
            result.push_back(&recipe);
        }
    }
    return result;
}

std::vector<const RecipeDefinition*> RecipeManager::getRecipesByResultItem(int itemId) const {
    std::vector<const RecipeDefinition*> result;
    for (const auto& recipe : _recipes) {
        if (recipe.resultItemId == itemId) {
            result.push_back(&recipe);
        }
    }
    return result;
}

size_t RecipeManager::getRecipeCount() const {
    return _recipes.size();
}
