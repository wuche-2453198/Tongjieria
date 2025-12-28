#include "PlayerTestScene.h"
#include "MainMenuScene.h"
#include "systems/player/PlayerFactory.h"
#include "components/player/PlayerComponents.h"
#include "core/PlayerInput.h"
#include "systems/player/PlayerSystems.h"

USING_NS_CC;

Scene* PlayerTestScene::createScene() {
    CCLOG("========================================");
    CCLOG("= ENTERING PLAYER TEST SCENE");
    CCLOG("========================================");

    // 创建带物理引擎的场景
    auto scene = Scene::createWithPhysics();

    // 设置重力
    auto physicsWorld = scene->getPhysicsWorld();
    physicsWorld->setGravity(Vec2(0, -980));

    // 启用物理调试绘制（可选）
    physicsWorld->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);

    auto layer = PlayerTestScene::create();
    scene->addChild(layer);

    CCLOG("PlayerTestScene: Scene created with physics");
    return scene;
}

bool PlayerTestScene::init() {
    if (!Layer::init()) {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 创建背景
    auto background = LayerColor::create(Color4B(135, 206, 235, 255)); // 天蓝色
    this->addChild(background, 0);

    // Create UI layer (fixed to screen) - COMMENTED OUT (not needed for this test scene)
    /*
    _uiLayer = Node::create();
    _uiLayer->setPosition(Vec2::ZERO);
    _uiLayer->setAnchorPoint(Vec2::ZERO);
    _uiLayer->setGlobalZOrder(1000);
    this->addChild(_uiLayer, 1000);

    // Create a separate camera for UI that doesn't move
    auto uiCamera = Camera::createOrthographic(
        visibleSize.width,
        visibleSize.height,
        -1024, 1024
    );
    uiCamera->setCameraFlag(CameraFlag::USER1);
    uiCamera->setDepth(10);  // Higher depth so it renders on top

    // Position UI camera at screen center (orthographic projection)
    uiCamera->setPosition(visibleSize.width / 2, visibleSize.height / 2);
    this->addChild(uiCamera);

    // Set UI layer to only be visible to the UI camera
    _uiLayer->setCameraMask((unsigned short)CameraFlag::USER1);

    CCLOG("PlayerTestScene: UI layer and UI camera created (fixed to screen)");
    */
    _uiLayer = nullptr;  // Not using UI layer in test scene

    // 添加标题
    auto titleLabel = Label::createWithTTF("Player System Test Scene",
                                          "fonts/Marker Felt.ttf", 32);
    if (titleLabel) {
        titleLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                    origin.y + visibleSize.height - 30));
        titleLabel->setColor(Color3B::BLACK);
        this->addChild(titleLabel, 1);
    }

    // 添加返回按钮
    auto backLabel = Label::createWithTTF("Back to Menu", "fonts/Marker Felt.ttf", 24);
    auto backItem = MenuItemLabel::create(
        backLabel, CC_CALLBACK_1(PlayerTestScene::menuBackCallback, this));

    if (backItem) {
        backItem->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + 50));
        auto menu = Menu::create(backItem, nullptr);
        menu->setPosition(Vec2::ZERO);
        this->addChild(menu, 1);
    }

    // 创建物理环境
    createPhysicsEnvironment();

    // 创建玩家
    createPlayer();

    // 创建调试UI
    createDebugUI();

    // 延迟初始化（确保场景已完全设置）
    this->scheduleOnce([this](float dt) {
        // 
        auto scene = this->getScene();
        if (scene) {
            PlayerInput::getInstance().initialize(scene);
            // 
            PlayerSystemsManager::setScene(scene);
            CCLOG("PlayerTestScene: Input system and scene reference initialized");
        } else {
            CCLOG("PlayerTestScene: ERROR - Scene is null!");
        }
    }, 0.1f, "init_input");

    // 启动更新
    this->scheduleUpdate();

    CCLOG("PlayerTestScene initialized");
    return true;
}

void PlayerTestScene::createPhysicsEnvironment() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    PhysicsMaterial material(1.0f, 0.0f, 1.0f); // 密度/恢复系数/摩擦力

    // ==================== 创建地面 ====================
    auto ground = Sprite::create();
    ground->setTextureRect(Rect(0, 0, visibleSize.width, 40));
    ground->setColor(Color3B(100, 150, 100)); // 草绿色
    ground->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + 20));
    this->addChild(ground, 0);

    auto groundBody = PhysicsBody::createBox(ground->getContentSize(), material);
    groundBody->setDynamic(false);
    groundBody->setContactTestBitmask(0xFFFFFFFF);
    ground->setPhysicsBody(groundBody);

    // ==================== 创建左墙 ====================
    auto leftWall = Sprite::create();
    leftWall->setTextureRect(Rect(0, 0, 10, visibleSize.height));
    leftWall->setColor(Color3B(80, 80, 80));
    leftWall->setPosition(Vec2(origin.x + 5, origin.y + visibleSize.height / 2));
    this->addChild(leftWall, 0);

    auto leftWallBody = PhysicsBody::createBox(leftWall->getContentSize(), material);
    leftWallBody->setDynamic(false);
    leftWallBody->setContactTestBitmask(0xFFFFFFFF);
    leftWall->setPhysicsBody(leftWallBody);

    // ==================== 创建右墙 ====================
    auto rightWall = Sprite::create();
    rightWall->setTextureRect(Rect(0, 0, 10, visibleSize.height));
    rightWall->setColor(Color3B(80, 80, 80));
    rightWall->setPosition(Vec2(origin.x + visibleSize.width - 5,
                               origin.y + visibleSize.height / 2));
    this->addChild(rightWall, 0);

    auto rightWallBody = PhysicsBody::createBox(rightWall->getContentSize(), material);
    rightWallBody->setDynamic(false);
    rightWallBody->setContactTestBitmask(0xFFFFFFFF);
    rightWall->setPhysicsBody(rightWallBody);

    // ==================== 创建平台1（中间偏左） ====================
    auto platform1 = Sprite::create();
    platform1->setTextureRect(Rect(0, 0, 200, 20));
    platform1->setColor(Color3B(139, 90, 43)); // 棕色
    platform1->setPosition(Vec2(origin.x + visibleSize.width / 2 - 180,
                               origin.y + visibleSize.height / 2 - 50));
    this->addChild(platform1, 0);

    auto platform1Body = PhysicsBody::createBox(platform1->getContentSize(), material);
    platform1Body->setDynamic(false);
    platform1Body->setContactTestBitmask(0xFFFFFFFF);
    platform1->setPhysicsBody(platform1Body);

    // ==================== 创建平台2（中间偏右） ====================
    auto platform2 = Sprite::create();
    platform2->setTextureRect(Rect(0, 0, 200, 20));
    platform2->setColor(Color3B(139, 90, 43)); // 棕色
    platform2->setPosition(Vec2(origin.x + visibleSize.width / 2 + 180,
                               origin.y + visibleSize.height / 2));
    this->addChild(platform2, 0);

    auto platform2Body = PhysicsBody::createBox(platform2->getContentSize(), material);
    platform2Body->setDynamic(false);
    platform2Body->setContactTestBitmask(0xFFFFFFFF);
    platform2->setPhysicsBody(platform2Body);

    // ==================== 创建平台3（高处） ====================
    auto platform3 = Sprite::create();
    platform3->setTextureRect(Rect(0, 0, 150, 20));
    platform3->setColor(Color3B(139, 90, 43)); // 棕色
    platform3->setPosition(Vec2(origin.x + visibleSize.width / 2,
                               origin.y + visibleSize.height - 150));
    this->addChild(platform3, 0);

    auto platform3Body = PhysicsBody::createBox(platform3->getContentSize(), material);
    platform3Body->setDynamic(false);
    platform3Body->setContactTestBitmask(0xFFFFFFFF);
    platform3->setPhysicsBody(platform3Body);

    CCLOG("Physics environment created");
}

void PlayerTestScene::createPlayer() {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 使用 PlayerFactory 创建玩家
    Vec2 spawnPos(visibleSize.width / 2, 300);
    auto scene = this->getScene();
    _playerEntity = PlayerFactory::createPlayer(_registry, spawnPos, scene, _uiLayer);

    CCLOG("Player created in test scene!");
}

void PlayerTestScene::createDebugUI() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 创建调试信息标签
    _debugLabel = Label::createWithSystemFont("", "Arial", 14);
    _debugLabel->setAnchorPoint(Vec2(0, 1));
    _debugLabel->setPosition(Vec2(origin.x + 10, origin.y + visibleSize.height - 90));
    _debugLabel->setColor(Color3B::BLACK);
    this->addChild(_debugLabel, 100);

    // 创建控制提示标签
    _controlsLabel = Label::createWithSystemFont(
        "Controls:\n"
        "A/D - Move Left/Right\n"
        "SPACE - Jump\n"
        "1-9 - Switch Hotbar Slot\n"
        "Mouse Wheel - Switch Hotbar",
        "Arial", 14
    );
    _controlsLabel->setAnchorPoint(Vec2(1, 1));
    _controlsLabel->setPosition(Vec2(origin.x + visibleSize.width - 10,
                                    origin.y + visibleSize.height - 90));
    _controlsLabel->setColor(Color3B::BLACK);
    this->addChild(_controlsLabel, 100);

    CCLOG("Debug UI created");
}

void PlayerTestScene::update(float delta) {
    // 1. 使用 PlayerSystemsManager 更新所有系统（先读取输入状态）
    PlayerSystemsManager::updateAllSystems(_registry, delta);

    // 2. 更新调试信息
    updateDebugInfo(delta);

    // 3. 更新输入状态（重置 justPressed 状态，准备下一帧）
    PlayerInput::getInstance().update(delta);
}

void PlayerTestScene::detectGround() {
    // 该方法已被 PlayerGroundDetectionSystem 接管
    // 保留此函数以避免编译错误，但实际不再使用
}

void PlayerTestScene::updatePlayer(float dt) {
    // 该方法已被 PlayerInputSystem 和 PlayerMovementSystem 接管
    // 保留此函数以避免编译错误，但实际不再使用
}

void PlayerTestScene::updatePlayerUI(float dt) {
    // 该方法已被 PlayerHealthSystem 接管
    // 保留此函数以避免编译错误，但实际不再使用
}

void PlayerTestScene::updateDebugInfo(float dt) {
    if (!_debugLabel) return;

    auto view = _registry.view<ecs::PlayerTag,
                                ecs::PlayerStatsComponent,
                                ecs::PlayerMovementComponent,
                                ecs::TransformComponent,
                                ecs::PlayerSpriteComponent>();

    for (auto entity : view) {
        auto [stats, movement, transform, sprite] =
            view.get<ecs::PlayerStatsComponent,
                     ecs::PlayerMovementComponent,
                     ecs::TransformComponent,
                     ecs::PlayerSpriteComponent>(entity);

        float actualVelY = 0.0f;
        if (sprite.sprite && sprite.sprite->getPhysicsBody()) {
            actualVelY = sprite.sprite->getPhysicsBody()->getVelocity().y;
        }

        std::string info = StringUtils::format(
            "FPS: %.1f\n"
            "Position: (%.0f, %.0f)\n"
            "Velocity: (%.1f, %.1f)\n"
            "HP: %.0f/%.0f  MP: %.0f/%.0f\n"
            "Defense: %d  Speed: %.0f\n"
            "On Ground: %s\n"
            "Facing: %s",
            1.0f / dt,
            transform.x, transform.y,
            movement.velocity.x, actualVelY,
            stats.currentHealth, stats.maxHealth,
            stats.currentMana, stats.maxMana,
            stats.defense, stats.moveSpeed,
            stats.isOnGround ? "Yes" : "No",
            movement.isFacingRight ? "Right" : "Left"
        );

        _debugLabel->setString(info);
    }
}

void PlayerTestScene::menuBackCallback(Ref* pSender) {
    CCLOG("Returning to main menu");

    // 清理输入系统
    PlayerInput::getInstance().cleanup();

    auto mainMenuScene = MainMenuScene::createScene();
    Director::getInstance()->replaceScene(
        TransitionFade::create(0.5f, mainMenuScene)
    );
}
