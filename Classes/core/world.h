#include <memory>
#include "cocos2d.h"
#include "entt/entt.hpp"
#pragma once

class AssetManager;
class BlockLayer;
class BlockWorld;
class BlockPhysicsLayer;
class ChunkRenderer;
class BlockSystemManager;
class InputManager;
class CommandSystem;

class World : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    World() = default;
    bool init();

    /**
    * @brief 每帧更新函数。
    * 
    * @param delta 距离上一帧的时间间隔，单位为秒。
    */
    void update(float delta) override;
private:
    bool initServers();
    std::unique_ptr<entt::registry> _registry = nullptr;                ///< 世界组件总线
    std::unique_ptr<entt::dispatcher> _dispatcher = nullptr;            ///< 世界事件总线
    std::unique_ptr<BlockSystemManager> _blockSystemManager = nullptr;  ///< 方块系统管理器
    CommandSystem* _renderingCommandsSystem = nullptr;                  ///< 渲染命令系统
};