#ifndef __ITEMS_TEST_SCENE_H__
#define __ITEMS_TEST_SCENE_H__

#include "cocos2d.h"

// Lightweight items/inventory test scene:
// - Shows InventoryLayer UI
// - Menu buttons to add/remove/overflow/swap/move items
// - Status label with key counts and capacity info
class ItemsTestScene : public cocos2d::Layer {
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;

    void menuBackCallback(cocos2d::Ref* sender);

    CREATE_FUNC(ItemsTestScene);

private:
    cocos2d::Label* _statusLabel = nullptr;

    void setupUI();
    void setupInventoryLayer();
    void setupStatusLabel();
    void setupEventListener();
    void setupGlobalMouseDebug();

    void addWood();
    void addStone();
    void addOverflowBundle();
    void addOverflowWithReturn();
    void removeWood();
    void removeStone();
    void swapSlots01();
    void moveHalf01();
    void clearInventory();
    void updateStatusText();

    // Crafting system tests
    void testCraftingSystem();
    void addWorkbench();
    void removeWorkbench();
    void craftFirst();
};

#endif // __ITEMS_TEST_SCENE_H__
