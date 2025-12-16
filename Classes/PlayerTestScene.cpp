#include "PlayerTestScene.h"
#include "MainMenuScene.h"
#include "player/PlayerFactory.h"
#include "player/PlayerComponents.h"
#include "player/PlayerInput.h"

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

    // 初始化输入系统
    PlayerInput::getInstance().initialize(this->getScene());

    // 创建玩家
    createPlayer();

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
    _playerEntity = PlayerFactory::createPlayer(_registry, spawnPos, this);

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
    // 1. 更新输入状态
    PlayerInput::getInstance().update(delta);

    // 2. 检测地面
    detectGround();

    // 3. 更新玩家逻辑
    updatePlayer(delta);

    // 4. 更新玩家UI
    updatePlayerUI(delta);

    // 5. 更新调试信息
    updateDebugInfo(delta);
}

void PlayerTestScene::detectGround() {
    auto view = _registry.view<ecs::PlayerTag,
                                ecs::PlayerSpriteComponent,
                                ecs::PlayerStatsComponent>();

    for (auto entity : view) {
        auto [sprite, stats] = view.get<ecs::PlayerSpriteComponent,
                                       ecs::PlayerStatsComponent>(entity);

        if (sprite.sprite && sprite.sprite->getPhysicsBody()) {
            auto body = sprite.sprite->getPhysicsBody();
            float velocityY = body->getVelocity().y;

            // 简单地面检测：速度接近0且位置较低
            if (std::abs(velocityY) < 10.0f && sprite.sprite->getPositionY() < 500) {
                stats.isOnGround = true;
            } else if (velocityY < -50.0f) {
                stats.isOnGround = false;
            }
        }
    }
}

void PlayerTestScene::updatePlayer(float dt) {
    auto& input = PlayerInput::getInstance();

    auto view = _registry.view<ecs::PlayerTag,
                                ecs::PlayerMovementComponent,
                                ecs::PlayerStatsComponent,
                                ecs::TransformComponent,
                                ecs::PlayerSpriteComponent>();

    for (auto entity : view) {
        auto [movement, stats, transform, sprite] =
            view.get<ecs::PlayerMovementComponent,
                     ecs::PlayerStatsComponent,
                     ecs::TransformComponent,
                     ecs::PlayerSpriteComponent>(entity);

        // === 处理输入 ===
        movement.isMovingLeft = input.isActionPressed("MoveLeft");
        movement.isMovingRight = input.isActionPressed("MoveRight");
        movement.wantsToJump = input.isActionJustPressed("Jump");

        // === 移动逻辑 ===
        float targetVelX = 0.0f;
        if (movement.isMovingRight) {
            targetVelX = stats.moveSpeed;
            movement.isFacingRight = true;
        }
        if (movement.isMovingLeft) {
            targetVelX = -stats.moveSpeed;
            movement.isFacingRight = false;
        }

        // 平滑加速
        float accel = movement.accelerationRate * dt;
        if (targetVelX != 0) {
            movement.velocity.x += (targetVelX - movement.velocity.x) * accel * 0.1f;
        } else {
            // 摩擦力减速
            movement.velocity.x *= (stats.isOnGround ? movement.friction : movement.airResistance);
        }

        // 限制最大速度
        if (std::abs(movement.velocity.x) > movement.maxHorizontalSpeed) {
            movement.velocity.x = (movement.velocity.x > 0)
                ? movement.maxHorizontalSpeed
                : -movement.maxHorizontalSpeed;
        }

        // === 跳跃逻辑 ===
        if (movement.wantsToJump && stats.isOnGround) {
            if (sprite.sprite && sprite.sprite->getPhysicsBody()) {
                auto body = sprite.sprite->getPhysicsBody();
                body->setVelocity(Vec2(body->getVelocity().x, stats.jumpHeight));
                stats.isOnGround = false;
                CCLOG("Player jumped!");
            }
            movement.wantsToJump = false;
        }

        // === 同步到物理引擎 ===
        if (sprite.sprite && sprite.sprite->getPhysicsBody()) {
            auto body = sprite.sprite->getPhysicsBody();
            body->setVelocity(Vec2(movement.velocity.x, body->getVelocity().y));

            // 更新位置
            transform.x = sprite.sprite->getPositionX();
            transform.y = sprite.sprite->getPositionY();
        }

        // === 翻转精灵 ===
        if (sprite.sprite) {
            sprite.sprite->setFlippedX(!movement.isFacingRight);
        }
    }
}

void PlayerTestScene::updatePlayerUI(float dt) {
    auto view = _registry.view<ecs::PlayerTag,
                                ecs::PlayerStatsComponent,
                                ecs::PlayerSpriteComponent>();

    for (auto entity : view) {
        auto [stats, sprite] = view.get<ecs::PlayerStatsComponent,
                                        ecs::PlayerSpriteComponent>(entity);

        // 更新血条
        if (sprite.healthBarFill) {
            float healthPercent = stats.currentHealth / stats.maxHealth;
            sprite.healthBarFill->setScaleX(healthPercent);
        }

        // 更新魔法条
        if (sprite.manaBarFill) {
            float manaPercent = stats.currentMana / stats.maxMana;
            sprite.manaBarFill->setScaleX(manaPercent);
        }
    }
}

void PlayerTestScene::updateDebugInfo(float dt) {
    if (!_debugLabel) return;

    auto& input = PlayerInput::getInstance();

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
