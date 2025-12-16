#ifndef __PLAYER_TEST_SCENE_H__
#define __PLAYER_TEST_SCENE_H__

#include "cocos2d.h"
// 注意：需要包含你的ECS World实现
// #include "ecs/World.h"

/**
 * @class PlayerTestScene
 * @brief 玩家系统测试场景 - 演示如何使用Player系统
 *
 * 功能：
 * - 创建玩家实体
 * - 初始化输入系统
 * - 测试基础移动和交互
 */
class PlayerTestScene : public cocos2d::Layer {
public:
    static cocos2d::Scene* createScene();

    virtual bool init();

    virtual void update(float delta) override;

    void menuBackCallback(cocos2d::Ref* pSender);

    CREATE_FUNC(PlayerTestScene);

private:
    // ECS世界（注意：根据你的ECS实现调整）
    // ecs::World _world;

    // 玩家实体ID
    // ecs::EntityId _playerEntity;

    // 调试标签
    cocos2d::Label* _debugLabel = nullptr;

    /**
     * @brief 创建物理环境（地面、平台）
     */
    void createPhysicsEnvironment();

    /**
     * @brief 创建调试UI
     */
    void createDebugUI();

    /**
     * @brief 更新调试信息显示
     */
    void updateDebugInfo(float dt);

    /**
     * @brief 测试输入系统
     */
    void testInputSystem();
};

#endif // __PLAYER_TEST_SCENE_H__
