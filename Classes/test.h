#pragma once

#include "cocos2d.h"
#include "systems/items/ItemManager.h"
#include "systems/items/Inventory.h"

using namespace cocos2d;

// Debug helpers for ItemManager / Inventory. Include this header only in a test .cpp and remove after use.
static void DebugItemManager() {
    auto mgr = ItemManager::getInstance();
    CCLOG("ItemManager loaded items: %zu", mgr->getAllItems().size());
    int testIds[] = {101, 102, 201};
    for (int id : testIds) {
        auto def = mgr->getItemData(id);
        CCLOG("[Item %d] found=%d name=%s maxStack=%d icon=%s",
              id, def != nullptr, def ? def->name.c_str() : "",
              def ? def->maxStack : -1, def ? def->iconPath.c_str() : "");
        auto frame = mgr->getItemSprite(id);
        CCLOG("[Item %d] sprite %s", id, frame ? "ok" : "null");
    }
}

static void DebugInventoryBasic() {
    auto inv = Inventory::getInstance();
    inv->init(10); // small capacity for easy observation
    CCLOG("Init slots=%zu", inv->getSlots().size());

    CCLOG("Add wood x120 (should split stacks)");
    auto ok = inv->addItem(101, 120);
    CCLOG("Add result=%d, count=%d", ok, inv->getItemCount(101));

    CCLOG("Remove wood x30 (should succeed)");
    CCLOG("Remove result=%d, count=%d", inv->removeItem(101, 30), inv->getItemCount(101));

    CCLOG("Add unknown id=0 (should reject)");
    CCLOG("Add unknown result=%d, count=%d", inv->addItem(0, 10), inv->getItemCount(0));
}

static void DebugInventoryMoveSwap() {
    auto inv = Inventory::getInstance();
    inv->init(6);
    inv->addItem(101, 80); // assume maxStack>=99
    inv->addItem(201, 1);

    // Log initial state
    auto& slots = inv->getSlots();
    CCLOG("Slots before: [0]=%d:%d [1]=%d:%d [2]=%d:%d",
          slots[0].itemId, slots[0].count, slots[1].itemId, slots[1].count, slots[2].itemId, slots[2].count);

    // Stack move
    inv->moveItem(0, 1, 30);
    CCLOG("After move 30 from 0->1: [0]=%d:%d [1]=%d:%d",
          slots[0].itemId, slots[0].count, slots[1].itemId, slots[1].count);

    // Swap different items
    inv->swapSlots(1, 2);
    CCLOG("After swap 1<->2: [1]=%d:%d [2]=%d:%d",
          slots[1].itemId, slots[1].count, slots[2].itemId, slots[2].count);
}

static void DebugInventoryEvent() {
    // Listen for inventory change events
    auto listener = EventListenerCustom::create("Event_InventoryChanged", [](EventCustom*) {
        CCLOG("Event_InventoryChanged fired");
    });
    Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(listener, 1);

    auto inv = Inventory::getInstance();
    inv->init(5);
    inv->addItem(101, 1);  // should trigger
    inv->removeItem(101, 1); // should trigger
}
