#ifndef __CRAFTING_EXECUTOR_H__
#define __CRAFTING_EXECUTOR_H__

#include "components/item/CraftingDef.h"
#include "cocos2d.h"

// Crafting executor singleton
// Executes crafting operations: deduct materials, add products, handle overflow
class CraftingExecutor {
public:
    static CraftingExecutor* getInstance();

    // Execute single craft
    bool craft(const RecipeDefinition& recipe);

    // Execute multiple crafts
    int craftMultiple(const RecipeDefinition& recipe, int count);

    // Maximize crafting (as many times as possible)
    int craftMax(const RecipeDefinition& recipe);

private:
    CraftingExecutor() = default;
    ~CraftingExecutor() = default;

    static CraftingExecutor* _instance;
};

#endif // __CRAFTING_EXECUTOR_H__
