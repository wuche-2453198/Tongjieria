#include <memory>
#include "world.h"
#include "server/assets_manager.h"
#include "server/input_manager.h"
#include "block/block_layer.h"
#include "block/block_physics_layer.h"
#include "block/chunk_render.h"
#include "block/block_system.h"
#include "block/block_component.h"
#include "flyingcamera.h"

cocos2d::Scene* World::createScene() {
    auto world = new World();
    world->init();
    return world;
}

bool World::init() 
{
    if (!Scene::initWithPhysics())
    {
        return false;
    }
    
    scheduleUpdate();

    initServers();

    // 2. 创建飞行摄像机控制器
    auto flyCamera = FlyCamera2D::createWithTarget(getDefaultCamera());
    flyCamera->setSpeed(800.0f);  // 设置移动速度
    addChild(flyCamera);
    
    // 3. 启用/禁用控制
    flyCamera->setActive(true);  // 启用控制
    // flyCamera->setActive(false); // 禁用控制
    
    return true;
}

void World::update(float delta) 
{
    _blockSystemManager->update(delta);
}

bool World::initServers()
{
    // 1. 创建实体组件系统
    _registry = std::make_unique<entt::registry>();
    _dispatcher = std::make_unique<entt::dispatcher>();

    // 2. 创建各类服务
    _registry->ctx().emplace<WorldScene>(this);
    _registry->ctx().emplace<BlockLayer>(*_registry);
    _registry->ctx().emplace<BlockPhysicsLayer>();
    _registry->ctx().emplace<AssetManager>();
    _registry->ctx().emplace<BlockWorld>(*_registry, *_dispatcher);
    _registry->ctx().emplace<InputManager>(*_dispatcher).init(this);

    // 3. 创建方块系统管理器
    _blockSystemManager = std::make_unique<BlockSystemManager>(*_registry, *_dispatcher);
    
    // 4. 创建渲染命令系统
    _renderingCommandsSystem = new CommandSystem(*_registry, *_dispatcher);
    addChild(_renderingCommandsSystem);

    // 5. 创建调试工具
    tools::MouseDebugTool::init(this);

    return true;
}


