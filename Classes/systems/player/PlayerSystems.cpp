#include "PlayerSystems.h"
#include "core/PlayerInput.h"
#include "PlayerAnimationLoader.h"
#include "PlayerInventoryIntegration.h"
#include <cmath>

USING_NS_CC;

// Static member initialization
cocos2d::Scene* PlayerSystemsManager::s_scene = nullptr;

// ==================== PlayerInputSystem ====================

void PlayerInputSystem::update(entt::registry& registry, float dt) {
    auto& input = PlayerInput::getInstance();

    // Iterate over all player entities
    auto view = registry.view<ecs::PlayerTag,
                              ecs::PlayerMovementComponent,
                              ecs::PlayerHotbarComponent>();

    for (auto entity : view) {
        auto& movement = view.get<ecs::PlayerMovementComponent>(entity);
        auto& hotbar = view.get<ecs::PlayerHotbarComponent>(entity);

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

        // Additional debug: display input state
        // static int inputDebugCounter = 0;
        // if (inputDebugCounter++ % 60 == 0) {
        //     CCLOG("Input Debug: MoveLeft=%s, MoveRight=%s, Jump=%s",
        //           movement.isMovingLeft ? "YES" : "NO",
        //           movement.isMovingRight ? "YES" : "NO",
        //           movement.wantsToJump ? "YES" : "NO");
        // }

        // ==================== Handle Hotbar Switching ====================
        // Number keys 1-9 switch hotbar slots
        for (int i = 0; i < 9; i++) {
            auto keyCode = static_cast<EventKeyboard::KeyCode>(
                static_cast<int>(EventKeyboard::KeyCode::KEY_1) + i
            );
            if (input.isKeyJustPressed(keyCode)) {
                hotbar.selectSlot(i);
                CCLOG("Hotbar slot changed to: %d", i);
                break;
            }
        }

        // Number key 0 selects slot 10
        if (input.isKeyJustPressed(EventKeyboard::KeyCode::KEY_0)) {
            hotbar.selectSlot(9);
            CCLOG("Hotbar slot changed to: 9");
        }

        // ==================== Handle Item Usage ====================
        // J key to use current hotbar item
        if (input.isKeyJustPressed(EventKeyboard::KeyCode::KEY_J)) {
            CCLOG("PlayerInputSystem: J key pressed - using hotbar item");
            PlayerInventoryBridge::useCurrentHotbarItem(registry, entity);
        }

        // Mouse left click to use item (alternative to J key)
        if (input.isMouseJustPressed(EventMouse::MouseButton::BUTTON_LEFT)) {
            // Only use item on left click if not clicking on UI
            // TODO: Add UI hit test to prevent using items when clicking UI elements
            CCLOG("PlayerInputSystem: Left click - using hotbar item");
            PlayerInventoryBridge::useCurrentHotbarItem(registry, entity);
        }

        // TODO: Mouse wheel to switch hotbar (need to extend PlayerInput to support mouse wheel)
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
        processHorizontalMovement(movement, stats, dt);

        // 2. Process jump
        processJump(movement, stats, sprite);

        // 3. Sync to physics engine
        syncPhysics(movement, transform, sprite);

        // 4. Update sprite direction
        updateSpriteDirection(movement, sprite);
    }
}

void PlayerMovementSystem::processHorizontalMovement(
    ecs::PlayerMovementComponent& movement,
    ecs::PlayerStatsComponent& stats,
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
        // TODO: Implement double jump (cloud in a bottle, etc.)
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
    ecs::PlayerMovementComponent& movement,
    ecs::TransformComponent& transform,
    ecs::PlayerSpriteComponent& sprite)
{
    if (!sprite.sprite || !sprite.sprite->getPhysicsBody()) {
        return;
    }

    auto body = sprite.sprite->getPhysicsBody();

    // Set horizontal velocity (vertical velocity controlled by physics engine)
    body->setVelocity(Vec2(movement.velocity.x, body->getVelocity().y));

    // Update Transform component
    transform.x = sprite.sprite->getPositionX();
    transform.y = sprite.sprite->getPositionY();

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
                              ecs::PlayerSpriteComponent>();

    for (auto entity : view) {
        auto& stats = view.get<ecs::PlayerStatsComponent>(entity);
        auto& sprite = view.get<ecs::PlayerSpriteComponent>(entity);

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
                stats.isOnGround = raycastGround(sprite.sprite, scene);
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

bool PlayerGroundDetectionSystem::raycastGround(cocos2d::Sprite* sprite, cocos2d::Scene* scene) {
    if (!sprite || !scene) {
        return false;
    }

    auto physicsWorld = scene->getPhysicsWorld();
    if (!physicsWorld) {
        return false;
    }

    Vec2 playerPos = sprite->getPosition();
    Size playerSize = sprite->getContentSize();

    // Raycast slightly downward from player bottom
    Vec2 rayStart = Vec2(playerPos.x, playerPos.y - playerSize.height / 2);
    Vec2 rayEnd = Vec2(playerPos.x, playerPos.y - playerSize.height / 2 - 5.0f); // 5 pixels down

    bool hitGround = false;

    // Execute raycast
    physicsWorld->rayCast([&hitGround, sprite](PhysicsWorld& world,
                                               const PhysicsRayCastInfo& info,
                                               void* data) -> bool {
        // Ignore player's own collider
        if (info.shape->getBody() == sprite->getPhysicsBody()) {
            return true; // Continue detection
        }

        // Hit other object
        hitGround = true;
        return false; // Stop detection
    }, rayStart, rayEnd, nullptr);

    return hitGround;
}

// ==================== PlayerAnimationSystem ====================

void PlayerAnimationSystem::update(entt::registry& registry, float dt) {
    auto view = registry.view<ecs::PlayerTag,
                              ecs::PlayerAnimationComponent,
                              ecs::PlayerMovementComponent,
                              ecs::PlayerStatsComponent,
                              ecs::PlayerSpriteComponent>();

    for (auto entity : view) {
        auto& animation = view.get<ecs::PlayerAnimationComponent>(entity);
        auto& movement = view.get<ecs::PlayerMovementComponent>(entity);
        auto& stats = view.get<ecs::PlayerStatsComponent>(entity);
        auto& sprite = view.get<ecs::PlayerSpriteComponent>(entity);

        // Determine which animation should be playing
        auto targetState = determineAnimationState(movement, stats, sprite);

        // Switch to new animation (if state changed)
        if (animation.currentState != targetState) {
            transitionToState(animation, targetState);
        }

        // Update animation time
        animation.animationTime += dt;

        // Play frame animation
        if (!animation.currentFrames.empty() && animation.totalFrames > 0) {
            // Calculate which frame should be displayed
            int targetFrame = static_cast<int>(animation.animationTime / animation.frameTime);

            // Loop playback
            if (targetFrame >= animation.totalFrames) {
                targetFrame = targetFrame % animation.totalFrames;
                animation.animationTime = targetFrame * animation.frameTime;
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
                        frameSprite->setPosition(sprite.sprite->getPosition());
                        frameSprite->setFlippedX(sprite.sprite->isFlippedX());
                    }
                }
            } else {
                // Even if not switching frames, still sync position
                if (animation.currentFrame >= 0 &&
                    animation.currentFrame < animation.currentFrames.size() &&
                    sprite.sprite) {
                    auto frameSprite = animation.currentFrames[animation.currentFrame];
                    frameSprite->setPosition(sprite.sprite->getPosition());
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

    // Check vertical velocity
    float velocityY = 0.0f;
    if (sprite.sprite && sprite.sprite->getPhysicsBody()) {
        velocityY = sprite.sprite->getPhysicsBody()->getVelocity().y;
    }

    // Determine animation state
    if (!stats.isOnGround) {
        // In air
        if (velocityY > 50.0f) {
            return ecs::PlayerAnimationComponent::AnimState::JUMP; // Rising
        } else {
            return ecs::PlayerAnimationComponent::AnimState::FALL; // Falling
        }
    } else {
        // On ground
        if (isMoving) {
            return ecs::PlayerAnimationComponent::AnimState::WALK; // Walking
        } else {
            return ecs::PlayerAnimationComponent::AnimState::IDLE; // Idle
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
        "IDLE", "WALK", "RUN", "JUMP", "FALL", "USE_ITEM", "HURT", "DEATH", "SWIM"
    };
    CCLOG("Animation state changed: %s -> %s (%d frames)",
          stateNames[static_cast<int>(animation.previousState)],
          stateNames[static_cast<int>(newState)],
          animation.totalFrames);
}

// ==================== PlayerHealthSystem ====================

void PlayerHealthSystem::update(entt::registry& registry, float dt) {
    auto view = registry.view<ecs::PlayerTag,
                              ecs::PlayerStatsComponent,
                              ecs::PlayerSpriteComponent>();

    for (auto entity : view) {
        auto& stats = view.get<ecs::PlayerStatsComponent>(entity);
        auto& sprite = view.get<ecs::PlayerSpriteComponent>(entity);

        // 1. Process health regeneration
        processHealthRegen(stats, dt);

        // 2. Process mana regeneration
        processManaRegen(stats, dt);

        // 3. Update UI
        updateUI(stats, sprite);

        // 4. Update invincibility frames
        if (stats.isInvincible) {
            stats.invincibleTimer -= dt;
            if (stats.invincibleTimer <= 0.0f) {
                stats.isInvincible = false;
                CCLOG("Player invincibility ended");
            }
        }

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
    // Update health bar
    if (sprite.healthBarFill) {
        float healthPercent = stats.currentHealth / stats.maxHealth;
        sprite.healthBarFill->setScaleX(healthPercent);
    }

    // Update mana bar
    if (sprite.manaBarFill) {
        float manaPercent = stats.currentMana / stats.maxMana;
        sprite.manaBarFill->setScaleX(manaPercent);
    }

    // Flicker effect during invincibility frames
    if (sprite.sprite) {
        if (stats.isInvincible) {
            // Use sin wave for flicker effect
            float opacity = 128.0f + 127.0f * std::sin(stats.invincibleTimer * 20.0f);
            sprite.sprite->setOpacity(static_cast<GLubyte>(opacity));
        } else {
            sprite.sprite->setOpacity(255);
        }
    }
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

    // 4. Health system (priority: 30)
    PlayerHealthSystem::update(registry, dt);

    // 5. Animation system (priority: 100)
    PlayerAnimationSystem::update(registry, dt);
}

void PlayerSystemsManager::setScene(cocos2d::Scene* scene) {
    s_scene = scene;
}

cocos2d::Scene* PlayerSystemsManager::getScene() {
    return s_scene;
}
