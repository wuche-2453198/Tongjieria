#include "PlayerSystems.h"
#include "core/PlayerInput.h"
#include "PlayerAnimationLoader.h"
#include "PlayerInventoryIntegration.h"
#include "systems/items/ItemManager.h"
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

        // ==================== Handle Item Usage ====================
        // J key to use current hotbar item
        if (input.isKeyJustPressed(EventKeyboard::KeyCode::KEY_J)) {
            CCLOG("PlayerInputSystem: J key pressed - using hotbar item");
            PlayerInventoryBridge::useCurrentHotbarItem(registry, entity);
        }

        // ==================== Handle Item Dropping ====================
        // Q key to drop current hotbar item
        if (input.isActionJustPressed("DropItem")) {
            int currentInvIndex = hotbar.getCurrentInventoryIndex();
            auto* inventory = Inventory::getInstance();

            InventorySlot droppedItem = inventory->dropItem(currentInvIndex);

            if (droppedItem.itemId != 0) {
                auto* itemMgr = ItemManager::getInstance();
                auto itemData = itemMgr->getItemData(droppedItem.itemId);

                CCLOG("========================================");
                CCLOG("PlayerInputSystem: Q key pressed - DROPPED ITEM");
                if (itemData) {
                    CCLOG("Item: %s (ID: %d)", itemData->name.c_str(), droppedItem.itemId);
                } else {
                    CCLOG("Item ID: %d", droppedItem.itemId);
                }
                CCLOG("Count: %d", droppedItem.count);
                CCLOG("From slot: %d", currentInvIndex);
                CCLOG("TODO: Spawn item entity in world");
                CCLOG("========================================");
            } else {
                CCLOG("PlayerInputSystem: Q key pressed - No item to drop");
            }
        }

        // ==================== Handle Block Interaction ====================
        // Mouse left click - break/destroy blocks (debug mode)
        if (input.isMouseJustPressed(EventMouse::MouseButton::BUTTON_LEFT)) {
            // Get mouse position (world coordinates for block interaction)
            Vec2 mouseWorldPos = input.getMouseWorldPosition();
            Vec2 mouseScreenPos = input.getMouseScreenPosition();

            // TODO: Add UI hit test to prevent interaction when clicking UI elements
            // For now, output debug info
            CCLOG("========================================");
            CCLOG("PlayerInputSystem: LEFT CLICK - BREAK BLOCK");
            CCLOG("Mouse screen position: (%.1f, %.1f)", mouseScreenPos.x, mouseScreenPos.y);
            CCLOG("Mouse world position: (%.1f, %.1f)", mouseWorldPos.x, mouseWorldPos.y);
            CCLOG("TODO: Implement block breaking logic");
            CCLOG("========================================");

            // Trigger break animation using transitionToState to properly hide previous frames
            animation.isPlayingOneShot = true;
            PlayerAnimationSystem::transitionToState(animation, ecs::PlayerAnimationComponent::AnimState::BREAK);
            CCLOG("PlayerInputSystem: Triggered BREAK animation with %d frames", animation.totalFrames);

            // Also use hotbar item (existing functionality)
            PlayerInventoryBridge::useCurrentHotbarItem(registry, entity);
        }

        // Mouse right click - use weapon / place blocks
        if (input.isMouseJustPressed(EventMouse::MouseButton::BUTTON_RIGHT)) {
            // Get mouse position (world coordinates for block placement)
            Vec2 mouseWorldPos = input.getMouseWorldPosition();
            Vec2 mouseScreenPos = input.getMouseScreenPosition();

            // Get current hotbar item
            int currentItemId = PlayerInventoryBridge::getCurrentHotbarItemId(registry, entity);

            CCLOG("========================================");
            CCLOG("PlayerInputSystem: RIGHT CLICK");
            CCLOG("Mouse screen position: (%.1f, %.1f)", mouseScreenPos.x, mouseScreenPos.y);
            CCLOG("Mouse world position: (%.1f, %.1f)", mouseWorldPos.x, mouseWorldPos.y);
            CCLOG("Current hotbar item ID: %d", currentItemId);

            // Check if holding a weapon
            bool isWeapon = false;
            if (currentItemId > 0) {
                auto* itemMgr = ItemManager::getInstance();
                const auto* itemData = itemMgr->getItemData(currentItemId);
                if (itemData) {
                    // Check if item has weapon tag (tag 2 or 201)
                    for (int tag : itemData->tags) {
                        if (tag == 2 || tag == 201) {
                            isWeapon = true;
                            break;
                        }
                    }
                    CCLOG("Item: %s, Tags count: %d, Is Weapon: %s",
                          itemData->name.c_str(),
                          (int)itemData->tags.size(),
                          isWeapon ? "YES" : "NO");
                }
            }

            if (isWeapon) {
                // Trigger weapon swing animation using transitionToState
                CCLOG("WEAPON DETECTED - Triggering WEAPON_SWING animation");
                animation.isPlayingOneShot = true;
                PlayerAnimationSystem::transitionToState(animation, ecs::PlayerAnimationComponent::AnimState::WEAPON_SWING);
                CCLOG("PlayerInputSystem: Triggered WEAPON_SWING animation with %d frames", animation.totalFrames);
            } else {
                // Trigger place animation using transitionToState
                CCLOG("NO WEAPON - Triggering PLACE animation");
                animation.isPlayingOneShot = true;
                PlayerAnimationSystem::transitionToState(animation, ecs::PlayerAnimationComponent::AnimState::PLACE);
                CCLOG("PlayerInputSystem: Triggered PLACE animation with %d frames", animation.totalFrames);
            }
            CCLOG("========================================");
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

    // DEBUG: Check how many player entities exist
    static int debugCounter = 0;
    int playerCount = 0;

    for (auto entity : view) {
        playerCount++;

        // Print debug info every 5 seconds
        if (debugCounter++ % 300 == 0) {
            CCLOG("==================== PLAYER DEBUG ====================");
            CCLOG("Number of player entities: %d", playerCount);
            CCLOG("====================================================");
        }
        auto& animation = view.get<ecs::PlayerAnimationComponent>(entity);
        auto& movement = view.get<ecs::PlayerMovementComponent>(entity);
        auto& stats = view.get<ecs::PlayerStatsComponent>(entity);
        auto& sprite = view.get<ecs::PlayerSpriteComponent>(entity);

        // Check if playing one-shot animation (weapon swing, etc.)
        // One-shot animations cannot be interrupted
        if (!animation.isPlayingOneShot) {
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
                    CCLOG("PlayerAnimationSystem: One-shot animation finished, returning to IDLE");

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
        AnimState::WEAPON_SWING
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
        "IDLE", "WALK", "RUN", "JUMP", "FALL", "USE_ITEM", "HURT", "DEATH", "SWIM", "BREAK", "PLACE", "WEAPON_SWING"
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
