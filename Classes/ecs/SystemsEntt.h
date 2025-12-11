#pragma once

#include <entt/entt.hpp>
#include "Components.h"
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

// ==================== System优先级（保持不变） ====================

namespace SystemPriority {
    constexpr int INPUT = -1000;   // 输入处理
    constexpr int AI = -500;       // AI决策
    constexpr int PHYSICS = 0;     // 物理模拟
    constexpr int MOVEMENT = 100;  // 移动
    constexpr int COLLISION = 200; // 碰撞处理
    constexpr int ANIMATION = 500; // 动画更新
    constexpr int RENDER = 1000;   // 渲染
    constexpr int UI = 2000;       // UI更新
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
        
        view.each([](auto entity, TransformComponent& transform, 
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
