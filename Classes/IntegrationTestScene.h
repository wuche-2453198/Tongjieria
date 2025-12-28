#ifndef __INTEGRATION_TEST_SCENE_H__
#define __INTEGRATION_TEST_SCENE_H__

#include "cocos2d.h"
#include "entt/entt.hpp"
#include <memory>

// 前置声明
class BlockSystemManager;
class CommandSystem;

/**
 * @file IntegrationTestScene.h
 * @brief Items与Player系统整合测试场景
 *
 * 测试功能：
 * - 物品栏UI显示和交互
 * - 装备面板拖拽装备
 * - 快捷栏切换（数字键1-9）
 * - 物品使用
 * - 玩家移动和跳跃
 * - 装备属性计算
 * - 方块系统集成
 * - 镐子挖矿（左键按住 + equipType为pickaxe的物品）
 */
class IntegrationTestScene : public cocos2d::Scene {
public:
    IntegrationTestScene();
    virtual ~IntegrationTestScene();
    static cocos2d::Scene* createScene();
    virtual bool init() override;

private:
    // ECS 系统 - 使用独立的 registry
    std::unique_ptr<entt::registry> _registry;
    std::unique_ptr<entt::dispatcher> _dispatcher;
    entt::entity _playerEntity;

    // UI references
    cocos2d::Node* _uiLayer = nullptr;  // UI layer fixed to camera (screen space)
    cocos2d::Camera* _uiCamera = nullptr;  // UI camera that follows default camera
    class InventoryLayer* _inventoryLayer = nullptr;
    class EquipmentPanel* _equipmentPanel = nullptr;
    bool _inventoryVisible = false;

    // Block system manager
    std::unique_ptr<BlockSystemManager> _blockSystemManager;

    // Rendering command system (for block rendering)
    CommandSystem* _renderingCommandsSystem = nullptr;

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
