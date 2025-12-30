#ifndef __ITEM_MANAGER_TEST_H__
#define __ITEM_MANAGER_TEST_H__

#include "ItemManager.h"
#include "cocos2d.h"
#include <chrono>

class ItemManagerTest {
public:
    static bool runAllTests() {
        CCLOG("========================================");
        CCLOG("ItemManager EnTT Test Start");
        CCLOG("========================================");

        bool allPassed = true;

        allPassed &= testBasicLoading();
        allPassed &= testEntityQueries();
        allPassed &= testTypeQueries();
        allPassed &= testTagQueries();
        allPassed &= testCompatibility();
        allPassed &= testPerformance();

        CCLOG("========================================");
        if (allPassed) {
            CCLOG("All tests PASSED!");
        } else {
            CCLOG("Some tests FAILED");
        }
        CCLOG("========================================");

        return allPassed;
    }

private:
    static bool testBasicLoading() {
        CCLOG("\n--- Test 1: Basic Loading ---");
        auto mgr = ItemManager::getInstance();

        size_t count = mgr->getItemCount();
        CCLOG("Total items loaded: %zu", count);

        if (count > 0) {
            CCLOG("Test 1 PASSED");
            return true;
        } else {
            CCLOG("Test 1 FAILED: No items loaded");
            return false;
        }
    }

    static bool testEntityQueries() {
        CCLOG("\n--- Test 2: Entity Queries ---");
        auto mgr = ItemManager::getInstance();
        auto& registry = mgr->getRegistry();

        auto view = registry.view<ItemId, ItemName>();
        int entityCount = 0;

        CCLOG("First 5 item entities:");
        for (auto entity : view) {
            if (entityCount >= 5) break;

            const auto& idComp = view.get<ItemId>(entity);
            const auto& nameComp = view.get<ItemName>(entity);

            CCLOG("  Entity: ID=%d, Name=%s", idComp.value, nameComp.value.c_str());
            entityCount++;
        }

        if (entityCount > 0) {
            CCLOG("Test 2 PASSED");
            return true;
        } else {
            CCLOG("Test 2 FAILED");
            return false;
        }
    }

    static bool testTypeQueries() {
        CCLOG("\n--- Test 3: Type Queries ---");
        auto mgr = ItemManager::getInstance();

        const char* typeNames[] = {"Unknown", "Equipment", "Materials", "Placeables", "Consumables"};
        ItemType types[] = {ItemType::Unknown, ItemType::Equipment, ItemType::Materials,
                            ItemType::Placeables, ItemType::Consumables};

        bool hasAnyType = false;
        for (int i = 0; i < 5; i++) {
            auto items = mgr->getItemsByType(types[i]);
            CCLOG("  %s type: %zu items", typeNames[i], items.size());
            if (items.size() > 0) {
                hasAnyType = true;
                auto entity = items[0];
                auto& registry = mgr->getRegistry();
                if (auto* name = registry.try_get<ItemName>(entity)) {
                    if (auto* stack = registry.try_get<StackLimit>(entity)) {
                        CCLOG("    Example: %s (maxStack: %d)", name->value.c_str(), stack->maxStack);
                    }
                }
            }
        }

        if (hasAnyType) {
            CCLOG("Test 3 PASSED");
            return true;
        } else {
            CCLOG("Test 3 FAILED");
            return false;
        }
    }

    static bool testTagQueries() {
        CCLOG("\n--- Test 4: Tag Queries ---");
        auto mgr = ItemManager::getInstance();

        bool hasAnyTag = false;
        for (int tag = 1; tag <= 5; tag++) {
            auto items = mgr->getItemsByTag(tag);
            if (items.size() > 0) {
                CCLOG("  Tag %d: %zu items", tag, items.size());
                hasAnyTag = true;
            }
        }

        CCLOG("Test 4 PASSED (%s)", hasAnyTag ? "found tagged items" : "no tagged items (this is OK)");
        return true;
    }

    static bool testCompatibility() {
        CCLOG("\n--- Test 5: Compatibility Test ---");
        auto mgr = ItemManager::getInstance();

        const auto& allItems = mgr->getAllItems();
        CCLOG("getAllItems() returned: %zu items", allItems.size());

        if (allItems.size() > 0) {
            int testId = allItems[0].id;
            const auto* def = mgr->getItemData(testId);
            if (def) {
                CCLOG("getItemData(%d) succeeded:", testId);
                CCLOG("  Name: %s", def->name.c_str());
                CCLOG("  MaxStack: %d", def->maxStack);
                CCLOG("  Value: %d", def->value);
                CCLOG("Test 5 PASSED");
                return true;
            } else {
                CCLOG("Test 5 FAILED: getItemData returned nullptr");
                return false;
            }
        } else {
            CCLOG("Test 5 FAILED: getAllItems returned empty");
            return false;
        }
    }

    static bool testPerformance() {
        CCLOG("\n--- Test 6: Performance Test ---");
        auto mgr = ItemManager::getInstance();
        auto& registry = mgr->getRegistry();

        auto start = std::chrono::high_resolution_clock::now();

        int iterations = 1000;
        for (int i = 0; i < iterations; i++) {
            auto view = registry.view<ItemId, ItemName, ItemTypeComponent>();
            for (auto entity : view) {
                const auto& id = view.get<ItemId>(entity);
                const auto& name = view.get<ItemName>(entity);
                const auto& type = view.get<ItemTypeComponent>(entity);
                (void)id; (void)name; (void)type;
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        CCLOG("Executed %d full iterations in: %lld microseconds", iterations, duration);
        CCLOG("Average per iteration: %.2f microseconds", duration / (double)iterations);
        CCLOG("Test 6 PASSED");

        return true;
    }
};

#endif
