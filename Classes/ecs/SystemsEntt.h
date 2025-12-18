#pragma once

#include <entt/entt.hpp>
#include "Components.h"
#include "SpriteComponent.h"
#include <vector>
#include <memory>
#include <algorithm>

/**
 * @file SystemsEntt.h
 * @brief EnTT版本的System实现
 */

namespace ecs {

// ==================== 系统优先级定义 ====================

/**
 * @brief 系统执行优先级（数值越小越先执行）
 */
namespace SystemPriority {
constexpr int INPUT = 0;
constexpr int PHYSICS = 100;
constexpr int COLLISION = 200;
constexpr int AI = 300;
constexpr int MOVEMENT = 400;
constexpr int ANIMATION = 500;
constexpr int RENDER = 600;
}

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

// ==================== 生命值系统 ====================

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
                if (health.invincibleTimer < 0.0f) {
                    health.invincibleTimer = 0.0f;
                }
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
                if (combat.attackTimer < 0.0f) {
                    combat.attackTimer = 0.0f;
                }
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

// ==================== 减益系统 ====================

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

// ==================== 史莱姆同步系统 ====================

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

            // 物理引擎自动更新精灵位置
            transform.position = sprite.sprite->getPosition();
        });
    }
};

// ==================== 怪物同步系统 ====================

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

// ==================== 地面检测系统 ====================

/**
 * @brief 地面检测系统 - 根据物理体速度判断地面和静止状态
 * 
 * 动态阻尼：地面时高阻尼快速停止，空中低阻尼保持灵活
 */
class GroundDetectorSystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "GroundDetectorSystem"; }
    int getPriority() const override { return SystemPriority::PHYSICS + 10; }

    // 阻尼配置
    static constexpr float GROUND_DAMPING = 8.0f;  // 地面高阻尼，快速停止
    static constexpr float AIR_DAMPING = 0.3f;     // 空中低阻尼，保持灵活

    void update(float delta) override {
        auto view = _registry->view<GroundDetectorComponent, SlimeSpriteComponent>();
        
        view.each([](auto entity, GroundDetectorComponent& ground,
                    SlimeSpriteComponent& sprite) {
            cocos2d::Vec2 velocity = sprite.getVelocity();

            // 判断是否静止
            ground.isStill = std::abs(velocity.x) < ground.stillThreshold &&
                           std::abs(velocity.y) < ground.stillThreshold;
            
            // 动态切换阻尼：地面高阻尼防止滑行，空中低阻尼保持跳跃灵活性
            auto* body = sprite.getPhysicsBody();
            if (body) {
                if (ground.isOnGround) {
                    body->setLinearDamping(GROUND_DAMPING);
                } else {
                    body->setLinearDamping(AIR_DAMPING);
                }
            }
        });
    }
};

// ==================== 怪物地面检测系统 ====================

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

// ==================== 缓降系统 ====================

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

// ==================== 仇恨检测系统 ====================

/**
 * @brief 仇恨检测系统 - 检测目标并更新仇恨状态
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
     * @brief 根据tag查找目标实体
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

// ==================== 跳跃移动系统 ====================

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
 * @brief 战士AI系统 - 管理僵尸等行走类怪物的移动和AI逻辑
 * 
 * 行为特点：
 * - 行走追踪玩家
 * - 跳过洞和障碍物
 * - 尝试垂直对齐目标高度
 * - 追击失败时后退重试
 */
class WarriorAISystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "WarriorAISystem"; }
    int getPriority() const override { return SystemPriority::MOVEMENT; }

    void update(float delta) override {
        auto view = _registry->view<WarriorMovementComponent, GroundDetectorComponent,
                                     AggroComponent, MonsterSpriteComponent, TransformComponent>();
        
        view.each([delta, this](auto entity, WarriorMovementComponent& warrior,
                               GroundDetectorComponent& ground, AggroComponent& aggro,
                               MonsterSpriteComponent& sprite, TransformComponent& transform) {
            
            // 检测落地并判断跳跃是否成功
            if (ground.isOnGround && warrior.isJumping) {
                warrior.onLand();
                
                // 检测跳跃是否失败（水平位移很小）
                if (warrior.wasJumping) {
                    if (warrior.checkJumpFailed(transform.position)) {
                        warrior.onJumpFailed();
                        CCLOG("Entity %u: Jump failed (fail count: %d)", 
                              entt::to_integral(entity), warrior.jumpFailCount);
                    } else {
                        warrior.onJumpSuccess();
                    }
                }
            }
            warrior.wasJumping = warrior.isJumping;
            
            // 更新跳跃冷却
            if (ground.isOnGround) {
                warrior.updateJumpCooldown(delta);
            }
            
            // 更新障碍物跳跃冷却
            if (ground.isOnGround && warrior.obstacleJumpTimer > 0) {
                warrior.obstacleJumpTimer -= delta;
            }
            
            // 更新后退状态
            warrior.updateRetreat(delta);
            
            cocos2d::Vec2 currentVelocity = sprite.getVelocity();
            bool shouldJump = false;
            float targetHeightDiff = 0.0f;
            float horizontalDistToTarget = 9999.0f;
            float totalDistToTarget = 9999.0f;
            
            // 计算与目标的距离
            if (aggro.hasAggro && aggro.targetEntity != INVALID_ENTITY) {
                auto targetEntity = static_cast<entt::entity>(aggro.targetEntity);
                auto* targetTransform = _registry->try_get<TransformComponent>(targetEntity);
                if (targetTransform) {
                    horizontalDistToTarget = std::abs(targetTransform->position.x - transform.position.x);
                    float verticalDist = std::abs(targetTransform->position.y - transform.position.y);
                    totalDistToTarget = std::sqrt(horizontalDistToTarget * horizontalDistToTarget + 
                                                  verticalDist * verticalDist);
                }
            }
            
            // 更新反应跳跃计时器
            if (warrior.pendingReactionJump) {
                warrior.targetJumpReactionTimer += delta;
                if (warrior.targetJumpReactionTimer >= warrior.targetJumpReactionTime) {
                    warrior.pendingReactionJump = false;
                    warrior.targetJumpReactionTimer = 0.0f;
                    if (ground.isOnGround && warrior.canJump() && 
                        horizontalDistToTarget < warrior.jumpDetectionRange) {
                        executeJump(warrior, sprite, ground, transform.position);
                    }
                }
            }
            
            // 确定移动方向和目标
            if (aggro.hasAggro && aggro.targetEntity != INVALID_ENTITY) {
                auto targetEntity = static_cast<entt::entity>(aggro.targetEntity);
                auto* targetTransform = _registry->try_get<TransformComponent>(targetEntity);
                if (targetTransform) {
                    
                    // 计算高度差
                    targetHeightDiff = targetTransform->position.y - transform.position.y;
                    
                    // ===== 战士AI：后退重试机制（只在连续跳跃失败时触发） =====
                    if (warrior.isRetreating) {
                        // 后退时反向移动
                        float dirX = targetTransform->position.x - transform.position.x;
                        warrior.currentDirection = dirX > 0 ? -1 : 1;  // 反向
                    } else {
                        // 正常追击
                        float dirX = targetTransform->position.x - transform.position.x;
                        if (std::abs(dirX) > 5.0f) {
                            warrior.currentDirection = dirX > 0 ? 1 : -1;
                        }
                        
                        // 检测是否需要后退（连续多次跳跃失败）
                        if (warrior.shouldRetreat() && ground.isOnGround) {
                            warrior.startRetreat();
                            CCLOG("Entity %u: Multiple jump failures, retreating to retry", entt::to_integral(entity));
                        }
                    }
                    
                    // 检测目标是否刚跳起来（跟随跳跃）
                    if (warrior.targetJumpEnabled && !warrior.pendingReactionJump &&
                        horizontalDistToTarget < warrior.jumpDetectionRange && !warrior.isRetreating) {
                        float targetCurrentY = targetTransform->position.y;
                        float targetYDelta = targetCurrentY - warrior.targetLastY;
                        
                        if (targetYDelta > 20.0f && warrior.targetLastY > 0) {
                            warrior.pendingReactionJump = true;
                            warrior.targetJumpReactionTimer = 0.0f;
                        }
                    }
                    warrior.targetLastY = targetTransform->position.y;
                    
                    // ===== 战士AI：垂直对齐 - 目标在高处时跳跃 =====
                    if (warrior.verticalAlignEnabled && !warrior.isRetreating &&
                        targetHeightDiff > warrior.verticalAlignThreshold && 
                        ground.isOnGround && warrior.canJump() &&
                        horizontalDistToTarget < warrior.jumpDetectionRange) {
                        shouldJump = true;
                    }
                }
            } else {
                // 无仇恨目标：巡逻模式（只在遇到障碍物跳跃失败时换向）
                warrior.currentDirection = warrior.patrolDirection;
                sprite.setFacing(warrior.currentDirection > 0);
                
                // 巡逻时如果连续跳跃失败，换方向
                if (warrior.shouldRetreat() && ground.isOnGround) {
                    warrior.patrolDirection *= -1;
                    warrior.currentDirection = warrior.patrolDirection;
                    warrior.jumpFailCount = 0;
                    warrior.retreatCooldownTimer = warrior.retreatCooldown;
                    CCLOG("Entity %u: Patrol direction changed due to obstacle", entt::to_integral(entity));
                }
                
                warrior.targetLastY = 0.0f;
                warrior.pendingReactionJump = false;
            }
            
            // 检测障碍物卡住（也用于跳过洞）
            if (warrior.shouldObstacleJump(transform.position, delta) && ground.isOnGround) {
                shouldJump = true;
            }
            
            // ===== 战士AI：即时障碍物/洞检测 =====
            if (warrior.gapJumpEnabled && ground.isOnGround && 
                warrior.initialized && warrior.canJump() && !shouldJump && !warrior.isRetreating) {
                float actualVelX = std::abs(currentVelocity.x);
                float expectedVelX = warrior.walkSpeed;
                
                // 速度差检测（卡住或前方有洞）
                if (warrior.isWalking && expectedVelX > 10.0f && 
                    actualVelX < expectedVelX * warrior.actualSpeedRatio) {
                    shouldJump = true;
                }
            }
            
            // 执行跳跃
            if (shouldJump && ground.isOnGround && warrior.canJump()) {
                executeJump(warrior, sprite, ground, transform.position);
            }
            
            // 计算目标水平速度
            float moveSpeed = warrior.isRetreating ? warrior.walkSpeed * 0.6f : warrior.walkSpeed;
            float targetVelX = moveSpeed * warrior.currentDirection;
            
            // 应用水平移动和朝向
            if (ground.isOnGround) {
                // 地面：直接设置速度
                warrior.expectedSpeed = warrior.walkSpeed;
                currentVelocity.x = targetVelX;
                sprite.setVelocity(currentVelocity);
                warrior.isWalking = true;
            } else {
                // 空中：持续控制水平速度，确保能越过障碍
                currentVelocity = sprite.getVelocity();
                // 如果空中水平速度不足，强制设置为目标速度
                if (std::abs(currentVelocity.x) < std::abs(targetVelX) * 0.5f) {
                    currentVelocity.x = targetVelX;
                    sprite.setVelocity(currentVelocity);
                }
            }
            
            // 更新朝向
            if (std::abs(targetVelX) > 1.0f) {
                bool movingRight = targetVelX > 0;
                sprite.setFacing(!movingRight);
            }
            
            // 更新跳跃状态
            warrior.isJumping = !ground.isOnGround;
        });
    }

private:
    void executeJump(WarriorMovementComponent& warrior, MonsterSpriteComponent& sprite,
                    GroundDetectorComponent& ground, const cocos2d::Vec2& currentPos) {
        cocos2d::Vec2 currentVel = sprite.getVelocity();
        
        // 记录跳跃前位置，用于后续判断跳跃是否成功
        warrior.onJumpStart(currentPos);
        
        // 如果当前水平速度很小（被卡住），使用AI期望的方向速度
        float jumpHorizontalVel = currentVel.x;
        if (std::abs(jumpHorizontalVel) < warrior.walkSpeed * 0.3f) {
            // 被卡住时，给予AI期望方向的水平速度
            jumpHorizontalVel = warrior.walkSpeed * warrior.currentDirection;
        }
        
        cocos2d::Vec2 jumpVel(jumpHorizontalVel, warrior.jumpForce);
        sprite.setVelocity(jumpVel);
        
        warrior.onJump();
        warrior.obstacleJumpTimer = warrior.obstacleJumpCooldown;
        warrior.stuckTime = 0.0f;
        ground.isOnGround = false;
        warrior.isJumping = true;
    }
};

// 为了向后兼容，保留别名
using WalkMovementSystemEntt = WarriorAISystemEntt;

// ==================== 投射物系统 ====================

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
        
        for (auto entity : view) {
            auto& proj = view.get<ProjectileComponent>(entity);
            auto& sprite = view.get<ProjectileSpriteComponent>(entity);
            auto& transform = view.get<TransformComponent>(entity);
            // 更新生命周期
            proj.lifetime -= delta;
            if (proj.lifetime <= 0 || proj.hasHit) {
                toDestroy.push_back(entity);
                continue;
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
        }
        
        // 销毁过期的投射物
        for (auto entity : toDestroy) {
            _registry->destroy(entity);
        }
    }
};

// ==================== 投射物攻击系统 ====================

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
            
            // 创建投射物实体
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
            
            // 如果贴图加载失败，使用纯色方块
            if (!projSprite.sprite || !projSprite.sprite->getTexture()) {
                CCLOG("Failed to load projectile sprite '%s', using fallback", 
                      attack.projectileSpritePath.c_str());
                projSprite.sprite = cocos2d::Sprite::create();
                projSprite.sprite->setTextureRect(
                    cocos2d::Rect(0, 0, attack.projectileSpriteWidth, attack.projectileSpriteHeight));
                projSprite.sprite->setColor(cocos2d::Color3B(100, 200, 255)); // 蓝色冰刺
            }
            
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
                
                // 注册到NodeEntityMap
                NodeEntityMap::getInstance().registerNode(projSprite.sprite, 
                                                         entt::to_integral(projectile));
            }
            
            CCLOG("Fired Ice Spike %d at angle %.1f, velocity (%.1f, %.1f)", 
                  i, angle, velocity.x, velocity.y);
        }
        
        CCLOG("Entity %u: Fired %d ice spikes!", entt::to_integral(owner), attack.projectileCount);
    }
};

// ==================== 恶魔眼AI系统 ====================

/**
 * @brief 恶魔眼AI系统 - 飞行追踪类怪物AI
 * 
 * 行为特点：
 * - 飞行追踪玩家（无重力）
 * - 缓慢转向，转弯速率较慢
 * - 撞墙/物块时弧形回弹
 * - 被击退时弧形轨迹回弹
 */
class DemonEyeAISystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "DemonEyeAISystem"; }
    int getPriority() const override { return SystemPriority::MOVEMENT; }

    void update(float delta) override {
        auto view = _registry->view<DemonEyeMovementComponent, AggroComponent,
                                     MonsterSpriteComponent, TransformComponent>();
        
        view.each([delta, this](auto entity, DemonEyeMovementComponent& demon,
                               AggroComponent& aggro, MonsterSpriteComponent& sprite,
                               TransformComponent& transform) {
            
            if (!sprite.sprite) return;

            // 获取目标位置（如果有仇恨）
            cocos2d::Vec2 targetPos = cocos2d::Vec2::ZERO;
            bool hasTarget = false;
            
            // 验证aggro组件状态
            if (aggro.hasAggro && aggro.targetEntity != INVALID_ENTITY) {
                targetPos = transform.position + aggro.directionToTarget * aggro.distanceToTarget;
                hasTarget = true;
            }

            // 更新冲刺冷却
            if (demon.dashCooldownTimer > 0.0f) {
                demon.dashCooldownTimer -= delta;
            }
            
            if (!hasTarget) {
                // 无目标：巡逻
                if (demon.aiState != DemonEyeMovementComponent::HOVERING) {
                    demon.aiState = DemonEyeMovementComponent::HOVERING;
                }
                updatePatrol(demon, delta);
                demon.smoothTurn(delta);
            } else {
                // 有目标：根据状态执行AI
                cocos2d::Vec2 toTarget = targetPos - transform.position;
                float dist = toTarget.length();
                
                switch (demon.aiState) {
                    case DemonEyeMovementComponent::HOVERING: {
                        // 盘旋状态：更激进地追踪玩家
                        // 惯性期检查：只有当aiTimer >= 0时才重新计算目标角度
                        if (demon.aiTimer >= 0.0f) {
                            // 直接朝向目标移动，更具攻击性
                            cocos2d::Vec2 desiredDir = toTarget;
                            if (!desiredDir.isZero()) desiredDir.normalize();
                            demon.targetAngle = atan2(desiredDir.y, desiredDir.x);
                            
                            // 检查是否满足冲刺条件（优化触发范围和预判）
                            bool isAbove = transform.position.y > targetPos.y + 30.0f;
                            bool horizontalInRange = abs(toTarget.x) < 250.0f; // 增加水平触发范围
                            bool verticalInRange = toTarget.y < -30.0f && toTarget.y > -300.0f; // 优化垂直范围
                            
                            if (isAbove && horizontalInRange && verticalInRange && demon.dashCooldownTimer <= 0.0f) {
                                demon.aiTimer += delta;
                                if (demon.aiTimer > 0.6f) { // 减少蓄力时间，提高反应速度
                                    demon.aiState = DemonEyeMovementComponent::DASHING;
                                    demon.aiTimer = 0.0f;
                                    // 预判玩家位置：考虑玩家当前移动趋势
                                    auto targetEntity = static_cast<entt::entity>(aggro.targetEntity);
                                    auto* targetTransform = _registry->try_get<TransformComponent>(targetEntity);
                                    if (targetTransform) {
                                        // 简单预判：假设玩家保持当前速度0.5秒
                                        cocos2d::Vec2 predictedPos = targetPos + aggro.directionToTarget * 100.0f;
                                        cocos2d::Vec2 dashDirection = (predictedPos - transform.position).getNormalized();
                                        demon.targetAngle = atan2(dashDirection.y, dashDirection.x);
                                        demon.currentAngle = demon.targetAngle; // 立即对准目标
                                    }
                                }
                            } else {
                                demon.aiTimer = 0.0f;
                            }
                        } else {
                            // 惯性期内：保持当前方向
                            demon.aiTimer += delta;
                        }
                        
                        // 动态调整转弯速率：准备冲刺时增加角速度
                        float originalTurnRate = demon.turnRate;
                        bool isPreparingDash = (demon.aiTimer > 0.0f && demon.aiTimer < 0.6f);
                        bool closeToTarget = hasTarget && aggro.distanceToTarget < 400.0f;
                        
                        if (isPreparingDash && closeToTarget) {
                            // 冲刺准备期：大幅提高转弯速率，快速调整到最佳攻击角度
                            demon.turnRate *= 3.0f; 
                        } else if (closeToTarget) {
                            // 接近目标时：适度提高转弯速率，增强追踪能力
                            demon.turnRate *= 2.0f;
                        }
                        
                        demon.smoothTurn(delta);
                        
                        // 恢复原始转弯速率
                        demon.turnRate = originalTurnRate;
                        break;
                    }
                    
                    case DemonEyeMovementComponent::DASHING: {
                        // 俯冲攻击 - 改进的追踪逻辑
                        demon.aiTimer += delta;
                        
                        // 冲刺分为三个阶段：加速阶段、追踪阶段、收尾阶段
                        if (demon.aiTimer < 0.2f) {
                            // 阶段1（0-0.2s）：加速阶段，保持初始方向
                            // 不转向，专注加速
                        } else if (demon.aiTimer < 1.2f) {
                            // 阶段2（0.2-1.2s）：主要追踪阶段，积极修正方向
                            float angleToPlayer = atan2(toTarget.y, toTarget.x);
                            float currentDiff = DemonEyeMovementComponent::angleDifference(angleToPlayer, demon.currentAngle);
                            float dashTurnRate = demon.turnRate * 1.0f; // 提高追踪灵敏度
                            
                            // 距离越近，转向越快（提高近距离命中率）
                            if (dist < 150.0f) {
                                dashTurnRate *= 2.0f;
                            }
                            
                            if (abs(currentDiff) < dashTurnRate * delta) {
                                demon.currentAngle = angleToPlayer;
                            } else {
                                demon.currentAngle += (currentDiff > 0 ? 1 : -1) * dashTurnRate * delta;
                            }
                        } else {
                            // 阶段3（1.2s+）：收尾阶段，轻微追踪
                            float angleToPlayer = atan2(toTarget.y, toTarget.x);
                            float currentDiff = DemonEyeMovementComponent::angleDifference(angleToPlayer, demon.currentAngle);
                            float dashTurnRate = demon.turnRate * 0.3f;
                            if (abs(currentDiff) < dashTurnRate * delta) demon.currentAngle = angleToPlayer;
                            else demon.currentAngle += (currentDiff > 0 ? 1 : -1) * dashTurnRate * delta;
                        }
                        
                        // 优化冲刺结束条件
                        bool passedPlayer = (transform.position.y < targetPos.y - 80.0f); // 减少穿透距离
                        bool tooFar = dist > 500.0f; // 增加追击距离
                        bool tooClose = dist < 30.0f && demon.aiTimer > 0.3f; // 添加近距离命中检测
                        
                        if (passedPlayer || demon.aiTimer > 2.5f || tooFar || tooClose) {
                            demon.aiState = DemonEyeMovementComponent::HOVERING;
                            demon.aiTimer = 0.0f;
                            demon.dashCooldownTimer = demon.dashCooldown; // 设置冷却
                        }
                        break;
                    }
                }
            }
            
            // 更新并添加摆动效果使飞行更自然
            demon.wobblePhase += demon.wobbleFrequency * delta;
            float wobbleOffset = sin(demon.wobblePhase) * demon.wobbleAmplitude;
            float effectiveAngle = demon.currentAngle + wobbleOffset;
            
            // ===== 关键修复：先读取物理引擎速度，再施加控制力 =====
            auto* body = sprite.getPhysicsBody();
            if (!body) return;
            
            // 从物理体读取当前速度（包含碰撞反弹结果）
            cocos2d::Vec2 physicsVelocity = body->getVelocity();
            demon.currentVelocity = physicsVelocity;
            float currentSpeedSq = demon.currentVelocity.lengthSquared();
            
            // 如果速度过低（初始化或卡住），强制给予初始速度
            if (currentSpeedSq < 100.0f) { // 10^2 = 100
                demon.currentVelocity.x = cos(demon.currentAngle) * demon.flySpeed;
                demon.currentVelocity.y = sin(demon.currentAngle) * demon.flySpeed;
                body->setVelocity(demon.currentVelocity);
                return;
            }
            
            // 计算期望速度方向和大小
            // 盘旋和冲刺使用相同的最大速度，保持一致的威胁感
            float targetSpeed = demon.maxSpeed;
            
            // 根据角速度降低最大速度：角速度越高，最大速度越低
            float angularVelocity = std::abs(body->getAngularVelocity());
            if (angularVelocity > 0.1f) {
                // 使用二次函数降低速度：速度 = maxSpeed * (1 - angularVelocity^2 / factor)
                // 当角速度为0.5时，速度降低到50%；当角速度为1.0时，速度降低到0%
                float speedReduction = std::min(1.0f, (angularVelocity * angularVelocity) / 0.25f);
                targetSpeed *= (1.0f - speedReduction);
            }
            
            // 转弯时略微减速以保持控制
            float angleDiff = std::abs(DemonEyeMovementComponent::angleDifference(
                demon.targetAngle, demon.currentAngle));
            if (angleDiff > 1.0f) targetSpeed *= 0.85f;
            
            // 计算期望速度向量
            cocos2d::Vec2 desiredVelocity;
            desiredVelocity.x = cos(effectiveAngle) * targetSpeed;
            desiredVelocity.y = sin(effectiveAngle) * targetSpeed;
            
            // 关键修复：使用速度插值而非applyForce，防止震荡
            // 计算插值系数：根据加速度和delta时间
            float lerpFactor = std::min(1.0f, demon.acceleration * delta / demon.flySpeed);
            
            // 对当前速度和期望速度进行插值
            cocos2d::Vec2 newVelocity;
            newVelocity.x = demon.currentVelocity.x + (desiredVelocity.x - demon.currentVelocity.x) * lerpFactor;
            newVelocity.y = demon.currentVelocity.y + (desiredVelocity.y - demon.currentVelocity.y) * lerpFactor;
            
            // 限制最大速度
            float newSpeed = newVelocity.length();
            if (newSpeed > demon.maxSpeed * 1.5f) {
                newVelocity.normalize();
                newVelocity *= demon.maxSpeed * 1.5f;
            }
            
            // 保证最小速度，防止卡住
            if (newSpeed < 50.0f && newSpeed > 0.1f) {
                newVelocity.normalize();
                newVelocity *= 50.0f;
            }
            
            // 直接设置速度（AI控制，但保留了物理碰撞的影响）
            body->setVelocity(newVelocity);
            
            // 同步位置
            transform.position = sprite.sprite->getPosition();
            demon.currentVelocity = body->getVelocity();
            
            // ===== 更新精灵旋转（贴图朝向移动方向） =====
            // 只有速度足够大时才更新朝向，防止微小震荡
            if (demon.currentVelocity.lengthSquared() > 100.0f) {
                // 使用实际速度方向，使碰撞回弹时朝向正确
                float velocityAngle = atan2(demon.currentVelocity.y, demon.currentVelocity.x);
                
                // 碰撞后同步更新AI角度，使后续追踪从新方向开始
                // 检测速度方向与AI角度的差异，如果较大则可能是碰撞
                float angleDiff = std::abs(DemonEyeMovementComponent::angleDifference(velocityAngle, demon.currentAngle));
                if (angleDiff > 1.57f) { // 90度 = 1.57弧度，说明发生了大角度偏转（碰撞）
                    demon.currentAngle = velocityAngle;
                    // 在碰撞瞬间，重置targetAngle为当前反弹方向，避免AI立即强行扭回原方向
                    demon.targetAngle = velocityAngle;
                    
                    // 碰撞反应：
                    // 1. 如果正在冲刺，立即中断并回到盘旋状态
                    if (demon.aiState == DemonEyeMovementComponent::DASHING) {
                        demon.aiState = DemonEyeMovementComponent::HOVERING;
                        demon.dashCooldownTimer = demon.dashCooldown; // 触发冷却
                    }
                    
                    // 2. 给予短暂的惯性期（例如0.4秒），期间不重新索敌，让物理反弹自然发生
                    demon.aiTimer = -0.4f;
                }
                
                float displayAngle = velocityAngle * 180.0f / M_PI;
                // 恶魔眼贴图默认朝左，需要加180度让它朝向移动方向
                sprite.sprite->setRotation(-displayAngle + 180.0f);
                
                // 根据移动方向翻转精灵（当向左飞行时翻转，避免上下颠倒）
                bool movingLeft = demon.currentVelocity.x < 0;
                sprite.sprite->setFlippedY(movingLeft);
            }
        });
    }

private:

    /**
     * @brief 更新巡逻行为
     */
    void updatePatrol(DemonEyeMovementComponent& demon, float delta) {
        demon.patrolTimer += delta;
        
        if (demon.patrolTimer >= demon.patrolChangeInterval) {
            demon.patrolTimer = 0.0f;
            // 随机选择新的巡逻方向
            demon.patrolAngle = ((float)rand() / RAND_MAX) * 2 * M_PI;
            CCLOG("DemonEye: New patrol angle: %.2f", demon.patrolAngle);
        }
        
        demon.targetAngle = demon.patrolAngle;
    }
};

// ==================== System管理器 ====================

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
