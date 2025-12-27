#include "WorldTest.h"
#include "core/block_world.h"
#include "core/assets_manager.h"
#include "components/block/block_component.h"
#include "systems/block/block_system_manager.h"
#include "systems/block_layer/block_physics_layer.h"
#include "systems\block\render_command_system.h"
USING_NS_CC;

Scene* WorldTest::createScene() {
    auto scene = new WorldTest();
    if (scene && scene->init()) {
        scene->autorelease();
        return scene;
    }
    CC_SAFE_DELETE(scene);
    return nullptr;
}

WorldTest::WorldTest() {
    // 构造函数在这里定义，此时 BlockSystemManager 的完整定义可见
}

WorldTest::~WorldTest() {
    // unique_ptr 会自动清理资源
    // 但需要在这里定义析构函数，因为 BlockSystemManager 的完整定义在这里可见
}

bool WorldTest::init() {
    if (!Scene::initWithPhysics()) {
        return false;
    }

    CCLOG("========================================");
    CCLOG("= ENTERING WORLD TEST SCENE");
    CCLOG("========================================");

    // Set background color
    auto background = LayerColor::create(Color4B(135, 206, 235, 255)); // Sky blue
    this->addChild(background, 0);

    // Initialize systems
    if (!initSystems()) {
        CCLOG("WorldTest: Failed to initialize systems");
        return false;
    }

    // Create a test entity with LoadingTicket to trigger chunk loading
    auto testEntity = _registry->create();
    _registry->emplace<Position>(testEntity, Vec2(400, 300));
    _registry->emplace<LoadingTicket>(
        testEntity,
        testEntity,  // entity_id
        3,           // radius: 3 chunks
        true         // permanent
    );
    CCLOG("WorldTest: Created test entity with LoadingTicket (radius: 3)");

    // Start update loop
    this->scheduleUpdate();

    CCLOG("WorldTest: Initialization complete");
    return true;
}

bool WorldTest::initSystems() {
    // 1. Create registry and dispatcher
    _registry = std::make_unique<entt::registry>();
    _dispatcher = std::make_unique<entt::dispatcher>();
    CCLOG("WorldTest: Registry and dispatcher created");

    // 2. Register context (following World scene pattern)
    _registry->ctx().emplace<WorldScene>(this);  // No World* for this test
    _registry->ctx().emplace<BlockPhysicsLayer>();
    _registry->ctx().emplace<AssetManager>();
    CCLOG("WorldTest: Context registered (WorldScene, AssetManager)");

    // 3. Create BlockWorld in context
    _registry->ctx().emplace<BlockWorld>(*_registry, *_dispatcher);
    CCLOG("WorldTest: BlockWorld created");

    // 4. Create BlockSystemManager
    _blockSystemManager = std::make_unique<BlockSystemManager>(*_registry, *_dispatcher);
    CCLOG("WorldTest: BlockSystemManager created");
    
    // 4. 创建渲染命令系统
      
     _renderingCommandsSystem = new CommandSystem(*_registry, *_dispatcher);
     addChild(_renderingCommandsSystem);
    return true;
}

void WorldTest::update(float delta) {
    Scene::update(delta);

    // Update block systems
    if (_blockSystemManager) {
        _blockSystemManager->update(delta);
    }
}
