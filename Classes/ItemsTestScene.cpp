#include "ItemsTestScene.h"
#include "MainMenuScene.h"
#include "components/items/InventoryDef.h"
#include "systems/items/ItemManager.h"
#include "systems/items/Inventory.h"
#include "ui/items/InventoryLayer.h"
#include "ui/items/EquipmentPanel.h"
#include "systems/items/EnttTest.h"
#include "systems/items/RecipeManager.h"
#include "systems/items/StationDetector.h"
#include "systems/items/CraftingMatcher.h"
#include "systems/items/CraftingExecutor.h"
#include "ui/items/CraftBar.h"

USING_NS_CC;

Scene* ItemsTestScene::createScene() {
    auto scene = Scene::create();
    auto layer = ItemsTestScene::create();
    scene->addChild(layer);
    return scene;
}

bool ItemsTestScene::init() {
    if (!Layer::init()) {
        return false;
    }

    // Simple dark background
    auto bg = LayerColor::create(Color4B(25, 30, 45, 255));
    this->addChild(bg, -1);

    // Ensure item data is loaded and inventory ready
    auto mgr = ItemManager::getInstance();
    Inventory::getInstance()->init(); // default 59 slots (50+4 coin/ammo+1 trash+4 weapon)

    auto woodDef = mgr->getItemData(1001);
    auto stoneDef = mgr->getItemData(1003);
    if (!woodDef || !stoneDef) {
        CCLOG("ItemsTestScene: missing item definitions for wood(1001) or stone(1003)");
    }

    // 运行 EnTT 测试
    CCLOG("\n========================================");
    CCLOG("Running EnTT Integration Tests...");
    CCLOG("========================================\n");

    if (EnttTest::runBasicTest()) {
        CCLOG("\n✓ EnTT is properly configured and working!");
    } else {
        CCLOG("\n✗ EnTT test failed - please check configuration");
    }

    CCLOG("\n========================================\n");

    // 可选：运行性能测试
    // EnttTest::runPerformanceTest();

    // Auto-populate inventory with diverse items for testing
    auto inventory = Inventory::getInstance();
    CCLOG("\n=== Auto-populating inventory with diverse items ===");

    // Equipment (Type 1) - Copper and Iron armor sets
    inventory->addItem(1101, 1);   // Copper Helmet
    inventory->addItem(1102, 1);   // Copper Chestplate
    inventory->addItem(1103, 1);   // Copper Greaves
    inventory->addItem(1111, 1);   // Iron Helmet
    inventory->addItem(1112, 1);   // Iron Chestplate
    inventory->addItem(1113, 1);   // Iron Greaves

    // Placeables (Type 5) - Building blocks and machines
    inventory->addItem(2001, 150); // Dirt x150
    inventory->addItem(2002, 120); // Stone x120
    inventory->addItem(2007, 200); // Wood x200
    inventory->addItem(2009, 80);  // Sand x80
    inventory->addItem(2011, 50);  // Clay x50
    inventory->addItem(2013, 30);  // Glass x30
    inventory->addItem(2101, 2);   // Workbench x2
    inventory->addItem(2102, 1);   // Furnace x1
    inventory->addItem(2111, 3);   // Wooden Chest x3

    // Materials (Type 3) - Ores and ingots
    inventory->addItem(3001, 45);  // Copper Ore x45
    inventory->addItem(3002, 38);  // Iron Ore x38
    inventory->addItem(3003, 25);  // Silver Ore x25
    inventory->addItem(3004, 18);  // Gold Ore x18
    inventory->addItem(3015, 60);  // Coal x60
    inventory->addItem(3101, 20);  // Copper Ingot x20
    inventory->addItem(3102, 15);  // Iron Ingot x15

    // Materials - Goo and herbs
    inventory->addItem(3201, 12);  // Green Goo x12
    inventory->addItem(3202, 8);   // Blue Goo x8
    inventory->addItem(3203, 5);   // Red Goo x5
    inventory->addItem(3301, 10);  // Greenleaf x10
    inventory->addItem(3302, 7);   // Sunflower x7
    inventory->addItem(3305, 6);   // Frostleaf x6

    // Materials - Seeds
    inventory->addItem(3401, 15);  // Greenleaf Seeds x15
    inventory->addItem(3402, 12);  // Sunflower Seeds x12
    inventory->addItem(3405, 10);  // Frostleaf Seeds x10

    // Consumables (Type 4)
    inventory->addItem(4001, 5);   // Varnish x5
    inventory->addItem(3502, 100);
    inventory->addItem(3503, 100);
    inventory->addItem(3504, 100);
    inventory->addItem(3505, 100);
    inventory->addItem(3504, 100);
    inventory->addItem(2120, 200);

    CCLOG("Added 35+ diverse items across all categories for testing");
    CCLOG("Equipment: 6 armor pieces | Placeables: 9 types | Materials: 15 types | Consumables: 1 type");
    CCLOG("=====================================================\n");

    setupUI();
    setupInventoryLayer();
    setupStatusLabel();
    setupEventListener();
    setupGlobalMouseDebug();  // Add global mouse debug
    updateStatusText();

    // Test crafting system
    testCraftingSystem();

    // Add CraftBar UI (simplified version)
    auto craftBar = CraftBar::create();
    if (craftBar) {
        this->addChild(craftBar, 100);
        CCLOG("ItemsTestScene: CraftBar UI added");
    }

    CCLOG("ItemsTestScene: initialized.");
    return true;
}

void ItemsTestScene::setupUI() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    auto title = Label::createWithTTF("Items Test Scene", "fonts/Marker Felt.ttf", 32);
    if (title) {
        title->setPosition(Vec2(origin.x + visibleSize.width * 0.5f,
                                origin.y + visibleSize.height - title->getContentSize().height - 16));
        this->addChild(title, 1);
    }

    auto backLabel = Label::createWithTTF("Back to Menu", "fonts/Marker Felt.ttf", 24);
    auto backItem = MenuItemLabel::create(backLabel, CC_CALLBACK_1(ItemsTestScene::menuBackCallback, this));
    backItem->setPosition(Vec2(origin.x + visibleSize.width * 0.5f, origin.y + 40));

    auto menu = Menu::create(backItem, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);
}

void ItemsTestScene::setupInventoryLayer() {
    // InventoryLayer now self-positions at top-left 1/4 of screen
    auto invLayer = InventoryLayer::create();
    if (invLayer) {
        this->addChild(invLayer, 10);  // Higher z-order to receive mouse events
        CCLOG("ItemsTestScene: InventoryLayer added with z-order 10");
    }

    // Add equipment panel independently with same coordinate system
    auto equipPanel = EquipmentPanel::create();
    if (equipPanel) {
        this->addChild(equipPanel, 10);  // Same z-order as inventory
        CCLOG("ItemsTestScene: EquipmentPanel added with z-order 10");
    }
}

void ItemsTestScene::setupStatusLabel() {
    // Status label removed for cleaner UI
}

void ItemsTestScene::setupEventListener() {
    auto listener = EventListenerCustom::create("Event_InventoryChanged", [this](EventCustom*) {
        updateStatusText();
    });
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

void ItemsTestScene::setupGlobalMouseDebug() {
    auto globalMouseListener = EventListenerMouse::create();
    globalMouseListener->onMouseDown = [](EventMouse* event) {
        Vec2 pos = event->getLocation();
        CCLOG("=== GLOBAL MOUSE DEBUG ===");
        CCLOG("Mouse clicked at world position: (%.1f, %.1f)", pos.x, pos.y);
        CCLOG("Mouse button: %s", event->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT ? "LEFT" : "RIGHT");
    };

    // Add with fixed priority (lowest priority, will be called last)
    _eventDispatcher->addEventListenerWithFixedPriority(globalMouseListener, -1);
    CCLOG("ItemsTestScene: Global mouse debug listener added");
}

void ItemsTestScene::addWood() {
    Inventory::getInstance()->addItem(1001, 80); // Wood
    CCLOG("ItemsTestScene: add wood x80");
}

void ItemsTestScene::addStone() {
    Inventory::getInstance()->addItem(1003, 120); // Stone
    CCLOG("ItemsTestScene: add stone x120");
}

void ItemsTestScene::addOverflowBundle() {
    // Add many items to test stacking and overflow behavior
    Inventory::getInstance()->addItem(1001, 500); // Wood
    Inventory::getInstance()->addItem(1003, 500); // Stone
    Inventory::getInstance()->addItem(2001, 40);  // Seeds
    CCLOG("ItemsTestScene: add overflow bundle");
}

void ItemsTestScene::addOverflowWithReturn() {
    // Use addItemWithOverflow to see returned drops
    auto inv = Inventory::getInstance();
    auto dropped = inv->addItemWithOverflow(1001, 1200); // likely exceeds capacity
    for (const auto& slot : dropped) {
        CCLOG("ItemsTestScene: overflow returned item %d x%d", slot.itemId, slot.count);
    }
}

void ItemsTestScene::removeWood() {
    bool ok = Inventory::getInstance()->removeItem(1001, 50);
    CCLOG("ItemsTestScene: remove wood x50 -> %s", ok ? "ok" : "fail");
}

void ItemsTestScene::removeStone() {
    bool ok = Inventory::getInstance()->removeItem(1003, 50);
    CCLOG("ItemsTestScene: remove stone x50 -> %s", ok ? "ok" : "fail");
}

void ItemsTestScene::swapSlots01() {
    bool ok = Inventory::getInstance()->swapSlots(0, 1);
    CCLOG("ItemsTestScene: swap slot 0 <-> 1 -> %s", ok ? "ok" : "fail");
}

void ItemsTestScene::moveHalf01() {
    auto inv = Inventory::getInstance();
    const auto& slots = inv->getSlots();
    if (slots.empty()) {
        CCLOG("ItemsTestScene: moveHalf skipped, slots empty");
        return;
    }
    int count = slots[0].count;
    int half = count / 2;
    if (half <= 0) {
        CCLOG("ItemsTestScene: moveHalf skipped, slot0 empty");
        return;
    }
    bool ok = inv->moveItem(0, 1, half);
    CCLOG("ItemsTestScene: move half slot0->1 (%d) -> %s", half, ok ? "ok" : "fail");
}

void ItemsTestScene::clearInventory() {
    Inventory::getInstance()->clear();
    CCLOG("ItemsTestScene: inventory cleared");
}

void ItemsTestScene::updateStatusText() {
    // Status text update removed for cleaner UI
}

void ItemsTestScene::menuBackCallback(Ref* pSender) {
    auto mainMenuScene = MainMenuScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, mainMenuScene));
}

// ===== Crafting System Tests =====

void ItemsTestScene::testCraftingSystem() {
    CCLOG("\n=== Crafting System Test ===");

    // 1. Load recipes
    auto recipeMgr = RecipeManager::getInstance();
    CCLOG("Loaded %zu recipes", recipeMgr->getRecipeCount());

    // 2. Initialize station detector
    auto detector = StationDetector::getInstance();
    CCLOG("Current stations: %zu (should have Hand by default)", detector->getCurrentStations().size());

    // 3. Test coin recipes specifically
    CCLOG("\n--- Testing Coin Recipes ---");
    auto itemMgr = ItemManager::getInstance();

    // Check if coin items are loaded
    for (int coinId = 3502; coinId <= 3505; ++coinId) {
        auto coinDef = itemMgr->getItemData(coinId);
        if (coinDef) {
            CCLOG("Coin item loaded: ID=%d, Name=%s, MaxStack=%d",
                  coinId, coinDef->name.c_str(), coinDef->maxStack);
        } else {
            CCLOG("WARNING: Coin item %d not found!", coinId);
        }
    }

    // Check if coin recipes are loaded
    CCLOG("\n--- Checking Coin Recipes ---");
    for (int resultId = 3503; resultId <= 3505; ++resultId) {
        auto coinRecipes = recipeMgr->getRecipesByResultItem(resultId);
        if (!coinRecipes.empty()) {
            auto recipe = coinRecipes[0];
            auto resultDef = itemMgr->getItemData(recipe->resultItemId);
            CCLOG("Recipe found: %d x %s",
                  recipe->resultCount,
                  resultDef ? resultDef->name.c_str() : "Unknown");
            CCLOG("  Station: %s",
                  recipe->requiredStation == StationType::Hand ? "Hand" : "Other");
            for (const auto& ing : recipe->ingredients) {
                auto ingDef = itemMgr->getItemData(ing.itemId);
                CCLOG("  - Requires: %d x %s",
                      ing.count,
                      ingDef ? ingDef->name.c_str() : "Unknown");
            }
        } else {
            CCLOG("WARNING: No recipe found for result item %d", resultId);
        }
    }

    // 3. Query available recipes (Hand only)
    auto matcher = CraftingMatcher::getInstance();

    // Debug: Check all recipes before filtering
    auto allRecipes = recipeMgr->getAllRecipes();
    CCLOG("\n--- Debug: Total recipes in RecipeManager: %zu ---", allRecipes.size());
    for (const auto& recipe : allRecipes) {
        CCLOG("  Recipe: result=%d, station=%d",
              recipe.resultItemId,
              static_cast<int>(recipe.requiredStation));
    }

    // Debug: Check station detector
    auto currentStations = detector->getCurrentStations();
    CCLOG("\n--- Debug: Current stations: %zu ---", currentStations.size());
    for (auto station : currentStations) {
        CCLOG("  Station: %d", static_cast<int>(station));
    }

    auto recipes = matcher->getAvailableRecipes();
    CCLOG("\n--- Available Recipes (Hand only): %zu ---", recipes.size());

    // List all available hand recipes with coin recipes highlighted
    int coinRecipeCount = 0;
    for (const auto* recipe : recipes) {
        auto resultDef = itemMgr->getItemData(recipe->resultItemId);
        bool isCoinRecipe = (recipe->resultItemId >= 3503 && recipe->resultItemId <= 3505);

        if (isCoinRecipe) {
            coinRecipeCount++;
            CCLOG(">>> COIN RECIPE: %d x %s (ID: %d)",
                  recipe->resultCount,
                  resultDef ? resultDef->name.c_str() : "Unknown",
                  recipe->resultItemId);

            // Check if we can craft it
            bool canCraft = matcher->canCraft(*recipe);
            auto status = matcher->getRecipeStatus(*recipe);
            CCLOG("    Can craft: %s, Status: %d",
                  canCraft ? "YES" : "NO",
                  static_cast<int>(status));

            if (status == CraftingStatus::MissingMaterial) {
                auto missing = matcher->getMissingMaterials(*recipe);
                for (const auto& m : missing) {
                    auto matDef = itemMgr->getItemData(m.first);
                    CCLOG("    Missing: %s x%d",
                          matDef ? matDef->name.c_str() : std::to_string(m.first).c_str(),
                          m.second);
                }
            }
        }
    }

    CCLOG("\n>>> Total coin recipes available: %d / 3 expected", coinRecipeCount);
    CCLOG("=== Crafting System Ready ===\n");
}

void ItemsTestScene::addWorkbench() {
    StationDetector::getInstance()->addStation(StationType::Workbench);

    auto recipes = CraftingMatcher::getInstance()->getAvailableRecipes();
    CCLOG("Added Workbench. Available recipes: %zu", recipes.size());
}

void ItemsTestScene::removeWorkbench() {
    StationDetector::getInstance()->removeStation(StationType::Workbench);

    auto recipes = CraftingMatcher::getInstance()->getAvailableRecipes();
    CCLOG("Removed Workbench. Available recipes: %zu", recipes.size());
}

void ItemsTestScene::craftFirst() {
    auto recipes = CraftingMatcher::getInstance()->getAvailableRecipes();

    if (recipes.empty()) {
        CCLOG("No recipes available to craft!");
        return;
    }

    auto firstRecipe = recipes[0];
    auto itemDef = ItemManager::getInstance()->getItemData(firstRecipe->resultItemId);

    CCLOG("Attempting to craft: %s x%d",
          itemDef ? itemDef->name.c_str() : "Unknown",
          firstRecipe->resultCount);

    // Check if can craft
    bool canCraft = CraftingMatcher::getInstance()->canCraft(*firstRecipe);
    if (!canCraft) {
        auto status = CraftingMatcher::getInstance()->getRecipeStatus(*firstRecipe);
        std::string reason = "Unknown";
        if (status == CraftingStatus::MissingMaterial) {
            reason = "Missing materials";
            auto missing = CraftingMatcher::getInstance()->getMissingMaterials(*firstRecipe);
            for (const auto& m : missing) {
                auto matDef = ItemManager::getInstance()->getItemData(m.first);
                CCLOG("  Missing: %s x%d",
                      matDef ? matDef->name.c_str() : std::to_string(m.first).c_str(),
                      m.second);
            }
        } else if (status == CraftingStatus::MissingStation) {
            reason = "Missing station";
        } else if (status == CraftingStatus::InventoryFull) {
            reason = "Inventory full";
        }
        CCLOG("Cannot craft: %s", reason.c_str());
        return;
    }

    // Execute crafting
    bool success = CraftingExecutor::getInstance()->craft(*firstRecipe);
    CCLOG("Craft result: %s", success ? "SUCCESS!" : "FAILED");
}
