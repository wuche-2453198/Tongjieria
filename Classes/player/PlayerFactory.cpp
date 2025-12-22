#include "PlayerFactory.h"
#include "PlayerComponents.h"
#include "PlayerAnimationLoader.h"

USING_NS_CC;

entt::entity PlayerFactory::createPlayer(entt::registry& registry,
                                        const Vec2& spawnPos,
                                        Node* parentNode) {
    CCLOG("PlayerFactory: Creating player at (%.1f, %.1f)", spawnPos.x, spawnPos.y);

    // 1. 创建实体
    auto player = registry.create();

    // 2. 添加玩家标识
    registry.emplace<ecs::PlayerTag>(player);

    // 3. 添加 Transform 组件
    auto& transform = registry.emplace<ecs::TransformComponent>(player);
    transform.x = spawnPos.x;
    transform.y = spawnPos.y;

    // 4. 添加属性组件
    auto& stats = registry.emplace<ecs::PlayerStatsComponent>(player);
    loadPlayerStats(stats);
    stats.isOnGround = true; // 初始化时假设在地面上

    // 5. 添加移动组件
    auto& movement = registry.emplace<ecs::PlayerMovementComponent>(player);
    movement.isFacingRight = true;

    // 6. 添加装备组件
    registry.emplace<ecs::PlayerEquipmentComponent>(player);

    // 7. 添加快捷栏组件
    auto& hotbar = registry.emplace<ecs::PlayerHotbarComponent>(player);
    // 初始化快捷栏：前10个背包槽位
    for (int i = 0; i < ecs::PlayerHotbarComponent::HOTBAR_SIZE; i++) {
        hotbar.slots[i] = i;
    }

    // 8. 添加动画组件并加载动画资源
    auto& animation = registry.emplace<ecs::PlayerAnimationComponent>(player);

    // 初始化动画加载器（只需要初始化一次）
    static bool animLoaderInitialized = false;
    if (!animLoaderInitialized) {
        PlayerAnimationLoader::initialize(parentNode);
        animLoaderInitialized = true;
    }

    // 加载初始动画（IDLE）
    const auto* idleAnim = PlayerAnimationLoader::getAnimation(
        ecs::PlayerAnimationComponent::AnimState::IDLE
    );
    if (idleAnim) {
        animation.currentFrames = idleAnim->frames;
        animation.totalFrames = idleAnim->getFrameCount();
        animation.frameTime = idleAnim->frameTime;
        CCLOG("PlayerFactory: Loaded IDLE animation with %d frames", animation.totalFrames);
    }

    // 9. 添加Buff组件
    registry.emplace<ecs::PlayerBuffComponent>(player);

    // 10. 添加能力组件
    registry.emplace<ecs::PlayerAbilityComponent>(player);

    // 11. 添加战斗组件
    registry.emplace<ecs::PlayerCombatComponent>(player);

    // 12. 创建Cocos精灵
    auto sprite = createPlayerSprite(spawnPos, parentNode);

    // 13. 添加精灵组件
    auto& spriteComp = registry.emplace<ecs::PlayerSpriteComponent>(player);
    spriteComp.sprite = sprite;

    // 14. 创建玩家UI
    createPlayerUI(player, registry, parentNode);

    CCLOG("PlayerFactory: Player created successfully!");
    CCLOG("  - Entity ID: %u", static_cast<uint32_t>(player));
    CCLOG("  - Position: (%.1f, %.1f)", spawnPos.x, spawnPos.y);
    CCLOG("  - Health: %.0f/%.0f", stats.currentHealth, stats.maxHealth);
    CCLOG("  - Mana: %.0f/%.0f", stats.currentMana, stats.maxMana);

    return player;
}

Sprite* PlayerFactory::createPlayerSprite(const Vec2& spawnPos, Node* parentNode) {
    Sprite* sprite = nullptr;

    // 使用站立动画的第一帧作为主精灵（仅用于物理体和位置定位）
    sprite = Sprite::create("player/idle/Style_1_male.png");

    if (sprite) {
        sprite->setPosition(spawnPos);
        sprite->setVisible(false);  // 隐藏主精灵，使用动画帧精灵显示
        parentNode->addChild(sprite, 10); // 较高的Z-order，确保在前景

        // 添加物理体
        addPhysicsBody(sprite);

        CCLOG("PlayerFactory: Player sprite created at (%.1f, %.1f) with animation frame",
              spawnPos.x, spawnPos.y);
    } else {
        CCLOG("PlayerFactory: ERROR - Failed to create player sprite!");
    }

    return sprite;
}

void PlayerFactory::addPhysicsBody(Sprite* sprite) {
    if (!sprite) {
        CCLOG("PlayerFactory: Cannot add physics body to null sprite");
        return;
    }

    // 玩家物理体参数（根据精灵大小动态调整）
    Size spriteSize = sprite->getContentSize();
    float bodyWidth = spriteSize.width * 0.6f;   // 物理体比精灵稍小，避免边缘碰撞问题
    float bodyHeight = spriteSize.height * 0.8f;

    // 如果是备用矩形，使用固定大小
    if (spriteSize.width <= 0 || spriteSize.height <= 0) {
        bodyWidth = 20.0f;
        bodyHeight = 42.0f;
        CCLOG("PlayerFactory: Using default physics body size");
    }

    PhysicsMaterial material(1.0f, 0.0f, 0.5f); // 密度/恢复系数/摩擦力

    auto physicsBody = PhysicsBody::createBox(
        Size(bodyWidth, bodyHeight),
        material
    );

    CCLOG("PlayerFactory: Physics body size: %.1fx%.1f (sprite size: %.1fx%.1f)",
          bodyWidth, bodyHeight, spriteSize.width, spriteSize.height);

    if (physicsBody) {
        physicsBody->setDynamic(true);               // 动态物体
        physicsBody->setRotationEnable(false);       // 禁止旋转（玩家不旋转）
        physicsBody->setGravityEnable(true);         // 受重力影响
        physicsBody->setContactTestBitmask(0xFFFFFFFF); // 碰撞检测
        physicsBody->setCollisionBitmask(0xFFFFFFFF);
        physicsBody->setCategoryBitmask(0x01);       // 玩家类别

        // 设置质量（影响惯性）
        physicsBody->setMass(1.0f);

        // 设置线性阻尼（空气阻力）
        physicsBody->setLinearDamping(0.0f); // 由代码控制，不用物理引擎的阻尼

        sprite->setPhysicsBody(physicsBody);

        CCLOG("PlayerFactory: Physics body added to player");
    } else {
        CCLOG("PlayerFactory: ERROR - Failed to create physics body!");
    }
}

void PlayerFactory::loadPlayerStats(ecs::PlayerStatsComponent& stats) {
    // 加载玩家初始属性
    // 可以从JSON配置文件或存档加载

    // 默认属性（新玩家）
    stats.maxHealth = 100.0f;
    stats.currentHealth = 100.0f;
    stats.healthRegen = 0.5f;

    stats.maxMana = 20.0f;
    stats.currentMana = 20.0f;
    stats.manaRegen = 0.2f;

    stats.defense = 0;

    stats.moveSpeed = 200.0f;
    stats.jumpHeight = 400.0f;
    stats.extraJumps = 0;

    stats.critChance = 0.04f;
    stats.critMultiplier = 2.0f;

    stats.invincibleDuration = 0.6f;

    // TODO: 从配置文件加载
    // 示例代码：
    // auto fileUtils = FileUtils::getInstance();
    // std::string configPath = fileUtils->fullPathForFilename("config/player_stats.json");
    // if (fileUtils->isFileExist(configPath)) {
    //     std::string content = fileUtils->getStringFromFile(configPath);
    //     // 使用 RapidJSON 解析并设置属性
    // }

    CCLOG("PlayerFactory: Player stats loaded (HP: %.0f/%.0f, Mana: %.0f/%.0f)",
          stats.currentHealth, stats.maxHealth,
          stats.currentMana, stats.maxMana);
}

void PlayerFactory::createPlayerUI(entt::entity entity,
                                   entt::registry& registry,
                                   Node* parentNode) {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    auto& spriteComp = registry.get<ecs::PlayerSpriteComponent>(entity);

    // 创建血条（屏幕左上角）

    // 血条背景
    auto healthBarBg = Sprite::create();
    healthBarBg->setTextureRect(Rect(0, 0, 200, 20));
    healthBarBg->setColor(Color3B(50, 50, 50));
    healthBarBg->setAnchorPoint(Vec2(0, 1));
    healthBarBg->setPosition(Vec2(origin.x + 20, origin.y + visibleSize.height - 20));
    parentNode->addChild(healthBarBg, 100);

    // 血条填充
    auto healthBarFill = Sprite::create();
    healthBarFill->setTextureRect(Rect(0, 0, 200, 20));
    healthBarFill->setColor(Color3B(220, 20, 60)); // 深红色
    healthBarFill->setAnchorPoint(Vec2(0, 0));
    healthBarFill->setPosition(Vec2(0, 0));
    healthBarBg->addChild(healthBarFill, 1);

    // 血条标签
    auto healthLabel = Label::createWithSystemFont("HP", "Arial", 14);
    healthLabel->setAnchorPoint(Vec2(0, 0.5f));
    healthLabel->setPosition(Vec2(5, 10));
    healthLabel->setColor(Color3B::WHITE);
    healthBarBg->addChild(healthLabel, 2);

    spriteComp.healthBarBg = healthBarBg;
    spriteComp.healthBarFill = healthBarFill;

    // 创建魔法条（血条下方）

    // 魔法条背景
    auto manaBarBg = Sprite::create();
    manaBarBg->setTextureRect(Rect(0, 0, 200, 20));
    manaBarBg->setColor(Color3B(50, 50, 50));
    manaBarBg->setAnchorPoint(Vec2(0, 1));
    manaBarBg->setPosition(Vec2(origin.x + 20, origin.y + visibleSize.height - 50));
    parentNode->addChild(manaBarBg, 100);

    // 魔法条填充
    auto manaBarFill = Sprite::create();
    manaBarFill->setTextureRect(Rect(0, 0, 200, 20));
    manaBarFill->setColor(Color3B(30, 144, 255)); // 蓝色
    manaBarFill->setAnchorPoint(Vec2(0, 0));
    manaBarFill->setPosition(Vec2(0, 0));
    manaBarBg->addChild(manaBarFill, 1);

    // 魔法条标签
    auto manaLabel = Label::createWithSystemFont("MP", "Arial", 14);
    manaLabel->setAnchorPoint(Vec2(0, 0.5f));
    manaLabel->setPosition(Vec2(5, 10));
    manaLabel->setColor(Color3B::WHITE);
    manaBarBg->addChild(manaLabel, 2);

    spriteComp.manaBarBg = manaBarBg;
    spriteComp.manaBarFill = manaBarFill;

    CCLOG("PlayerFactory: Player UI created (Health bar and Mana bar)");
}
