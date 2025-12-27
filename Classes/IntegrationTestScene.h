#ifndef __INTEGRATION_TEST_SCENE_H__
#define __INTEGRATION_TEST_SCENE_H__

#include "cocos2d.h"
#include "entt/entt.hpp"

/**
 * @file IntegrationTestScene.h
 * @brief Items与Player系统整合测试场景
 *
 * 测试功能：
 * - 物品栏UI显示和交互
 * - 装备面板拖拽装备
 * - 快捷栏切换（数字键1-9）
 * - 物品使用（J键或鼠标左键）
 * - 玩家移动和跳跃
 * - 装备属性计算
 * - 方块系统集成（挖掘、放置）
 */
class IntegrationTestScene : public cocos2d::Layer {
public:
    IntegrationTestScene();
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    CREATE_FUNC(IntegrationTestScene);

private:
    entt::registry& _registry;
    entt::entity _playerEntity;

    // UI references
    class InventoryLayer* _inventoryLayer = nullptr;
    class EquipmentPanel* _equipmentPanel = nullptr;
    bool _inventoryVisible = false;

    // Block system manager
    class BlockSystemManager* _blockSystemManager = nullptr;

    // Setup methods
    void createPhysicsEnvironment();
    void createPlayer();
    void setupTestItems();
    void setupUI();
    void setupKeyboardListener();

    // Update methods
    void update(float dt) override;
    void toggleInventory();
};

#endif // __INTEGRATION_TEST_SCENE_H__
