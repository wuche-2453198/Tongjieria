#pragma once

#include <entt/entt.hpp>
#include "Components.h"
#include "SpriteComponent.h"
#include "System.h"  // 使用System.h中的SystemPriority定义
#include <vector>
#include <memory>
#include <algorithm>

/**
 * @file SystemsEntt.h
 * @brief EnTT版本的System实现
 * 
 * 渐进式迁移策略：
 * 1. 保留ISystemEntt基类接口（兼容原有结构）
 * 2. 将World替换为Registry
 * 3. 逐个迁移System到此文件
 */

namespace ecs {

// SystemPriority已在System.h中定义，无需重复

// ==================== EnTT System基类 ====================

/**
 * @brief EnTT版System基类
 * 
 * 与原ISystem接口兼容，方便渐进式迁移
 */
class ISystemEntt {
protected:
    entt::registry* _registry = nullptr;

public:
    virtual ~ISystemEntt() = default;

    /**
     * @brief 获取系统名称
     */
    virtual const char* getName() const = 0;

    /**
     * @brief 获取系统优先级（数值越小越先执行）
     */
    virtual int getPriority() const { return 0; }

    /**
     * @brief 每帧更新
     * @param delta 帧时间（秒）
     */
    virtual void update(float delta) = 0;

    /**
     * @brief 设置Registry
     */
    void setRegistry(entt::registry* registry) {
        _registry = registry;
    }

    /**
     * @brief 获取Registry
     */
    entt::registry* getRegistry() {
        return _registry;
    }
};

// ==================== 生命值系统（EnTT版本） ====================

/**
 * @brief 生命值系统 - 处理无敌时间和死亡
 * 
 * 迁移说明：
 * - 原：_world->forEach<HealthComponent>(λ)
 * - 新：registry.view<HealthComponent>().each(λ)
 */
class HealthSystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "HealthSystem"; }
    int getPriority() const override { return SystemPriority::COLLISION + 50; }

    void update(float delta) override {
        // EnTT方式：创建view并迭代
        auto view = _registry->view<HealthComponent>();
        
        view.each([delta](auto entity, HealthComponent& health) {
            // 更新无敌时间
            if (health.invincibleTimer > 0) {
                health.invincibleTimer -= delta;
            }
        });
    }
};

// ==================== 战斗系统（EnTT版本） ====================

/**
 * @brief 战斗系统 - 处理攻击冷却
 */
class CombatSystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "CombatSystem"; }
    int getPriority() const override { return SystemPriority::COLLISION + 100; }

    void update(float delta) override {
        auto view = _registry->view<CombatComponent>();
        
        view.each([delta](auto entity, CombatComponent& combat) {
            if (combat.attackTimer > 0) {
                combat.attackTimer -= delta;
            }
        });
    }
};

// ==================== 生命周期系统（EnTT版本） ====================

/**
 * @brief 生命周期系统 - 处理限时实体
 */
class LifetimeSystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "LifetimeSystem"; }
    int getPriority() const override { return SystemPriority::COLLISION + 150; }

    void update(float delta) override {
        // 收集需要销毁的实体
        std::vector<entt::entity> toDestroy;
        
        auto view = _registry->view<LifetimeComponent>();
        view.each([&](auto entity, LifetimeComponent& comp) {
            comp.elapsed += delta;
            
            if (comp.elapsed >= comp.lifetime) {
                toDestroy.push_back(entity);
            }
        });
        
        // 延迟销毁（避免迭代中修改容器）
        for (auto entity : toDestroy) {
            _registry->destroy(entity);
        }
    }
};

// ==================== 怪物动画系统（EnTT版本） ====================

/**
 * @brief 怪物精灵动画系统 - 更新MonsterSpriteComponent的帧动画
 */
class MonsterAnimationSystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "MonsterAnimationSystem"; }
    int getPriority() const override { return SystemPriority::ANIMATION; }

    void update(float delta) override {
        auto view = _registry->view<MonsterSpriteComponent>();
        
        view.each([delta](auto entity, MonsterSpriteComponent& sprite) {
            sprite.updateAnimation(delta);
        });
    }
};

// ==================== 史莱姆渲染系统（EnTT版本） ====================

/**
 * @brief 史莱姆渲染系统 - 同步Transform到SlimeSprite
 */
class SlimeRenderSystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "SlimeRenderSystem"; }
    int getPriority() const override { return SystemPriority::RENDER; }

    void update(float delta) override {
        auto view = _registry->view<TransformComponent, SlimeSpriteComponent>();
        
        view.each([delta](auto entity, TransformComponent& transform, 
                     SlimeSpriteComponent& slimeSprite) {
            if (!slimeSprite.sprite)
                return;

            // 注意：不要手动同步位置！物理引擎会自动更新精灵位置
            // 只从精灵读取位置到transform供其他系统使用
            if (auto* body = slimeSprite.sprite->getPhysicsBody()) {
                transform.position = slimeSprite.sprite->getPosition();
            }

            // 同步显示属性
            slimeSprite.sprite->setRotation(transform.rotation);
            slimeSprite.sprite->setVisible(slimeSprite.visible);
            slimeSprite.sprite->setColor(slimeSprite.color);
            slimeSprite.sprite->setOpacity(slimeSprite.opacity);
            
            // 播放动画
            if (slimeSprite.animationLoaded && slimeSprite.animFrames.size() >= 2) {
                slimeSprite.frameTimer += delta;
                if (slimeSprite.frameTimer >= slimeSprite.frameTime) {
                    slimeSprite.frameTimer -= slimeSprite.frameTime;
                    slimeSprite.currentFrameIndex = (slimeSprite.currentFrameIndex + 1) % slimeSprite.animFrames.size();
                    slimeSprite.sprite->setSpriteFrame(slimeSprite.animFrames.at(slimeSprite.currentFrameIndex));
                }
            }
        });
    }
};

// ==================== 减益系统（EnTT版本） ====================

/**
 * @brief 减益效果系统 - 处理冷冻、冰冻和中毒减益的计时
 */
class DebuffSystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "DebuffSystem"; }
    int getPriority() const override { return SystemPriority::AI - 5; }

    void update(float delta) override {
        auto view = _registry->view<DebuffComponent>();
        
        view.each([delta, this](auto entity, DebuffComponent& debuff) {
            // 更新冷冻计时器
            if (debuff.hasChillDebuff) {
                debuff.chillTimer += delta;
                if (debuff.chillTimer >= debuff.chillDuration) {
                    debuff.hasChillDebuff = false;
                    CCLOG("Entity %u: Chill debuff expired", entt::to_integral(entity));
                }
            }
            
            // 更新冰冻计时器
            if (debuff.hasFreezeDebuff) {
                debuff.freezeTimer += delta;
                if (debuff.freezeTimer >= debuff.freezeDuration) {
                    debuff.hasFreezeDebuff = false;
                    CCLOG("Entity %u: Freeze debuff expired", entt::to_integral(entity));
                }
            }
            
            // 更新中毒计时器和伤害
            if (debuff.hasPoisonDebuff) {
                debuff.poisonTimer += delta;
                debuff.poisonTickTimer += delta;
                
                // 每秒造成一次伤害
                if (debuff.poisonTickTimer >= 1.0f) {
                    debuff.poisonTickTimer -= 1.0f;
                    
                    // 对实体造成毒素伤害
                    if (auto* health = _registry->try_get<HealthComponent>(entity)) {
                        health->takeDamage(debuff.poisonDamagePerSecond);
                        CCLOG("Entity %u: Poison tick %.1f damage (%.1fs remaining)", 
                              entt::to_integral(entity), 
                              debuff.poisonDamagePerSecond,
                              debuff.poisonDuration - debuff.poisonTimer);
                    }
                }
                
                if (debuff.poisonTimer >= debuff.poisonDuration) {
                    debuff.hasPoisonDebuff = false;
                    CCLOG("Entity %u: Poison debuff expired", entt::to_integral(entity));
                }
            }
        });
    }
};

// ==================== 史莱姆同步系统（EnTT版本） ====================

/**
 * @brief 史莱姆同步系统 - 从精灵读取位置到Transform（物理引擎驱动精灵位置）
 */
class SlimeSyncSystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "SlimeSyncSystem"; }
    int getPriority() const override { return SystemPriority::RENDER - 10; }

    void update(float delta) override {
        auto view = _registry->view<TransformComponent, SlimeSpriteComponent>();
        
        view.each([delta](auto entity, TransformComponent& transform,
                         SlimeSpriteComponent& sprite) {
            if (!sprite.sprite)
                return;

            // 物理引擎自动更新精灵位置，我们只需要读取它
            transform.position = sprite.sprite->getPosition();
        });
    }
};

// ==================== 怪物同步系统（EnTT版本） ====================

/**
 * @brief 怪物同步系统 - 同步Transform位置到精灵（从物理体读取）
 */
class MonsterSyncSystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "MonsterSyncSystem"; }
    int getPriority() const override { return SystemPriority::RENDER - 10; }

    void update(float delta) override {
        auto view = _registry->view<TransformComponent, MonsterSpriteComponent>();
        
        view.each([this, delta](auto entity, TransformComponent& transform,
                               MonsterSpriteComponent& sprite) {
            if (!sprite.sprite)
                return;

            auto* body = sprite.getPhysicsBody();
            if (!body)
                return;

            // performRaycastCorrection(body, delta); // 原版中已禁用
            // 从精灵位置同步到Transform（物理体驱动）
            transform.position = body->getPosition();
        });
    }
};

// ==================== 地面检测系统（EnTT版本） ====================

/**
 * @brief 地面检测系统 - 根据物理体速度判断地面和静止状态
 */
class GroundDetectorSystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "GroundDetectorSystem"; }
    int getPriority() const override { return SystemPriority::PHYSICS + 10; }

    void update(float delta) override {
        auto view = _registry->view<GroundDetectorComponent, SlimeSpriteComponent>();
        
        view.each([](auto entity, GroundDetectorComponent& ground,
                    SlimeSpriteComponent& sprite) {
            cocos2d::Vec2 velocity = sprite.getVelocity();

            // 判断是否静止
            ground.isStill = std::abs(velocity.x) < ground.stillThreshold &&
                           std::abs(velocity.y) < ground.stillThreshold;
        });
    }
};

// ==================== 怪物地面检测系统（EnTT版本） ====================

/**
 * @brief 怪物地面检测系统 - 根据物理体速度判断MonsterSpriteComponent的地面状态
 */
class MonsterGroundDetectorSystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "MonsterGroundDetectorSystem"; }
    int getPriority() const override { return SystemPriority::PHYSICS + 10; }

    void update(float delta) override {
        auto view = _registry->view<GroundDetectorComponent, MonsterSpriteComponent>();
        
        view.each([](auto entity, GroundDetectorComponent& ground,
                    MonsterSpriteComponent& sprite) {
            cocos2d::Vec2 velocity = sprite.getVelocity();

            // 判断是否静止
            ground.isStill = std::abs(velocity.x) < ground.stillThreshold &&
                           std::abs(velocity.y) < ground.stillThreshold;
        });
    }
};

// ==================== 缓降系统（EnTT版本） ====================

/**
 * @brief 缓降系统 - 处理伞史莱姆等下落时的空气阻力
 * 
 * 当实体下落时，限制其最大下落速度，模拟撑伞的效果
 */
class SlowFallSystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "SlowFallSystem"; }
    int getPriority() const override { return SystemPriority::PHYSICS + 1; }

    void update(float delta) override {
        auto view = _registry->view<SlowFallComponent, SlimeSpriteComponent>();
        
        view.each([](auto entity, SlowFallComponent& slowFall, 
                    SlimeSpriteComponent& sprite) {
            if (!slowFall.isActive || !sprite.sprite)
                return;
            
            auto* body = sprite.sprite->getPhysicsBody();
            if (!body)
                return;
            
            cocos2d::Vec2 velocity = body->getVelocity();
            
            // 只在下落时应用缓降效果（velocity.y < 0 表示向下）
            if (velocity.y < 0) {
                bool modified = false;
                
                // 垂直方向：限制最大下落速度
                float maxFall = -slowFall.maxFallSpeed;
                if (velocity.y < maxFall) {
                    // 应用阻尼，逐渐减速到最大下落速度
                    velocity.y = velocity.y * slowFall.fallDamping;
                    if (velocity.y < maxFall) {
                        velocity.y = maxFall;
                    }
                    modified = true;
                }
                
                // 水平方向：施加空气阻力
                if (std::abs(velocity.x) > 10.0f) {
                    velocity.x = velocity.x * slowFall.horizontalDamping;
                    modified = true;
                }
                
                if (modified) {
                    body->setVelocity(velocity);
                }
            }
        });
    }
};

// ==================== 仇恨检测系统（EnTT版本） ====================

/**
 * @brief 仇恨检测系统 - 检测目标并更新仇恨状态
 * 
 * EnTT优化：使用view直接获取玩家实体，O(1)复杂度
 * 原版：遍历所有实体查找tag，O(N)复杂度
 */
class AggroSystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "AggroSystem"; }
    int getPriority() const override { return SystemPriority::AI - 10; }

    void update(float delta) override {
        // 遍历所有具有仇恨组件的实体
        auto aggroView = _registry->view<AggroComponent, TransformComponent>();
        
        aggroView.each([this](auto entity, AggroComponent& aggro, 
                             TransformComponent& transform) {
            // EnTT高效查找：根据targetTag直接获取目标实体
            entt::entity target = findTargetByTag(aggro.targetTag);
            // 转换entt::entity到EntityId（都是uint32_t）
            aggro.targetEntity = (target == entt::null) ? INVALID_ENTITY : entt::to_integral(target);

            if (target == entt::null) {
                aggro.distanceToTarget = 99999.0f;
                aggro.directionToTarget = cocos2d::Vec2::ZERO;
                if (aggro.hasAggro) {
                    aggro.hasAggro = false;
                    CCLOG("Entity %u: Lost target, exiting aggro", entt::to_integral(entity));
                }
                return;
            }

            // 计算到目标的距离和方向
            auto* targetTransform = _registry->try_get<TransformComponent>(target);
            if (!targetTransform)
                return;

            cocos2d::Vec2 diff = targetTransform->position - transform.position;
            aggro.distanceToTarget = diff.length();
            aggro.directionToTarget = diff.getNormalized();

            // 仇恨状态切换
            if (aggro.shouldEnterAggro()) {
                aggro.hasAggro = true;
                CCLOG("Entity %u: Target in range (%.1f <= %.1f), entering aggro",
                      entt::to_integral(entity), aggro.distanceToTarget, aggro.aggroRange);
            } else if (aggro.shouldExitAggro()) {
                aggro.hasAggro = false;
                CCLOG("Entity %u: Target too far (%.1f > %.1f), exiting aggro",
                      entt::to_integral(entity), aggro.distanceToTarget, aggro.deaggroRange);
            }
        });
    }

private:
    /**
     * @brief 根据tag查找目标实体（EnTT高效版本）
     * 
     * 性能对比：
     * - 旧版：O(N) 遍历所有实体
     * - EnTT：O(1) 直接从view获取
     */
    entt::entity findTargetByTag(const std::string& tag) {
        // 目前只支持查找玩家
        if (tag == "Player") {
            // 使用PlayerTag组件查找玩家实体
            auto playerView = _registry->view<PlayerTag, TransformComponent>();
            // EnTT 3.x使用迭代器检查是否为空
            for (auto entity : playerView) {
                return entity;  // 返回第一个玩家（O(1)！）
            }
        }
        
        // 如果需要支持更多tag类型，可以添加：
        // else if (tag == "Enemy") { ... }
        // else if (tag == "Boss") { ... }
        
        return entt::null;
    }
};

// ==================== 跳跃移动系统（EnTT版本） ====================

/**
 * @brief 跳跃移动系统 - 管理跳跃冷却和执行跳跃
 */
class JumpMovementSystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "JumpMovementSystem"; }
    int getPriority() const override { return SystemPriority::MOVEMENT - 10; }

    void update(float delta) override {
        auto view = _registry->view<JumpMovementComponent, GroundDetectorComponent,
                                     AggroComponent, SlimeSpriteComponent, TransformComponent>();
        
        view.each([delta, this](auto entity, JumpMovementComponent& jump,
                               GroundDetectorComponent& ground, AggroComponent& aggro,
                               SlimeSpriteComponent& sprite, TransformComponent& transform) {
            // 只要在地面就计时冷却
            if (ground.isOnGround) {
                jump.jumpTimer += delta;
                if (jump.jumpTimer >= jump.jumpCooldown) {
                    jump.readyToJump = true;
                }
            }

            // 执行跳跃
            if (jump.readyToJump && ground.isOnGround) {
                cocos2d::Vec2 impulse;

                // 更新追踪状态
                jump.isChasing = aggro.hasAggro;

                if (aggro.hasAggro && aggro.targetEntity != INVALID_ENTITY) {
                    // 有仇恨目标：智能追踪跳跃
                    auto targetEntity = static_cast<entt::entity>(aggro.targetEntity);
                    auto* targetTransform = _registry->try_get<TransformComponent>(targetEntity);
                    if (targetTransform) {
                        float heightDiff = targetTransform->position.y - transform.position.y;
                        impulse = jump.calculateChaseImpulse(aggro.directionToTarget.x,
                                                            heightDiff,
                                                            aggro.distanceToTarget);

                        // 更新朝向
                        sprite.setFacing(aggro.directionToTarget.x > 0);
                    }
                } else {
                    // 无仇恨目标：随机巡逻跳跃
                    impulse = jump.calculateRandomImpulse();
                    sprite.setFacing(jump.randomDirection > 0);
                }

                // 应用冲量
                sprite.applyImpulse(impulse);
                jump.lastJumpImpulse = impulse;

                // 播放跳跃动画
                sprite.playJumpAnimation();

                // 重置状态
                jump.readyToJump = false;
                jump.jumpTimer = 0.0f;
                ground.isOnGround = false;

                CCLOG("Entity %u: Jump (%.1f, %.1f) %s", entt::to_integral(entity), 
                      impulse.x, impulse.y, jump.isChasing ? "CHASE" : "PATROL");
            }
        });
    }
};

// ==================== 行走移动系统（EnTT版本） ====================

/**
 * @brief 行走移动系统 - 管理僵尸等行走类怪物的移动逻辑
 */
class WalkMovementSystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "WalkMovementSystem"; }
    int getPriority() const override { return SystemPriority::MOVEMENT; }

    void update(float delta) override {
        auto view = _registry->view<WalkMovementComponent, GroundDetectorComponent,
                                     AggroComponent, MonsterSpriteComponent, TransformComponent>();
        
        view.each([delta, this](auto entity, WalkMovementComponent& walk,
                               GroundDetectorComponent& ground, AggroComponent& aggro,
                               MonsterSpriteComponent& sprite, TransformComponent& transform) {
            
            // 检测落地
            if (ground.isOnGround && walk.isJumping) {
                walk.onLand();
            }
            
            // 更新跳跃冷却
            if (ground.isOnGround) {
                walk.updateJumpCooldown(delta);
            }
            
            // 更新障碍物跳跃冷却
            if (ground.isOnGround && walk.obstacleJumpTimer > 0) {
                walk.obstacleJumpTimer -= delta;
            }
            
            cocos2d::Vec2 currentVelocity = sprite.getVelocity();
            bool shouldJump = false;
            float targetHeightDiff = 0.0f;
            float horizontalDistToTarget = 9999.0f;
            
            // 计算与目标的水平距离
            if (aggro.hasAggro && aggro.targetEntity != INVALID_ENTITY) {
                auto targetEntity = static_cast<entt::entity>(aggro.targetEntity);
                auto* targetTransform = _registry->try_get<TransformComponent>(targetEntity);
                if (targetTransform) {
                    horizontalDistToTarget = std::abs(targetTransform->position.x - transform.position.x);
                }
            }
            
            // 更新反应跳跃计时器
            if (walk.pendingReactionJump) {
                walk.targetJumpReactionTimer += delta;
                if (walk.targetJumpReactionTimer >= walk.targetJumpReactionTime) {
                    walk.pendingReactionJump = false;
                    walk.targetJumpReactionTimer = 0.0f;
                    if (ground.isOnGround && walk.canJump() && 
                        horizontalDistToTarget < walk.jumpDetectionRange) {
                        executeJump(walk, sprite, ground);
                        CCLOG("Entity %u: Reaction jump (target jumped)", entt::to_integral(entity));
                    }
                }
            }
            
            // 确定移动方向和目标
            if (aggro.hasAggro && aggro.targetEntity != INVALID_ENTITY) {
                auto targetEntity = static_cast<entt::entity>(aggro.targetEntity);
                auto* targetTransform = _registry->try_get<TransformComponent>(targetEntity);
                if (targetTransform) {
                    
                    // 设置移动方向
                    float dirX = targetTransform->position.x - transform.position.x;
                    if (std::abs(dirX) > 5.0f) {
                        walk.currentDirection = dirX > 0 ? 1 : -1;
                    }
                    
                    // 计算高度差
                    targetHeightDiff = targetTransform->position.y - transform.position.y;
                    
                    // 检测目标是否刚跳起来
                    if (walk.targetJumpEnabled && !walk.pendingReactionJump &&
                        horizontalDistToTarget < walk.jumpDetectionRange) {
                        float targetCurrentY = targetTransform->position.y;
                        float targetYDelta = targetCurrentY - walk.targetLastY;
                        
                        if (targetYDelta > 20.0f && walk.targetLastY > 0) {
                            walk.pendingReactionJump = true;
                            walk.targetJumpReactionTimer = 0.0f;
                            CCLOG("Entity %u: Detected target jump, will react in %.1fs", 
                                  entt::to_integral(entity), walk.targetJumpReactionTime);
                        }
                    }
                    walk.targetLastY = targetTransform->position.y;
                    
                    // 目标在高处，需要跳跃追击
                    if (targetHeightDiff > walk.targetHeightThreshold && 
                        ground.isOnGround && walk.canJump() &&
                        horizontalDistToTarget < walk.jumpDetectionRange) {
                        shouldJump = true;
                    }
                }
            } else {
                // 无仇恨目标：巡逻模式
                walk.patrolTimer += delta;
                if (walk.patrolTimer >= walk.patrolDirectionChangeInterval) {
                    walk.patrolTimer = 0.0f;
                    if ((float)rand() / RAND_MAX < walk.patrolDirectionChangeChance) {
                        walk.patrolDirection *= -1;
                    }
                }
                walk.currentDirection = walk.patrolDirection;
                sprite.setFacing(walk.currentDirection > 0);
                
                walk.targetLastY = 0.0f;
                walk.pendingReactionJump = false;
            }
            
            // 检测障碍物卡住
            if (walk.shouldObstacleJump(transform.position, delta) && ground.isOnGround) {
                shouldJump = true;
                CCLOG("Entity %u: Obstacle detected (stuck), jumping", entt::to_integral(entity));
            }
            
            // 即时障碍物检测
            if (walk.useInstantObstacleDetection && ground.isOnGround && 
                walk.initialized && walk.canJump() && !shouldJump) {
                float actualVelX = std::abs(currentVelocity.x);
                float expectedVelX = walk.walkSpeed;
                
                if (walk.isWalking && expectedVelX > 10.0f && 
                    actualVelX < expectedVelX * walk.actualSpeedRatio) {
                    shouldJump = true;
                    CCLOG("Entity %u: Obstacle detected (speed diff), jumping", entt::to_integral(entity));
                }
            }
            
            // 执行跳跃
            if (shouldJump && ground.isOnGround && walk.canJump()) {
                executeJump(walk, sprite, ground);
            }
            
            // 应用水平移动和朝向
            if (ground.isOnGround) {
                walk.expectedSpeed = walk.walkSpeed;
                currentVelocity.x = walk.walkSpeed * walk.currentDirection;
                sprite.setVelocity(currentVelocity);
                walk.isWalking = true;
                if (std::abs(currentVelocity.x) > 1.0f) {
                    bool movingRight = currentVelocity.x > 0;
                    sprite.setFacing(!movingRight);
                }
            }
            
            // 更新跳跃状态
            walk.isJumping = !ground.isOnGround;
        });
    }

private:
    void executeJump(WalkMovementComponent& walk, MonsterSpriteComponent& sprite,
                    GroundDetectorComponent& ground) {
        cocos2d::Vec2 currentVel = sprite.getVelocity();
        cocos2d::Vec2 jumpVel(currentVel.x, walk.jumpForce);
        sprite.setVelocity(jumpVel);
        
        walk.onJump();
        walk.obstacleJumpTimer = walk.obstacleJumpCooldown;
        walk.stuckTime = 0.0f;
        ground.isOnGround = false;
        walk.isJumping = true;
    }
};

// ==================== 投射物系统（EnTT版本） ====================

/**
 * @brief 投射物系统 - 更新投射物位置、旋转和生命周期
 */
class ProjectileSystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "ProjectileSystem"; }
    int getPriority() const override { return SystemPriority::PHYSICS + 5; }

    void update(float delta) override {
        std::vector<entt::entity> toDestroy;
        
        auto view = _registry->view<ProjectileComponent, ProjectileSpriteComponent, TransformComponent>();
        
        view.each([delta, &toDestroy](auto entity, ProjectileComponent& proj,
                                      ProjectileSpriteComponent& sprite,
                                      TransformComponent& transform) {
            // 更新生命周期
            proj.lifetime -= delta;
            if (proj.lifetime <= 0 || proj.hasHit) {
                toDestroy.push_back(entity);
                return;
            }
            
            // 从物理体同步位置和旋转
            if (sprite.sprite && sprite.sprite->getPhysicsBody()) {
                transform.position = sprite.sprite->getPosition();
                cocos2d::Vec2 velocity = sprite.sprite->getPhysicsBody()->getVelocity();
                
                // 根据速度方向更新旋转
                if (velocity.lengthSquared() > 1.0f) {
                    float rotAngle = atan2(velocity.y, velocity.x) * 180.0f / M_PI;
                    sprite.sprite->setRotation(-rotAngle + 90.0f);
                }
            }
        });
        
        // 销毁过期的投射物
        for (auto entity : toDestroy) {
            _registry->destroy(entity);
        }
    }
};

// ==================== 投射物攻击系统（EnTT版本） ====================

/**
 * @brief 投射物攻击系统 - 处理投射物发射
 */
class ProjectileAttackSystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "ProjectileAttackSystem"; }
    int getPriority() const override { return SystemPriority::AI + 10; }

    void update(float delta) override {
        auto view = _registry->view<ProjectileAttackComponent, AggroComponent, 
                                     TransformComponent, SlimeSpriteComponent,
                                     GroundDetectorComponent, JumpMovementComponent>();
        
        view.each([delta, this](auto entity, ProjectileAttackComponent& attack,
                               AggroComponent& aggro, TransformComponent& transform,
                               SlimeSpriteComponent& sprite, GroundDetectorComponent& ground,
                               JumpMovementComponent& jump) {
            
            // 更新发射计时器
            if (!attack.canFire) {
                attack.fireTimer += delta;
                if (attack.fireTimer >= attack.fireInterval) {
                    attack.canFire = true;
                    attack.fireTimer = 0.0f;
                }
            }
            
            // 检查目标是否在发射范围内
            attack.targetInRange = aggro.hasAggro && 
                                  aggro.distanceToTarget <= attack.fireRange;
            
            // 如果目标在范围内，阻止跳跃并尝试发射
            if (attack.targetInRange) {
                // 阻止跳跃
                jump.readyToJump = false;
                jump.jumpTimer = 0.0f;
                
                // 更新朝向
                if (aggro.directionToTarget.x != 0) {
                    sprite.setFacing(aggro.directionToTarget.x > 0);
                }
                
                // 只有在地面上且可以发射时才发射
                if (attack.canFire && ground.isOnGround) {
                    fireProjectiles(entity, attack, transform, aggro, sprite);
                    attack.canFire = false;
                    attack.fireTimer = 0.0f;
                }
            }
        });
    }

private:
    void fireProjectiles(entt::entity owner, ProjectileAttackComponent& attack,
                        TransformComponent& transform, AggroComponent& aggro,
                        SlimeSpriteComponent& ownerSprite) {
        
        cocos2d::Node* parentNode = ownerSprite.sprite ? 
                                     ownerSprite.sprite->getParent() : nullptr;
        if (!parentNode) return;
        
        // 发射方向固定为正上方散开
        float baseAngle = 90.0f;
        float spreadStep = attack.projectileCount > 1 ? 
                          attack.horizontalSpread / (attack.projectileCount - 1) : 0;
        float startAngle = baseAngle - attack.horizontalSpread / 2.0f;
        
        for (int i = 0; i < attack.projectileCount; i++) {
            float angle = startAngle + spreadStep * i;
            float radians = angle * M_PI / 180.0f;
            
            // 计算初始速度
            cocos2d::Vec2 velocity;
            velocity.x = cos(radians) * attack.projectileSpeed;
            velocity.y = sin(radians) * attack.verticalImpulse;
            
            // 创建投射物实体（EnTT方式）
            auto projectile = _registry->create();
            
            // 添加变换组件
            auto& projTransform = _registry->emplace<TransformComponent>(projectile);
            projTransform.position = transform.position;
            projTransform.velocity = velocity;
            
            // 添加投射物组件
            auto& projComp = _registry->emplace<ProjectileComponent>(projectile);
            projComp.owner = entt::to_integral(owner);
            projComp.damage = attack.projectileDamage;
            projComp.lifetime = attack.projectileLifetime;
            projComp.chillChance = attack.chillChance;
            projComp.chillDuration = attack.chillDuration;
            projComp.chillSpeedReduction = attack.chillSpeedReduction;
            projComp.freezeChance = attack.freezeChance;
            projComp.freezeDuration = attack.freezeDuration;
            projComp.poisonChance1 = attack.poisonChance1;
            projComp.poisonDuration1 = attack.poisonDuration1;
            projComp.poisonDamage1 = attack.poisonDamage1;
            projComp.poisonChance2 = attack.poisonChance2;
            projComp.poisonDuration2 = attack.poisonDuration2;
            projComp.poisonDamage2 = attack.poisonDamage2;
            
            // 创建投射物精灵
            auto& projSprite = _registry->emplace<ProjectileSpriteComponent>(projectile);
            projSprite.sprite = cocos2d::Sprite::create(attack.projectileSpritePath);
            
            if (projSprite.sprite) {
                projSprite.sprite->retain();
                
                // 根据物理体尺寸计算缩放
                float scaleX = attack.projectileSpriteWidth / 
                              projSprite.sprite->getContentSize().width;
                float scaleY = attack.projectileSpriteHeight / 
                              projSprite.sprite->getContentSize().height;
                projSprite.sprite->setScale(scaleX, scaleY);
                
                projSprite.sprite->setPosition(transform.position);
                parentNode->addChild(projSprite.sprite, 2);
                
                // 设置物理体
                cocos2d::PhysicsMaterial material(0.1f, 0.0f, 0.0f);
                auto body = cocos2d::PhysicsBody::createBox(
                    cocos2d::Size(attack.projectileSpriteWidth, attack.projectileSpriteHeight),
                    material);
                body->setDynamic(true);
                body->setMass(0.1f);
                body->setGravityEnable(attack.useGravity);
                body->setRotationEnable(true);
                body->setVelocity(velocity);
                body->setContactTestBitmask(0xFFFFFFFF);
                body->setCollisionBitmask(0x0001);
                body->setCategoryBitmask(0x0004);
                body->setGroup(-2);
                projSprite.sprite->setPhysicsBody(body);
                
                // 根据速度方向设置旋转
                float rotAngle = atan2(velocity.y, velocity.x) * 180.0f / M_PI;
                projSprite.sprite->setRotation(-rotAngle + 90.0f);
                
                // 注册到NodeEntityMap（使用EntityId）
                NodeEntityMap::getInstance().registerNode(projSprite.sprite, 
                                                         entt::to_integral(projectile));
            }
            
            CCLOG("Fired Ice Spike %d at angle %.1f, velocity (%.1f, %.1f)", 
                  i, angle, velocity.x, velocity.y);
        }
        
        CCLOG("Entity %u: Fired %d ice spikes!", entt::to_integral(owner), attack.projectileCount);
    }
};

// ==================== System管理器（EnTT版本） ====================

/**
 * @brief System管理器
 * 
 * 用法：
 * SystemManagerEntt manager;
 * manager.setRegistry(&registry);
 * manager.addSystem<HealthSystemEntt>();
 * manager.update(delta);
 */
class SystemManagerEntt {
private:
    entt::registry* _registry = nullptr;
    std::vector<std::unique_ptr<ISystemEntt>> _systems;

public:
    /**
     * @brief 设置Registry
     */
    void setRegistry(entt::registry* registry) {
        _registry = registry;
        
        // 更新所有已注册System的Registry
        for (auto& system : _systems) {
            system->setRegistry(_registry);
        }
    }

    /**
     * @brief 添加System
     */
    template<typename T, typename... Args>
    T* addSystem(Args&&... args) {
        static_assert(std::is_base_of<ISystemEntt, T>::value, 
                      "T must derive from ISystemEntt");
        
        auto system = std::make_unique<T>(std::forward<Args>(args)...);
        system->setRegistry(_registry);
        
        T* ptr = system.get();
        _systems.push_back(std::move(system));
        
        // 按优先级排序
        sortSystems();
        
        return ptr;
    }

    /**
     * @brief 更新所有System
     */
    void update(float delta) {
        for (auto& system : _systems) {
            system->update(delta);
        }
    }

    /**
     * @brief 获取System数量
     */
    size_t getSystemCount() const {
        return _systems.size();
    }

    /**
     * @brief 清空所有System
     */
    void clear() {
        _systems.clear();
    }

private:
    void sortSystems() {
        std::sort(_systems.begin(), _systems.end(),
            [](const auto& a, const auto& b) {
                return a->getPriority() < b->getPriority();
            });
    }
};

} // namespace ecs
