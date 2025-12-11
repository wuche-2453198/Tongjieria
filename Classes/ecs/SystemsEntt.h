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
    constexpr int INPUT = 0;
    constexpr int AI = 100;
    constexpr int MOVEMENT = 200;
    constexpr int PHYSICS = 300;
    constexpr int COLLISION = 400;
    constexpr int RENDERING = 500;
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
