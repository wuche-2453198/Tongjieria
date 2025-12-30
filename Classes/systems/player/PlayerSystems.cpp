#include "PlayerSystems.h"
#include "core/PlayerInput.h"
#include "core/consts.h"
#include "PlayerAnimationLoader.h"
#include "components/combat/HealthComponent.h"
#include "components/render/SpriteStateComponent.h"
#include "components/physics/PhysicsBodyComponent.h"
#include "components/block/block_component.h"
#include <cmath>
#include <algorithm>

USING_NS_CC;

// Static member initialization
cocos2d::Scene* PlayerSystemsManager::s_scene = nullptr;
float PlayerCameraSystem::s_followSpeed = 0.1f;  // 默认跟随速度

// ==================== PlayerInputSystem ====================

void PlayerInputSystem::update(entt::registry& registry, float dt) {
    auto& input = PlayerInput::getInstance();

    // Iterate over all player entities
    auto view = registry.view<ecs::PlayerTag,
                              ecs::PlayerMovementComponent,
                              ecs::PlayerHotbarComponent,
                              ecs::PlayerAnimationComponent>();

    for (auto entity : view) {
        auto& movement = view.get<ecs::PlayerMovementComponent>(entity);
        auto& hotbar = view.get<ecs::PlayerHotbarComponent>(entity);
        auto& animation = view.get<ecs::PlayerAnimationComponent>(entity);

        // ==================== Update Movement Input ====================
        movement.isMovingLeft = input.isActionPressed("MoveLeft");
        movement.isMovingRight = input.isActionPressed("MoveRight");

        // Detailed jump input debugging
        bool jumpPressed = input.isActionJustPressed("Jump");
        bool spacePressed = input.isKeyJustPressed(EventKeyboard::KeyCode::KEY_SPACE);

        if (spacePressed) {
            CCLOG(">>> SPACE KEY PRESSED! (direct check)");
        }
        if (jumpPressed) {
            CCLOG(">>> JUMP ACTION TRIGGERED! (action mapping check)");
        }

        movement.wantsToJump = jumpPressed;

        bool hotbarChanged = false;
        int newSlotIndex = -1;

        for (int i = 0; i < 9; i++) {
            auto keyCode = static_cast<EventKeyboard::KeyCode>(
                static_cast<int>(EventKeyboard::KeyCode::KEY_1) + i
            );
            if (input.isKeyJustPressed(keyCode)) {
                hotbar.selectSlot(i);  // KEY_1 -> slot 0, KEY_2 -> slot 1, etc.
                newSlotIndex = i;
                hotbarChanged = true;
                CCLOG("Hotbar slot changed to: %d (key: %d)", i, i + 1);
                break;
            }
        }

        // Number key 0 selects slot 9 (10th slot)
        if (input.isKeyJustPressed(EventKeyboard::KeyCode::KEY_0)) {
            hotbar.selectSlot(9);  // KEY_0 -> slot 9
            newSlotIndex = 9;
            hotbarChanged = true;
            CCLOG("Hotbar slot changed to: 9 (key: 0)");
        }

        // Dispatch hotbar changed event for UI updates
        if (hotbarChanged) {
            auto event = EventCustom("Event_HotbarChanged");
            event.setUserData(&newSlotIndex);
            Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
        }
    }
}

// ==================== PlayerMovementSystem ====================

void PlayerMovementSystem::update(entt::registry& registry, float dt) {
    auto view = registry.view<ecs::PlayerTag,
                              ecs::PlayerMovementComponent,
                              ecs::PlayerStatsComponent,
                              ecs::TransformComponent,
                              ecs::PlayerSpriteComponent>();

    for (auto entity : view) {
        auto& movement = view.get<ecs::PlayerMovementComponent>(entity);
        auto& stats = view.get<ecs::PlayerStatsComponent>(entity);
        auto& transform = view.get<ecs::TransformComponent>(entity);
        auto& sprite = view.get<ecs::PlayerSpriteComponent>(entity);

        // 1. Process horizontal movement
        processHorizontalMovement(movement, stats, sprite, dt);

        // 2. Process jump
        processJump(movement, stats, sprite);

        // 3. Sync to physics engine and block system
        syncPhysics(registry, entity, movement, transform, sprite);

        // 4. Update sprite direction
        updateSpriteDirection(movement, sprite);
    }
}

void PlayerMovementSystem::processHorizontalMovement(
    ecs::PlayerMovementComponent& movement,
    ecs::PlayerStatsComponent& stats,
    ecs::PlayerSpriteComponent& sprite,
    float dt)
{
    float targetVelX = 0.0f;

    // Determine target velocity
    if (movement.isMovingRight) {
        targetVelX = stats.moveSpeed;
        movement.isFacingRight = true;
    }
    if (movement.isMovingLeft) {
        targetVelX = -stats.moveSpeed;
        movement.isFacingRight = false;
    }

    // Check wall collision before applying movement
    auto scene = PlayerSystemsManager::getScene();
    if (scene && sprite.sprite) {
        // Check left wall collision
        if (targetVelX < 0 && PlayerGroundDetectionSystem::raycastLeftWall(sprite.sprite, scene, 3.0f)) {
            targetVelX = 0.0f;
            movement.velocity.x = 0.0f;  // Stop immediately when hitting wall
        }
        // Check right wall collision
        else if (targetVelX > 0 && PlayerGroundDetectionSystem::raycastRightWall(sprite.sprite, scene, 3.0f)) {
            targetVelX = 0.0f;
            movement.velocity.x = 0.0f;  // Stop immediately when hitting wall
        }
    }

    // Terraria-style accelerated movement
    if (targetVelX != 0) {
        // Accelerate
        float accel = movement.accelerationRate * dt;
        movement.velocity.x += (targetVelX - movement.velocity.x) * accel * 0.1f;
    } else {
        // Friction deceleration
        float frictionCoeff = stats.isOnGround ? movement.friction : movement.airResistance;
        movement.velocity.x *= frictionCoeff;

        // Stop directly when velocity is very small
        if (std::abs(movement.velocity.x) < 1.0f) {
            movement.velocity.x = 0.0f;
        }
    }

    // Limit maximum horizontal velocity
    if (std::abs(movement.velocity.x) > movement.maxHorizontalSpeed) {
        movement.velocity.x = (movement.velocity.x > 0)
            ? movement.maxHorizontalSpeed
            : -movement.maxHorizontalSpeed;
    }
}

void PlayerMovementSystem::processJump(
    ecs::PlayerMovementComponent& movement,
    ecs::PlayerStatsComponent& stats,
    ecs::PlayerSpriteComponent& sprite)
{
    if (!movement.wantsToJump) {
        return;
    }

    CCLOG("Jump requested! isOnGround=%s", stats.isOnGround ? "YES" : "NO");

    // Reset jump request
    movement.wantsToJump = false;

    // Check if can jump
    if (!stats.isOnGround) {
        CCLOG("Jump rejected: player not on ground");
        // TODO: Implement double jump 双跳暂时不做先
        // if (stats.currentJumpCount < stats.extraJumps) { ... }
        return;
    }

    // Execute jump
    if (sprite.sprite && sprite.sprite->getPhysicsBody()) {
        auto body = sprite.sprite->getPhysicsBody();
        body->setVelocity(Vec2(body->getVelocity().x, stats.jumpHeight));
        stats.isOnGround = false;
        stats.currentJumpCount = 1;
        CCLOG("Player jumped! (velocity Y set to: %.1f)", stats.jumpHeight);
    } else {
        CCLOG("Jump failed: sprite or physics body is null");
    }
}

void PlayerMovementSystem::syncPhysics(
    entt::registry& registry,
    entt::entity entity,
    ecs::PlayerMovementComponent& movement,
    ecs::TransformComponent& transform,
    ecs::PlayerSpriteComponent& sprite)
{
    if (!sprite.sprite || !sprite.sprite->getPhysicsBody()) {
        return;
    }

    const bool hasUnifiedPhysicsSync = registry.all_of<ecs::SpriteStateComponent, ecs::PhysicsBodyComponent>(entity);

    auto body = sprite.sprite->getPhysicsBody();

    // Set horizontal velocity (vertical velocity controlled by physics engine)
    body->setVelocity(Vec2(movement.velocity.x, body->getVelocity().y));

    // Update Transform component
    transform.previousPosition = transform.position;
    if (!hasUnifiedPhysicsSync) {
        transform.position = sprite.sprite->getPosition();
    }

    // Sync Position component for block system (if exists)
    static bool loggedPositionSync = false;
    if (!hasUnifiedPhysicsSync) {
        if (registry.all_of<Position>(entity)) {
            auto& position = registry.get<Position>(entity);
            position.setPosition(transform.position);

            if (!loggedPositionSync) {
                CCLOG("[PlayerMovement] Position component synced! transform(%.1f,%.1f)", transform.position.x, transform.position.y);
                loggedPositionSync = true;
            }
        } else {
            static bool warnedOnce = false;
            if (!warnedOnce) {
                CCLOG("[PlayerMovement] ERROR: Player has NO Position component!");
                warnedOnce = true;
            }
        }
    }

    // Update Movement component's vertical velocity (for debugging and animation)
    movement.velocity.y = body->getVelocity().y;
}

void PlayerMovementSystem::updateSpriteDirection(
    ecs::PlayerMovementComponent& movement,
    ecs::PlayerSpriteComponent& sprite)
{
    if (sprite.sprite) {
        sprite.sprite->setFlippedX(!movement.isFacingRight);
    }
}

// ==================== PlayerGroundDetectionSystem ====================

void PlayerGroundDetectionSystem::update(entt::registry& registry, float dt) {
    auto view = registry.view<ecs::PlayerTag,
                              ecs::PlayerStatsComponent,
                              ecs::PlayerSpriteComponent,
                              ecs::PhysicsBodyComponent>();

    for (auto entity : view) {
        auto& stats = view.get<ecs::PlayerStatsComponent>(entity);
        auto& sprite = view.get<ecs::PlayerSpriteComponent>(entity);
        auto& physics = view.get<ecs::PhysicsBodyComponent>(entity);

        if (!sprite.sprite || !sprite.sprite->getPhysicsBody()) {
            continue;
        }

        auto body = sprite.sprite->getPhysicsBody();
        float velocityY = body->getVelocity().y;

        // Record previous state
        bool wasOnGround = stats.isOnGround;

        // Improved ground detection logic
        if (velocityY > 50.0f) {
            // Rapidly rising (just jumped)
            stats.isOnGround = false;
        } else {
            // Other cases use raycast detection
            auto scene = PlayerSystemsManager::getScene();
            if (scene) {
                stats.isOnGround = raycastGround(sprite.sprite, scene, physics);
            } else {
                // Fallback: use velocity detection
                // Vertical velocity near 0 and not rising, likely on ground
                stats.isOnGround = (velocityY >= -10.0f && velocityY <= 10.0f);
            }
        }

        // Reset jump count when landing
        if (!wasOnGround && stats.isOnGround) {
            stats.currentJumpCount = 0;
            CCLOG("Player landed on ground");
        }

        // Debug output (output once every 60 frames to avoid excessive logging)
        // static int debugCounter = 0;
        // if (debugCounter++ % 60 == 0) {
        //     CCLOG("Ground Detection: isOnGround=%s, velocityY=%.1f",
        //           stats.isOnGround ? "YES" : "NO", velocityY);
        // }
    }
}

bool PlayerGroundDetectionSystem::raycastGround(cocos2d::Sprite* sprite, cocos2d::Scene* scene, const ecs::PhysicsBodyComponent& physics) {
    if (!sprite || !scene) {
        return false;
    }

    auto physicsWorld = scene->getPhysicsWorld();
    if (!physicsWorld) {
        return false;
    }

    auto body = sprite->getPhysicsBody();
    if (!body) {
        return false;
    }

    Vec2 playerPos = sprite->getPosition();

    float bodyHalfWidth = 10.0f;
    float bodyHalfHeight = 21.0f;
    if (physics.shape == ecs::PhysicsBodyComponent::BodyShape::Box) {
        bodyHalfWidth = std::max(0.0f, physics.width) * 0.5f;
        bodyHalfHeight = std::max(0.0f, physics.height) * 0.5f;
    } else {
        bodyHalfWidth = std::max(0.0f, physics.radius);
        bodyHalfHeight = std::max(0.0f, physics.radius);
    }

    const float footInset = 1.0f;
    const float xOffsets[] = {
        -(bodyHalfWidth - footInset),
        0.0f,
        (bodyHalfWidth - footInset)
    };

    const float startY = playerPos.y - bodyHalfHeight + 2.0f;
    const float endY = playerPos.y - bodyHalfHeight - 10.0f;

    bool hitGround = false;
    for (float xo : xOffsets) {
        if (hitGround) {
            break;
        }

        const Vec2 rayStart(playerPos.x + xo, startY);
        const Vec2 rayEnd(playerPos.x + xo, endY);

        physicsWorld->rayCast([&hitGround, body](PhysicsWorld& world,
                                                 const PhysicsRayCastInfo& info,
                                                 void* data) -> bool {
            // Ignore player's own collider
            if (info.shape && info.shape->getBody() == body) {
                return true;
            }

            // Only treat static bodies as ground (blocks/terrain)
            if (info.shape && info.shape->getBody() && !info.shape->getBody()->isDynamic()) {
                hitGround = true;
                return false;
            }

            return true;
        }, rayStart, rayEnd, nullptr);
    }

    return hitGround;
}

bool PlayerGroundDetectionSystem::raycastLeftWall(cocos2d::Sprite* sprite, cocos2d::Scene* scene, float checkDistance) {
    if (!sprite || !scene) {
        return false;
    }

    auto physicsWorld = scene->getPhysicsWorld();
    if (!physicsWorld) {
        return false;
    }

    auto body = sprite->getPhysicsBody();
    if (!body) {
        return false;
    }

    Vec2 playerPos = sprite->getPosition();

    // Physics body width is 20.0f (from PlayerFactory)
    float bodyHalfWidth = 10.0f;  // 20.0f / 2

    // Raycast from player left side
    Vec2 rayStart = Vec2(playerPos.x - bodyHalfWidth, playerPos.y);
    Vec2 rayEnd = Vec2(playerPos.x - bodyHalfWidth - checkDistance, playerPos.y);

    bool hitWall = false;

    // Execute raycast
    physicsWorld->rayCast([&hitWall, body](PhysicsWorld& world,
                                            const PhysicsRayCastInfo& info,
                                            void* data) -> bool {
        // Ignore player's own collider
        if (info.shape->getBody() == body) {
            return true; // Continue detection
        }

        // Hit wall
        hitWall = true;
        return false; // Stop detection
    }, rayStart, rayEnd, nullptr);

    return hitWall;
}

bool PlayerGroundDetectionSystem::raycastRightWall(cocos2d::Sprite* sprite, cocos2d::Scene* scene, float checkDistance) {
    if (!sprite || !scene) {
        return false;
    }

    auto physicsWorld = scene->getPhysicsWorld();
    if (!physicsWorld) {
        return false;
    }

    auto body = sprite->getPhysicsBody();
    if (!body) {
        return false;
    }

    Vec2 playerPos = sprite->getPosition();

    // Physics body width is 20.0f (from PlayerFactory)
    float bodyHalfWidth = 10.0f;  // 20.0f / 2

    // Raycast from player right side
    Vec2 rayStart = Vec2(playerPos.x + bodyHalfWidth, playerPos.y);
    Vec2 rayEnd = Vec2(playerPos.x + bodyHalfWidth + checkDistance, playerPos.y);

    bool hitWall = false;

    // Execute raycast
    physicsWorld->rayCast([&hitWall, body](PhysicsWorld& world,
                                            const PhysicsRayCastInfo& info,
                                            void* data) -> bool {
        // Ignore player's own collider
        if (info.shape->getBody() == body) {
            return true; // Continue detection
        }

        // Hit wall
        hitWall = true;
        return false; // Stop detection
    }, rayStart, rayEnd, nullptr);

    return hitWall;
}

// ==================== PlayerAnimationSystem ====================

void PlayerAnimationSystem::update(entt::registry& registry, float dt) {
    auto view = registry.view<ecs::PlayerTag,
                              ecs::PlayerAnimationComponent,
                              ecs::PlayerMovementComponent,
                              ecs::PlayerStatsComponent,
                              ecs::PlayerSpriteComponent>();

    // DEBUG: Check how many player entities exist
    static int debugCounter = 0;
    int playerCount = 0;

    for (auto entity : view) {
        playerCount++;

        // Print debug info every 5 seconds
        // if (debugCounter++ % 300 == 0) {
        //     CCLOG("==================== PLAYER DEBUG ====================");
        //     CCLOG("Number of player entities: %d", playerCount);
        //     CCLOG("====================================================");
        // }
        auto& animation = view.get<ecs::PlayerAnimationComponent>(entity);
        auto& movement = view.get<ecs::PlayerMovementComponent>(entity);
        auto& stats = view.get<ecs::PlayerStatsComponent>(entity);
        auto& sprite = view.get<ecs::PlayerSpriteComponent>(entity);

        // Check if playing one-shot animation or special continuous animations
        // One-shot animations (ATTACK, BREAK, PLACE) and special animations (MINE, EAT, DRINK) cannot be interrupted
        bool isSpecialAnimation = (animation.currentState == ecs::PlayerAnimationComponent::AnimState::MINE ||
                                   animation.currentState == ecs::PlayerAnimationComponent::AnimState::EAT ||
                                   animation.currentState == ecs::PlayerAnimationComponent::AnimState::DRINK);

        if (!animation.isPlayingOneShot && !isSpecialAnimation) {
            // Determine which animation should be playing
            auto targetState = determineAnimationState(movement, stats, sprite);

            // Switch to new animation (if state changed)
            if (animation.currentState != targetState) {
                transitionToState(animation, targetState);
            }
        }

        // Update animation time
        animation.animationTime += dt;

        // Play frame animation
        if (!animation.currentFrames.empty() && animation.totalFrames > 0) {
            // Calculate which frame should be displayed
            int targetFrame = static_cast<int>(animation.animationTime / animation.frameTime);

            // Loop playback or finish one-shot animation
            if (targetFrame >= animation.totalFrames) {
                if (animation.isPlayingOneShot) {
                    // One-shot animation finished, return to IDLE
                    animation.isPlayingOneShot = false;
                    targetFrame = animation.totalFrames - 1;  // Keep at last frame
                    // CCLOG("PlayerAnimationSystem: One-shot animation finished, returning to IDLE");

                    // Transition to IDLE state
                    transitionToState(animation, ecs::PlayerAnimationComponent::AnimState::IDLE);
                } else {
                    // Loop animation
                    targetFrame = targetFrame % animation.totalFrames;
                    animation.animationTime = targetFrame * animation.frameTime;
                }
            }

            // Switch frame
            if (targetFrame != animation.currentFrame) {
                // Hide current frame
                if (animation.currentFrame >= 0 &&
                    animation.currentFrame < animation.currentFrames.size()) {
                    animation.currentFrames[animation.currentFrame]->setVisible(false);
                }

                // Show new frame
                animation.currentFrame = targetFrame;
                if (animation.currentFrame >= 0 &&
                    animation.currentFrame < animation.currentFrames.size()) {
                    auto frameSprite = animation.currentFrames[animation.currentFrame];
                    frameSprite->setVisible(true);

                    // Sync position and direction to main sprite
                    if (sprite.sprite) {
                        float bodyHalfHeight = 21.0f;
                        if (registry.all_of<ecs::PhysicsBodyComponent>(entity)) {
                            const auto& phys = registry.get<ecs::PhysicsBodyComponent>(entity);
                            if (phys.shape == ecs::PhysicsBodyComponent::BodyShape::Box && phys.height > 0.0f) {
                                bodyHalfHeight = phys.height * 0.5f;
                            } else if (phys.shape == ecs::PhysicsBodyComponent::BodyShape::Circle && phys.radius > 0.0f) {
                                bodyHalfHeight = phys.radius;
                            }
                        }
                        const auto parentSize = sprite.sprite->getContentSize();
                        const auto parentAnchor = sprite.sprite->getAnchorPoint();
                        const float baseX = parentSize.width * parentAnchor.x;
                        const float baseY = parentSize.height * parentAnchor.y;
                        const float anchorY = frameSprite->getAnchorPoint().y;
                        const float frameHeight = frameSprite->getContentSize().height;
                        const float localY = (-bodyHalfHeight) + frameHeight * anchorY;
                        frameSprite->setPosition(cocos2d::Vec2(baseX, baseY + localY));
                        frameSprite->setFlippedX(sprite.sprite->isFlippedX());
                    }
                }
            } else {
                // Even if not switching frames, still sync position
                if (animation.currentFrame >= 0 &&
                    animation.currentFrame < animation.currentFrames.size() &&
                    sprite.sprite) {
                    auto frameSprite = animation.currentFrames[animation.currentFrame];
                    if (frameSprite) {
                        frameSprite->setVisible(true);
                    }
                    float bodyHalfHeight = 21.0f;
                    if (registry.all_of<ecs::PhysicsBodyComponent>(entity)) {
                        const auto& phys = registry.get<ecs::PhysicsBodyComponent>(entity);
                        if (phys.shape == ecs::PhysicsBodyComponent::BodyShape::Box && phys.height > 0.0f) {
                            bodyHalfHeight = phys.height * 0.5f;
                        } else if (phys.shape == ecs::PhysicsBodyComponent::BodyShape::Circle && phys.radius > 0.0f) {
                            bodyHalfHeight = phys.radius;
                        }
                    }
                    const auto parentSize = sprite.sprite->getContentSize();
                    const auto parentAnchor = sprite.sprite->getAnchorPoint();
                    const float baseX = parentSize.width * parentAnchor.x;
                    const float baseY = parentSize.height * parentAnchor.y;
                    const float anchorY = frameSprite->getAnchorPoint().y;
                    const float frameHeight = frameSprite->getContentSize().height;
                    const float localY = (-bodyHalfHeight) + frameHeight * anchorY;
                    frameSprite->setPosition(cocos2d::Vec2(baseX, baseY + localY));
                    frameSprite->setFlippedX(sprite.sprite->isFlippedX());
                }
            }
        }
    }
}

ecs::PlayerAnimationComponent::AnimState PlayerAnimationSystem::determineAnimationState(
    const ecs::PlayerMovementComponent& movement,
    const ecs::PlayerStatsComponent& stats,
    const ecs::PlayerSpriteComponent& sprite)
{
    // Check if moving
    bool isMoving = std::abs(movement.velocity.x) > 10.0f;

    // Determine animation state
    if (!stats.isOnGround) {
        return ecs::PlayerAnimationComponent::AnimState::FALL;
    } else {
        // On ground
        if (isMoving) {
            return ecs::PlayerAnimationComponent::AnimState::WALK; // Walking
        } else {
            return ecs::PlayerAnimationComponent::AnimState::IDLE; // Idle
        }
    }
}

void PlayerAnimationSystem::hideAllCachedFrames() {
    // Iterate through all animation states and hide their frames
    using AnimState = ecs::PlayerAnimationComponent::AnimState;

    const AnimState allStates[] = {
        AnimState::IDLE,
        AnimState::WALK,
        AnimState::JUMP,
        AnimState::FALL,
        AnimState::BREAK,
        AnimState::PLACE,
        AnimState::ATTACK,
        AnimState::EAT,
        AnimState::DRINK,
        AnimState::MINE
    };

    // DEBUG: Count visible sprites before hiding
    int totalVisible = 0;
    for (auto state : allStates) {
        const auto* anim = PlayerAnimationLoader::getAnimation(state);
        if (anim) {
            for (auto* frame : anim->frames) {
                if (frame && frame->isVisible()) {
                    totalVisible++;
                }
            }
        }
    }

    if (totalVisible > 1) {
        CCLOG("⚠️ WARNING: Found %d visible animation sprites before hiding (should be 0-1)", totalVisible);
    }

    for (auto state : allStates) {
        const auto* anim = PlayerAnimationLoader::getAnimation(state);
        if (anim) {
            for (auto* frame : anim->frames) {
                if (frame) {
                    frame->setVisible(false);
                }
            }
        }
    }
}

void PlayerAnimationSystem::transitionToState(
    ecs::PlayerAnimationComponent& animation,
    ecs::PlayerAnimationComponent::AnimState newState)
{
    // Hide all frames of old animation
    for (auto* frame : animation.currentFrames) {
        if (frame) {
            frame->setVisible(false);
        }
    }

    // CRITICAL FIX: Hide ALL cached animation frames to prevent ghosting
    // This ensures no leftover sprites from previous animations remain visible
    hideAllCachedFrames();

    animation.previousState = animation.currentState;
    animation.currentState = newState;
    animation.animationTime = 0.0f;
    animation.currentFrame = 0;

    // Load frames of new animation
    const auto* newAnim = PlayerAnimationLoader::getAnimation(newState);
    if (newAnim) {
        animation.currentFrames = newAnim->frames;
        animation.totalFrames = newAnim->getFrameCount();
        animation.frameTime = newAnim->frameTime;

        // Show first frame
        if (!animation.currentFrames.empty()) {
            animation.currentFrames[0]->setVisible(true);
        }
    } else {
        CCLOG("PlayerAnimationSystem: Failed to load animation for state %d", static_cast<int>(newState));
        animation.currentFrames.clear();
        animation.totalFrames = 0;
    }

    // Output debug info
    static const char* stateNames[] = {
        "IDLE", "WALK", "RUN", "JUMP", "FALL", "USE_ITEM", "HURT", "DEATH", "SWIM", "BREAK", "PLACE", "ATTACK", "EAT", "DRINK", "MINE"
    };
    CCLOG("Animation state changed: %s -> %s (%d frames)",
          stateNames[static_cast<int>(animation.previousState)],
          stateNames[static_cast<int>(newState)],
          animation.totalFrames);
}

// ==================== PlayerHealthSystem ====================

 static void applyInvincibilityFlickerToAnimation(entt::registry& registry, entt::entity entity);

void PlayerHealthSystem::update(entt::registry& registry, float dt) {
    auto view = registry.view<ecs::PlayerTag,
                              ecs::PlayerStatsComponent,
                              ecs::PlayerAnimationComponent,
                              ecs::PlayerSpriteComponent,
                              ecs::HealthComponent>();

    for (auto entity : view) {
        auto& stats = view.get<ecs::PlayerStatsComponent>(entity);
        auto& sprite = view.get<ecs::PlayerSpriteComponent>(entity);
        auto& health = view.get<ecs::HealthComponent>(entity);

        stats.maxHealth = health.maxHealth;
        stats.currentHealth = health.currentHealth;
        stats.isDead = health.isDead;

        // 1. Process health regeneration
        processHealthRegen(stats, dt);

        health.maxHealth = stats.maxHealth;
        health.currentHealth = std::min(stats.currentHealth, health.maxHealth);
        health.isDead = (health.currentHealth <= 0.0f);
        stats.isDead = health.isDead;

        // 2. Process mana regeneration
        processManaRegen(stats, dt);

        // 3. Update UI
        updateUI(stats, sprite);

        stats.isInvincible = (health.invincibleTimer > 0.0f);
        stats.invincibleTimer = health.invincibleTimer;

        applyInvincibilityFlickerToAnimation(registry, entity);

        // 5. Check death
        if (stats.currentHealth <= 0.0f && !stats.isDead) {
            stats.isDead = true;
            CCLOG("Player died!");
            // TODO: Trigger death logic
        }
    }
}

void PlayerHealthSystem::processHealthRegen(
    ecs::PlayerStatsComponent& stats,
    float dt)
{
    if (stats.isDead || stats.currentHealth >= stats.maxHealth) {
        return;
    }

    stats.regenTimer += dt;

    // Regenerate once per second
    if (stats.regenTimer >= 1.0f) {
        stats.currentHealth += stats.healthRegen;

        // Limit to maximum
        if (stats.currentHealth > stats.maxHealth) {
            stats.currentHealth = stats.maxHealth;
        }

        stats.regenTimer = 0.0f;
    }
}

void PlayerHealthSystem::processManaRegen(
    ecs::PlayerStatsComponent& stats,
    float dt)
{
    if (stats.currentMana >= stats.maxMana) {
        return;
    }

    // Mana regeneration is continuous (unlike health which is once per second)
    stats.currentMana += stats.manaRegen * dt;

    // Limit to maximum
    if (stats.currentMana > stats.maxMana) {
        stats.currentMana = stats.maxMana;
    }
}

void PlayerHealthSystem::updateUI(
    const ecs::PlayerStatsComponent& stats,
    ecs::PlayerSpriteComponent& sprite)
{
    // CRITICAL: Update UI position every frame to follow camera
    // UI elements must move in world coords to stay fixed on screen
    auto scene = PlayerSystemsManager::getScene();
    if (scene) {
        auto camera = scene->getDefaultCamera();
        if (camera && sprite.healthBarBg) {
            auto visibleSize = Director::getInstance()->getVisibleSize();
            Vec3 camPos = camera->getPosition3D();

            // Calculate UI position relative to camera
            float rightMargin = 20.0f;
            float barHeight = 20.0f;
            float barSpacing = 8.0f;

            // UI position = camera position + offset from camera center to screen edge
            float startX = camPos.x + (visibleSize.width / 2.0f - rightMargin);
            float startY = camPos.y + (visibleSize.height / 2.0f - 20.0f);

            // Update all UI bar positions
            if (sprite.healthBarBg) {
                sprite.healthBarBg->setPosition(Vec2(startX, startY));
            }
            if (sprite.manaBarBg) {
                sprite.manaBarBg->setPosition(Vec2(startX, startY - barHeight - barSpacing));
            }
            if (sprite.defenseBarBg) {
                sprite.defenseBarBg->setPosition(Vec2(startX, startY - 2 * (barHeight + barSpacing)));
            }
        }
    }

    // Update health bar
    if (sprite.healthBarFill) {
        float healthPercent = stats.currentHealth / stats.maxHealth;
        sprite.healthBarFill->setScaleX(healthPercent);

        // Update health text label
        if (sprite.healthBarBg) {
            cocos2d::Label* healthLabel = nullptr;
            // Find label by iterating children
            auto& children = sprite.healthBarBg->getChildren();
            for (auto child : children) {
                healthLabel = dynamic_cast<cocos2d::Label*>(child);
                if (healthLabel) {
                    break;
                }
            }
            if (healthLabel) {
                char buffer[32];
                snprintf(buffer, sizeof(buffer), "HP: %.0f/%.0f",
                        stats.currentHealth, stats.maxHealth);
                healthLabel->setString(buffer);
            }
        }
    }

    // Update mana bar
    if (sprite.manaBarFill) {
        float manaPercent = stats.currentMana / stats.maxMana;
        sprite.manaBarFill->setScaleX(manaPercent);

        // Update mana text label
        if (sprite.manaBarBg) {
            cocos2d::Label* manaLabel = nullptr;
            // Find label by iterating children
            auto& children = sprite.manaBarBg->getChildren();
            for (auto child : children) {
                manaLabel = dynamic_cast<cocos2d::Label*>(child);
                if (manaLabel) {
                    break;
                }
            }
            if (manaLabel) {
                char buffer[32];
                snprintf(buffer, sizeof(buffer), "MP: %.0f/%.0f",
                        stats.currentMana, stats.maxMana);
                manaLabel->setString(buffer);
            }
        }
    }

    // Update defense label
    if (sprite.defenseLabel) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "Defense: %d", stats.defense);
        sprite.defenseLabel->setString(buffer);
    }

}

static void applyInvincibilityFlickerToAnimation(entt::registry& registry, entt::entity entity) {
    if (!registry.valid(entity)) {
        return;
    }
    if (!registry.all_of<ecs::PlayerStatsComponent, ecs::PlayerAnimationComponent>(entity)) {
        return;
    }

    const auto& stats = registry.get<ecs::PlayerStatsComponent>(entity);
    auto& animation = registry.get<ecs::PlayerAnimationComponent>(entity);
    cocos2d::Sprite* frame = nullptr;
    if (animation.currentFrame >= 0 && animation.currentFrame < static_cast<int>(animation.currentFrames.size())) {
        frame = animation.currentFrames[animation.currentFrame];
    }
    if (!frame) {
        return;
    }
    if (stats.isInvincible) {
        float opacity = 128.0f + 127.0f * std::sin(stats.invincibleTimer * 20.0f);
        frame->setOpacity(static_cast<GLubyte>(opacity));
    } else {
        frame->setOpacity(255);
    }
}

// ==================== PlayerCameraSystem ====================

void PlayerCameraSystem::update(entt::registry& registry, float dt) {
    if (!PlayerSystemsManager::getScene()) {
        return;  // 没有场景，无法更新摄像机
    }

    // 获取默认摄像机
    auto camera = PlayerSystemsManager::getScene()->getDefaultCamera();
    if (!camera) {
        return;
    }

    // 查找玩家实体
    auto view = registry.view<ecs::PlayerTag, ecs::PlayerStatsComponent, ecs::TransformComponent, ecs::PlayerSpriteComponent>();

    for (auto entity : view) {
        auto& transform = view.get<ecs::TransformComponent>(entity);
        auto& sprite = view.get<ecs::PlayerSpriteComponent>(entity);

        // 获取玩家位置
        Vec2 playerPos = transform.position;
        if (sprite.sprite && sprite.sprite->getPhysicsBody()) {
            playerPos = sprite.sprite->getPhysicsBody()->getPosition();
        }

        // 获取当前摄像机位置
        Vec3 currentCamPos = camera->getPosition3D();
        Vec2 targetPos = playerPos;

        // 完全锁定：每帧直接对齐到玩家位置（玩家居中）
        camera->setPosition3D(Vec3(targetPos.x, targetPos.y, currentCamPos.z));

        // 只处理第一个玩家实体
        break;
    }
}

void PlayerCameraSystem::setFollowSpeed(float speed) {
    s_followSpeed = cocos2d::clampf(speed, 0.0f, 1.0f);
}

float PlayerCameraSystem::getFollowSpeed() {
    return s_followSpeed;
}

// ==================== PlayerSystemsManager ====================

void PlayerSystemsManager::updateAllSystems(entt::registry& registry, float dt) {
    // Execute systems in priority order

    // 1. Input system (priority: 0)
    PlayerInputSystem::update(registry, dt);

    // 2. Ground detection system (priority: 5)
    PlayerGroundDetectionSystem::update(registry, dt);

    // 3. Movement system (priority: 10)
    PlayerMovementSystem::update(registry, dt);

    // 4. Camera system (priority: 200) - ensure UI uses updated camera
    PlayerCameraSystem::update(registry, dt);

    // 5. Health system (priority: 30)
    PlayerHealthSystem::update(registry, dt);

    // 6. Animation system (priority: 100)
    PlayerAnimationSystem::update(registry, dt);
}

void PlayerSystemsManager::setScene(cocos2d::Scene* scene) {
    s_scene = scene;
}

cocos2d::Scene* PlayerSystemsManager::getScene() {
    return s_scene;
}
