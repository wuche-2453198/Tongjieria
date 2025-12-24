#include "PlayerTestScene_Example.h"
#include "core/PlayerInput.h"
#include "PlayerFactory.h"
#include "components/player/PlayerComponents.h"
#include "MainMenuScene.h"
//模版演示文件
USING_NS_CC;

Scene* PlayerTestScene::createScene() {
    // 创建带物理引擎的场景
    auto scene = Scene::createWithPhysics();

    // 设置重力
    auto physicsWorld = scene->getPhysicsWorld();
    physicsWorld->setGravity(Vec2(0, -980));

    // 启用物理调试绘制（可选）
    physicsWorld->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);

    auto layer = PlayerTestScene::create();
    scene->addChild(layer);
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

    // 添加标题
    auto titleLabel = Label::createWithTTF("Player System Test Scene",
                                          "fonts/Marker Felt.ttf", 32);
    if (titleLabel) {
        titleLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                    origin.y + visibleSize.height - 30));
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

    // 初始化输入系统
    PlayerInput::getInstance().initialize(this->getScene());

    // 创建玩家实体
    Vec2 spawnPos(visibleSize.width / 2, 300);
    // _playerEntity = PlayerFactory::createPlayer(_world, spawnPos, this);

    // 临时：直接创建精灵测试
    auto playerSprite = PlayerFactory::createPlayerSprite(spawnPos, this);
    CCLOG("Player sprite created for testing");

    // 创建调试UI
    createDebugUI();

    // 启动更新
    this->scheduleUpdate();

    CCLOG("PlayerTestScene initialized");
    return true;
}

void PlayerTestScene::createPhysicsEnvironment() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    PhysicsMaterial material(1.0f, 0.0f, 1.0f);

    // 创建地面
    auto ground = Sprite::create();
    ground->setTextureRect(Rect(0, 0, visibleSize.width, 40));
    ground->setColor(Color3B(100, 150, 100)); // 草绿色
    ground->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + 20));
    this->addChild(ground, 0);

    auto groundBody = PhysicsBody::createBox(ground->getContentSize(), material);
    groundBody->setDynamic(false);
    groundBody->setContactTestBitmask(0xFFFFFFFF);
    ground->setPhysicsBody(groundBody);

    // 创建左墙
    auto leftWall = Sprite::create();
    leftWall->setTextureRect(Rect(0, 0, 10, visibleSize.height));
    leftWall->setColor(Color3B(80, 80, 80));
    leftWall->setPosition(Vec2(origin.x + 5, origin.y + visibleSize.height / 2));
    this->addChild(leftWall, 0);

    auto leftWallBody = PhysicsBody::createBox(leftWall->getContentSize(), material);
    leftWallBody->setDynamic(false);
    leftWallBody->setContactTestBitmask(0xFFFFFFFF);
    leftWall->setPhysicsBody(leftWallBody);

    // 创建右墙
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

    // 创建平台
    auto platform = Sprite::create();
    platform->setTextureRect(Rect(0, 0, 200, 20));
    platform->setColor(Color3B(139, 90, 43)); // 棕色
    platform->setPosition(Vec2(origin.x + visibleSize.width / 2,
                              origin.y + visibleSize.height / 2));
    this->addChild(platform, 0);

    auto platformBody = PhysicsBody::createBox(platform->getContentSize(), material);
    platformBody->setDynamic(false);
    platformBody->setContactTestBitmask(0xFFFFFFFF);
    platform->setPhysicsBody(platformBody);

    CCLOG("Physics environment created");
}

void PlayerTestScene::createDebugUI() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 创建调试信息标签
    _debugLabel = Label::createWithSystemFont("", "Arial", 14);
    _debugLabel->setAnchorPoint(Vec2(0, 1));
    _debugLabel->setPosition(Vec2(origin.x + 10, origin.y + visibleSize.height - 70));
    _debugLabel->setColor(Color3B::BLACK);
    this->addChild(_debugLabel, 100);

    // 创建提示标签
    auto hintLabel = Label::createWithSystemFont(
        "Controls:\n"
        "A/D - Move Left/Right\n"
        "Space - Jump\n"
        "E - Hook (not implemented yet)\n"
        "Mouse Click - Attack (not implemented yet)",
        "Arial", 14
    );
    hintLabel->setAnchorPoint(Vec2(1, 1));
    hintLabel->setPosition(Vec2(origin.x + visibleSize.width - 10,
                                origin.y + visibleSize.height - 70));
    hintLabel->setColor(Color3B::BLACK);
    this->addChild(hintLabel, 100);
}

void PlayerTestScene::update(float delta) {
    // 更新输入状态
    PlayerInput::getInstance().update(delta);

    // 测试输入系统
    testInputSystem();

    // 更新ECS世界（待实现）
    // _world.update(delta);

    // 更新调试信息
    updateDebugInfo(delta);
}

void PlayerTestScene::testInputSystem() {
    auto& input = PlayerInput::getInstance();

    // 测试按键输入
    if (input.isKeyJustPressed(EventKeyboard::KeyCode::KEY_SPACE)) {
        CCLOG("Space key just pressed!");
    }

    if (input.isActionPressed("MoveLeft")) {
        CCLOG("Player wants to move left");
    }

    if (input.isActionPressed("MoveRight")) {
        CCLOG("Player wants to move right");
    }

    // 测试鼠标输入
    if (input.isMouseJustPressed()) {
        Vec2 mousePos = input.getMouseWorldPosition();
        CCLOG("Mouse clicked at: (%.1f, %.1f)", mousePos.x, mousePos.y);
    }
}

void PlayerTestScene::updateDebugInfo(float dt) {
    if (!_debugLabel) return;

    auto& input = PlayerInput::getInstance();
    Vec2 mousePos = input.getMouseWorldPosition();

    // 显示输入状态
    std::string info = StringUtils::format(
        "FPS: %.1f\n"
        "Mouse: (%.0f, %.0f)\n"
        "Keys:\n"
        "  A: %s\n"
        "  D: %s\n"
        "  Space: %s\n"
        "  E: %s",
        1.0f / dt,
        mousePos.x, mousePos.y,
        input.isKeyPressed(EventKeyboard::KeyCode::KEY_A) ? "Pressed" : "Released",
        input.isKeyPressed(EventKeyboard::KeyCode::KEY_D) ? "Pressed" : "Released",
        input.isKeyPressed(EventKeyboard::KeyCode::KEY_SPACE) ? "Pressed" : "Released",
        input.isKeyPressed(EventKeyboard::KeyCode::KEY_E) ? "Pressed" : "Released"
    );

    // 如果玩家实体存在，显示玩家状态
    // if (_playerEntity != ecs::INVALID_ENTITY) {
    //     auto& stats = _world.getComponent<ecs::PlayerStatsComponent>(_playerEntity);
    //     auto& movement = _world.getComponent<ecs::PlayerMovementComponent>(_playerEntity);
    //     auto& transform = _world.getComponent<ecs::TransformComponent>(_playerEntity);
    //
    //     info += StringUtils::format(
    //         "\n\nPlayer:\n"
    //         "  Pos: (%.0f, %.0f)\n"
    //         "  HP: %.0f/%.0f\n"
    //         "  Vel: (%.1f, %.1f)\n"
    //         "  On Ground: %s",
    //         transform.x, transform.y,
    //         stats.currentHealth, stats.maxHealth,
    //         movement.velocity.x, movement.velocity.y,
    //         stats.isOnGround ? "Yes" : "No"
    //     );
    // }

    _debugLabel->setString(info);
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
