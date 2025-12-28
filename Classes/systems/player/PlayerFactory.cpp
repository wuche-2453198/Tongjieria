#include "PlayerFactory.h"
#include "components/player/PlayerComponents.h"
#include "PlayerAnimationLoader.h"

USING_NS_CC;

entt::entity PlayerFactory::createPlayer(entt::registry& registry,
                                        const Vec2& spawnPos,
                                        Node* parentNode,
                                        Node* uiLayer) {
    CCLOG("PlayerFactory: Creating player at (%.1f, %.1f)", spawnPos.x, spawnPos.y);

    // 1. Create entity
    auto player = registry.create();

    // 2. Add player tag
    registry.emplace<ecs::PlayerTag>(player);

    // 3. Add Transform component
    auto& transform = registry.emplace<ecs::TransformComponent>(player);
    transform.x = spawnPos.x;
    transform.y = spawnPos.y;

    // 4. Add stats component
    auto& stats = registry.emplace<ecs::PlayerStatsComponent>(player);
    loadPlayerStats(stats);
    stats.isOnGround = true; // Assume on ground at initialization

    // 5. Add movement component
    auto& movement = registry.emplace<ecs::PlayerMovementComponent>(player);
    movement.isFacingRight = true;

    // 6. Add equipment component
    registry.emplace<ecs::PlayerEquipmentComponent>(player);

    // 7. Add hotbar component
    auto& hotbar = registry.emplace<ecs::PlayerHotbarComponent>(player);
    // Initialize hotbar: weapon slots (inventory indices 40-49)
    for (int i = 0; i < ecs::PlayerHotbarComponent::HOTBAR_SIZE; i++) {
        hotbar.slots[i] = 40 + i;  // Map to weapon slots (40-49)
    }

    // 8. Add animation component and load animation resources
    auto& animation = registry.emplace<ecs::PlayerAnimationComponent>(player);

    // Initialize animation loader (only need to initialize once)
    static bool animLoaderInitialized = false;
    if (!animLoaderInitialized) {
        PlayerAnimationLoader::initialize(parentNode);
        animLoaderInitialized = true;
    }

    // Load initial animation (IDLE)
    const auto* idleAnim = PlayerAnimationLoader::getAnimation(
        ecs::PlayerAnimationComponent::AnimState::IDLE
    );
    if (idleAnim) {
        animation.currentFrames = idleAnim->frames;
        animation.totalFrames = idleAnim->getFrameCount();
        animation.frameTime = idleAnim->frameTime;
        CCLOG("PlayerFactory: Loaded IDLE animation with %d frames", animation.totalFrames);
    }

    // 9. Add buff component
    registry.emplace<ecs::PlayerBuffComponent>(player);

    // 10. Add ability component
    registry.emplace<ecs::PlayerAbilityComponent>(player);

    // 11. Add combat component
    registry.emplace<ecs::PlayerCombatComponent>(player);

    // 12. Create Cocos sprite
    auto sprite = createPlayerSprite(spawnPos, parentNode);

    // 13. Add sprite component
    auto& spriteComp = registry.emplace<ecs::PlayerSpriteComponent>(player);
    spriteComp.sprite = sprite;

    // 14. Create player UI (added to UI layer)
    createPlayerUI(player, registry, uiLayer);

    CCLOG("PlayerFactory: Player created successfully!");
    CCLOG("  - Entity ID: %u", static_cast<uint32_t>(player));
    CCLOG("  - Position: (%.1f, %.1f)", spawnPos.x, spawnPos.y);
    CCLOG("  - Health: %.0f/%.0f", stats.currentHealth, stats.maxHealth);
    CCLOG("  - Mana: %.0f/%.0f", stats.currentMana, stats.maxMana);

    return player;
}

Sprite* PlayerFactory::createPlayerSprite(const Vec2& spawnPos, Node* parentNode) {
    Sprite* sprite = nullptr;

    // Use first frame of idle animation as main sprite (only for physics body and positioning)
    sprite = Sprite::create("player/idle/Style_1_male.png");

    if (sprite) {
        sprite->setPosition(spawnPos);
        sprite->setVisible(false);  // Hide main sprite, use animation frame sprites for display
        parentNode->addChild(sprite, 10); // Higher Z-order to ensure foreground

        // Add physics body
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

    // Player physics body parameters (dynamically adjusted based on sprite size)
    Size spriteSize = sprite->getContentSize();
    float bodyWidth = spriteSize.width * 0.6f;   // Physics body slightly smaller than sprite to avoid edge collision issues
    float bodyHeight = spriteSize.height * 0.8f;

    // If fallback rectangle, use fixed size
    if (spriteSize.width <= 0 || spriteSize.height <= 0) {
        bodyWidth = 20.0f;
        bodyHeight = 42.0f;
        CCLOG("PlayerFactory: Using default physics body size");
    }

    PhysicsMaterial material(1.0f, 0.0f, 0.5f); // Density/Restitution/Friction

    auto physicsBody = PhysicsBody::createBox(
        Size(bodyWidth, bodyHeight),
        material
    );

    CCLOG("PlayerFactory: Physics body size: %.1fx%.1f (sprite size: %.1fx%.1f)",
          bodyWidth, bodyHeight, spriteSize.width, spriteSize.height);

    if (physicsBody) {
        physicsBody->setDynamic(true);               // Dynamic body
        physicsBody->setRotationEnable(false);       // Disable rotation (player doesn't rotate)
        physicsBody->setGravityEnable(true);         // Affected by gravity
        physicsBody->setContactTestBitmask(0xFFFFFFFF); // Collision detection
        physicsBody->setCollisionBitmask(0xFFFFFFFF);
        physicsBody->setCategoryBitmask(0x01);       // Player category

        // Set mass (affects inertia)
        physicsBody->setMass(1.0f);

        // Set linear damping (air resistance)
        physicsBody->setLinearDamping(0.0f); // Controlled by code, not physics engine damping

        sprite->setPhysicsBody(physicsBody);

        CCLOG("PlayerFactory: Physics body added to player");
    } else {
        CCLOG("PlayerFactory: ERROR - Failed to create physics body!");
    }
}

void PlayerFactory::loadPlayerStats(ecs::PlayerStatsComponent& stats) {
    // Load player initial stats
    // Can load from JSON config file or save file

    // Default stats (new player)
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

    // TODO: Load from config file
    // Example code:
    // auto fileUtils = FileUtils::getInstance();
    // std::string configPath = fileUtils->fullPathForFilename("config/player_stats.json");
    // if (fileUtils->isFileExist(configPath)) {
    //     std::string content = fileUtils->getStringFromFile(configPath);
    //     // Parse and set stats using RapidJSON
    // }

    CCLOG("PlayerFactory: Player stats loaded (HP: %.0f/%.0f, Mana: %.0f/%.0f)",
          stats.currentHealth, stats.maxHealth,
          stats.currentMana, stats.maxMana);
}

void PlayerFactory::createPlayerUI(entt::entity entity,
                                   entt::registry& registry,
                                   Node* uiLayer) {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    auto& spriteComp = registry.get<ecs::PlayerSpriteComponent>(entity);
    auto& stats = registry.get<ecs::PlayerStatsComponent>(entity);

    // UI layout configuration - Right-top corner
    float rightMargin = 20.0f;
    float topMargin = 20.0f;
    float barWidth = 200.0f;
    float barHeight = 20.0f;
    float barSpacing = 8.0f;

    // CRITICAL FIX: Use absolute screen coordinates (origin + visibleSize)
    // This ensures UI stays fixed at screen right-top corner regardless of camera movement
    // Anchor point (1, 1) means positioning from top-right corner of the element
    float startX = origin.x + visibleSize.width - rightMargin;
    float startY = origin.y + visibleSize.height - topMargin;

    // ==================== Health Bar (1st row) ====================
    // Health bar background
    auto healthBarBg = Sprite::create();
    healthBarBg->setTextureRect(Rect(0, 0, barWidth, barHeight));
    healthBarBg->setColor(Color3B(50, 50, 50));
    healthBarBg->setAnchorPoint(Vec2(1, 1));  // Anchor at top-right corner
    healthBarBg->setPosition(Vec2(startX, startY));
    uiLayer->addChild(healthBarBg, 100);

    // Health bar fill
    auto healthBarFill = Sprite::create();
    healthBarFill->setTextureRect(Rect(0, 0, barWidth, barHeight));
    healthBarFill->setColor(Color3B(220, 20, 60)); // Crimson red
    healthBarFill->setAnchorPoint(Vec2(0, 0));
    healthBarFill->setPosition(Vec2(0, 0));
    healthBarBg->addChild(healthBarFill, 1);

    // Health bar label (centered)
    auto healthLabel = Label::createWithSystemFont("HP: 100/100", "Arial", 14);
    healthLabel->setAnchorPoint(Vec2(0.5f, 0.5f));
    healthLabel->setPosition(Vec2(barWidth / 2, barHeight / 2));
    healthLabel->setColor(Color3B::WHITE);
    healthBarBg->addChild(healthLabel, 2);

    spriteComp.healthBarBg = healthBarBg;
    spriteComp.healthBarFill = healthBarFill;

    // ==================== Mana Bar (2nd row) ====================
    // Mana bar background
    auto manaBarBg = Sprite::create();
    manaBarBg->setTextureRect(Rect(0, 0, barWidth, barHeight));
    manaBarBg->setColor(Color3B(50, 50, 50));
    manaBarBg->setAnchorPoint(Vec2(1, 1));  // Anchor at top-right corner
    manaBarBg->setPosition(Vec2(startX, startY - barHeight - barSpacing));
    uiLayer->addChild(manaBarBg, 100);

    // Mana bar fill
    auto manaBarFill = Sprite::create();
    manaBarFill->setTextureRect(Rect(0, 0, barWidth, barHeight));
    manaBarFill->setColor(Color3B(30, 144, 255)); // Blue
    manaBarFill->setAnchorPoint(Vec2(0, 0));
    manaBarFill->setPosition(Vec2(0, 0));
    manaBarBg->addChild(manaBarFill, 1);

    // Mana bar label (centered)
    auto manaLabel = Label::createWithSystemFont("MP: 20/20", "Arial", 14);
    manaLabel->setAnchorPoint(Vec2(0.5f, 0.5f));
    manaLabel->setPosition(Vec2(barWidth / 2, barHeight / 2));
    manaLabel->setColor(Color3B::WHITE);
    manaBarBg->addChild(manaLabel, 2);

    spriteComp.manaBarBg = manaBarBg;
    spriteComp.manaBarFill = manaBarFill;

    // ==================== Defense Bar (3rd row) ====================
    // Defense bar background
    auto defenseBarBg = Sprite::create();
    defenseBarBg->setTextureRect(Rect(0, 0, barWidth, barHeight));
    defenseBarBg->setColor(Color3B(60, 60, 60));
    defenseBarBg->setAnchorPoint(Vec2(1, 1));  // Anchor at top-right corner
    defenseBarBg->setPosition(Vec2(startX, startY - 2 * (barHeight + barSpacing)));
    uiLayer->addChild(defenseBarBg, 100);

    // Defense label (centered)
    auto defenseLabel = Label::createWithSystemFont("Defense: 0", "Arial", 14);
    defenseLabel->setAnchorPoint(Vec2(0.5f, 0.5f));
    defenseLabel->setPosition(Vec2(barWidth / 2, barHeight / 2));
    defenseLabel->setColor(Color3B(192, 192, 192)); // Silver
    defenseBarBg->addChild(defenseLabel, 2);

    spriteComp.defenseBarBg = defenseBarBg;
    spriteComp.defenseLabel = defenseLabel;

    CCLOG("PlayerFactory: Player UI created at top-right corner");
    CCLOG("  - Health: %.0f/%.0f", stats.currentHealth, stats.maxHealth);
    CCLOG("  - Mana: %.0f/%.0f", stats.currentMana, stats.maxMana);
    CCLOG("  - Defense: %d", stats.defense);
}
