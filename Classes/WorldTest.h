#ifndef __WORLD_TEST_H__
#define __WORLD_TEST_H__

#include "cocos2d.h"
#include "entt/entt.hpp"
#include <memory>

// 前置声明
class BlockSystemManager;
class CommandSystem;
/**
 * @file WorldTest.h
 * @brief 方块系统简单测试场景
 *
 * 用于测试方块系统的基本功能：
 * - 方块世界初始化
 * - 区块加载
 * - 方块渲染
 */
class WorldTest : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();

    WorldTest();  // 显式声明构造函数
    virtual ~WorldTest();  // 显式声明析构函数
    virtual bool init() override;

    void update(float delta) override;

private:
    bool initSystems();

    std::unique_ptr<entt::registry> _registry;
    std::unique_ptr<entt::dispatcher> _dispatcher;
    std::unique_ptr<BlockSystemManager> _blockSystemManager;
    CommandSystem* _renderingCommandsSystem = nullptr;                  ///< 渲染命令系统
};

#endif // __WORLD_TEST_H__
