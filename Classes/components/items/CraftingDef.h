#ifndef __CRAFTING_DEF_H__
#define __CRAFTING_DEF_H__

#include <string>
#include <vector>

// Station type enum (corresponds to item tag system)
// tag=1 Workbench, tag=2 Furnace, tag=3 Anvil
enum class StationType {
    Hand = 0,        // Hand crafting (no station needed)
    Workbench = 1,   // Workbench (tag=1)
    Furnace = 2,     // Furnace (tag=2)
    Anvil = 3        // Anvil (tag=3)
};

// Recipe ingredient definition
struct RecipeIngredient {
    int itemId;      // Material item ID
    int count;       // Required count

    RecipeIngredient(int id = 0, int c = 1)
        : itemId(id), count(c) {}
};

// Recipe definition
struct RecipeDefinition {
    int recipeId;                           // Unique recipe ID
    int resultItemId;                       // Result item ID
    int resultCount;                        // Result count
    StationType requiredStation;            // Required crafting station
    std::vector<RecipeIngredient> ingredients; // List of ingredients

    RecipeDefinition()
        : recipeId(0)
        , resultItemId(0)
        , resultCount(1)
        , requiredStation(StationType::Hand) {}
};

// Crafting status enum
enum class CraftingStatus {
    Available,      // Can craft (has materials and station)
    MissingMaterial,// Missing materials
    MissingStation, // Missing required station
    InventoryFull   // Inventory is full
};

// Recipe category for UI filtering
enum class RecipeCategory {
    All,            // All recipes
    Tools,          // Tools (Equipment.Tools)
    Blocks,         // Blocks/Building (Placeables)
    Weapons,        // Weapons (Equipment.Weapons)
    Consumables,    // Consumables
    Materials       // Material processing
};

#endif // __CRAFTING_DEF_H__
