#include "PlayerSystems.h"
#include "PlayerInput.h"
#include <cmath>

USING_NS_CC;

// 静态成员初始化
cocos2d::Scene* PlayerSystemsManager::s_scene = nullptr;

// ==================== PlayerInputSystem ====================

void PlayerInputSystem::update(entt::registry& registry, float dt) {
    auto& input = PlayerInput::getInstance();

    // 遍历所有玩家实体
    auto view = registry.view<ecs::PlayerTag,
                              ecs::PlayerMovementComponent,
                              ecs::PlayerHotbarComponent>();

    for (auto entity : view) {
        auto& movement = view.get<ecs::PlayerMovementComponent>(entity);
        auto& hotbar = view.get<ecs::PlayerHotbarComponent>(entity);

        // ==================== 更新移动输入 ====================
        movement.isMovingLeft = input.isActionPressed("MoveLeft");
        movement.isMovingRight = input.isActionPressed("MoveRight");

        // 详细的跳跃输入调试
        bool jumpPressed = input.isActionJustPressed("Jump");
        bool spacePressed = input.isKeyJustPressed(EventKeyboard::KeyCode::KEY_SPACE);

        if (spacePressed) {
            CCLOG(">>> SPACE KEY PRESSED! (direct check)");
        }
        if (jumpPressed) {
            CCLOG(">>> JUMP ACTION TRIGGERED! (action mapping check)");
        }

        movement.wantsToJump = jumpPressed;

        // 额外调试：显示输入状态
        static int inputDebugCounter = 0;
        if (inputDebugCounter++ % 60 == 0) {
            CCLOG("Input Debug: MoveLeft=%s, MoveRight=%s, Jump=%s",
                  movement.isMovingLeft ? "YES" : "NO",
                  movement.isMovingRight ? "YES" : "NO",
                  movement.wantsToJump ? "YES" : "NO");
        }

        // ==================== 处理快捷栏切换 ====================
        // 数字键1-9切换快捷栏
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

        // 数字键0选择第10格
        if (input.isKeyJustPressed(EventKeyboard::KeyCode::KEY_0)) {
            hotbar.selectSlot(9);
            CCLOG("Hotbar slot changed to: 9");
        }

        // TODO: 鼠标滚轮切换快捷栏（需要扩展 PlayerInput 支持鼠标滚轮）
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

        // 1. 处理水平移动
        processHorizontalMovement(movement, stats, dt);

        // 2. 处理跳跃
        processJump(movement, stats, sprite);

        // 3. 同步到物理引擎
        syncPhysics(movement, transform, sprite);

        // 4. 更新精灵方向
        updateSpriteDirection(movement, sprite);
    }
}

void PlayerMovementSystem::processHorizontalMovement(
    ecs::PlayerMovementComponent& movement,
    ecs::PlayerStatsComponent& stats,
    float dt)
{
    float targetVelX = 0.0f;

    // 确定目标速度
    if (movement.isMovingRight) {
        targetVelX = stats.moveSpeed;
        movement.isFacingRight = true;
    }
    if (movement.isMovingLeft) {
        targetVelX = -stats.moveSpeed;
        movement.isFacingRight = false;
    }

    // 泰拉瑞亚风格加速移动
    if (targetVelX != 0) {
        // 加速
        float accel = movement.accelerationRate * dt;
        movement.velocity.x += (targetVelX - movement.velocity.x) * accel * 0.1f;
    } else {
        // 摩擦力减速
        float frictionCoeff = stats.isOnGround ? movement.friction : movement.airResistance;
        movement.velocity.x *= frictionCoeff;

        // 速度很小时直接停止
        if (std::abs(movement.velocity.x) < 1.0f) {
            movement.velocity.x = 0.0f;
        }
    }

    // 限制最大水平速度
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

    // 重置跳跃请求
    movement.wantsToJump = false;

    // 检查是否可以跳跃
    if (!stats.isOnGround) {
        CCLOG("Jump rejected: player not on ground");
        // TODO: 实现多段跳（云瓶等）
        // if (stats.currentJumpCount < stats.extraJumps) { ... }
        return;
    }

    // 执行跳跃
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

    // 设置水平速度（垂直速度由物理引擎控制）
    body->setVelocity(Vec2(movement.velocity.x, body->getVelocity().y));

    // 更新 Transform 组件
    transform.x = sprite.sprite->getPositionX();
    transform.y = sprite.sprite->getPositionY();

    // 更新 Movement 组件的垂直速度（用于调试和动画）
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

        // 记录之前的状态
        bool wasOnGround = stats.isOnGround;

        // 改进的地面检测逻辑
        if (velocityY > 50.0f) {
            // 正在快速上升（刚跳跃）
            stats.isOnGround = false;
        } else {
            // 其他情况都使用射线检测
            auto scene = PlayerSystemsManager::getScene();
            if (scene) {
                stats.isOnGround = raycastGround(sprite.sprite, scene);
            } else {
                // 降级方案：使用速度检测
                // 垂直速度接近0且不在上升，可能在地面
                stats.isOnGround = (velocityY >= -10.0f && velocityY <= 10.0f);
            }
        }

        // 着地时重置跳跃计数
        if (!wasOnGround && stats.isOnGround) {
            stats.currentJumpCount = 0;
            CCLOG("Player landed on ground");
        }

        // 调试输出（每60帧输出一次，避免日志过多）
        static int debugCounter = 0;
        if (debugCounter++ % 60 == 0) {
            CCLOG("Ground Detection: isOnGround=%s, velocityY=%.1f",
                  stats.isOnGround ? "YES" : "NO", velocityY);
        }
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

    // 从玩家底部稍微往下射线检测
    Vec2 rayStart = Vec2(playerPos.x, playerPos.y - playerSize.height / 2);
    Vec2 rayEnd = Vec2(playerPos.x, playerPos.y - playerSize.height / 2 - 5.0f); // 向下5像素

    bool hitGround = false;

    // 执行射线检测
    physicsWorld->rayCast([&hitGround, sprite](PhysicsWorld& world,
                                               const PhysicsRayCastInfo& info,
                                               void* data) -> bool {
        // 忽略玩家自己的碰撞体
        if (info.shape->getBody() == sprite->getPhysicsBody()) {
            return true; // 继续检测
        }

        // 碰到其他物体
        hitGround = true;
        return false; // 停止检测
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

        // 确定当前应该播放的动画
        auto targetState = determineAnimationState(movement, stats, sprite);

        // 切换到新动画（如果状态改变）
        if (animation.currentState != targetState) {
            transitionToState(animation, targetState);
        }

        // 更新动画时间
        animation.animationTime += dt;

        // TODO: 实际播放动画帧（需要加载动画资源）
        // 目前使用单张图片，暂时跳过帧动画
    }
}

ecs::PlayerAnimationComponent::AnimState PlayerAnimationSystem::determineAnimationState(
    const ecs::PlayerMovementComponent& movement,
    const ecs::PlayerStatsComponent& stats,
    const ecs::PlayerSpriteComponent& sprite)
{
    // 检查是否在移动
    bool isMoving = std::abs(movement.velocity.x) > 10.0f;

    // 检查垂直速度
    float velocityY = 0.0f;
    if (sprite.sprite && sprite.sprite->getPhysicsBody()) {
        velocityY = sprite.sprite->getPhysicsBody()->getVelocity().y;
    }

    // 确定动画状态
    if (!stats.isOnGround) {
        // 空中状态
        if (velocityY > 50.0f) {
            return ecs::PlayerAnimationComponent::AnimState::JUMP; // 上升
        } else {
            return ecs::PlayerAnimationComponent::AnimState::FALL; // 下降
        }
    } else {
        // 地面状态
        if (isMoving) {
            return ecs::PlayerAnimationComponent::AnimState::WALK; // 行走
        } else {
            return ecs::PlayerAnimationComponent::AnimState::IDLE; // 待机
        }
    }
}

void PlayerAnimationSystem::transitionToState(
    ecs::PlayerAnimationComponent& animation,
    ecs::PlayerAnimationComponent::AnimState newState)
{
    animation.previousState = animation.currentState;
    animation.currentState = newState;
    animation.animationTime = 0.0f;
    animation.currentFrame = 0;

    // 输出调试信息
    static const char* stateNames[] = {
        "IDLE", "WALK", "RUN", "JUMP", "FALL", "USE_ITEM", "HURT", "DEATH", "SWIM"
    };
    CCLOG("Animation state changed: %s -> %s",
          stateNames[static_cast<int>(animation.previousState)],
          stateNames[static_cast<int>(newState)]);
}

// ==================== PlayerHealthSystem ====================

void PlayerHealthSystem::update(entt::registry& registry, float dt) {
    auto view = registry.view<ecs::PlayerTag,
                              ecs::PlayerStatsComponent,
                              ecs::PlayerSpriteComponent>();

    for (auto entity : view) {
        auto& stats = view.get<ecs::PlayerStatsComponent>(entity);
        auto& sprite = view.get<ecs::PlayerSpriteComponent>(entity);

        // 1. 处理生命回复
        processHealthRegen(stats, dt);

        // 2. 处理魔法回复
        processManaRegen(stats, dt);

        // 3. 更新UI
        updateUI(stats, sprite);

        // 4. 更新无敌帧
        if (stats.isInvincible) {
            stats.invincibleTimer -= dt;
            if (stats.invincibleTimer <= 0.0f) {
                stats.isInvincible = false;
                CCLOG("Player invincibility ended");
            }
        }

        // 5. 检查死亡
        if (stats.currentHealth <= 0.0f && !stats.isDead) {
            stats.isDead = true;
            CCLOG("Player died!");
            // TODO: 触发死亡逻辑
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

    // 每秒回复一次
    if (stats.regenTimer >= 1.0f) {
        stats.currentHealth += stats.healthRegen;

        // 限制最大值
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

    // 魔法回复是持续的（不像生命是每秒一次）
    stats.currentMana += stats.manaRegen * dt;

    // 限制最大值
    if (stats.currentMana > stats.maxMana) {
        stats.currentMana = stats.maxMana;
    }
}

void PlayerHealthSystem::updateUI(
    const ecs::PlayerStatsComponent& stats,
    ecs::PlayerSpriteComponent& sprite)
{
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

    // 无敌帧时闪烁效果
    if (sprite.sprite) {
        if (stats.isInvincible) {
            // 使用 sin 波实现闪烁
            float opacity = 128.0f + 127.0f * std::sin(stats.invincibleTimer * 20.0f);
            sprite.sprite->setOpacity(static_cast<GLubyte>(opacity));
        } else {
            sprite.sprite->setOpacity(255);
        }
    }
}

// ==================== PlayerSystemsManager ====================

void PlayerSystemsManager::updateAllSystems(entt::registry& registry, float dt) {
    // 按优先级顺序执行系统

    // 1. 输入系统（优先级：0）
    PlayerInputSystem::update(registry, dt);

    // 2. 地面检测系统（优先级：5）
    PlayerGroundDetectionSystem::update(registry, dt);

    // 3. 移动系统（优先级：10）
    PlayerMovementSystem::update(registry, dt);

    // 4. 生命系统（优先级：30）
    PlayerHealthSystem::update(registry, dt);

    // 5. 动画系统（优先级：100）
    PlayerAnimationSystem::update(registry, dt);
}

void PlayerSystemsManager::setScene(cocos2d::Scene* scene) {
    s_scene = scene;
}

cocos2d::Scene* PlayerSystemsManager::getScene() {
    return s_scene;
}
