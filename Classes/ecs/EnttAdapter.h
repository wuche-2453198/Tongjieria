#pragma once

#include <entt/entt.hpp>
#include "cocos2d.h"
#include <functional>
#include <unordered_map>

/**
 * @file EnttAdapter.h
 * @brief 自研ECS到EnTT的适配层
 * 
 * 提供向后兼容的API，使得迁移更平滑。
 * 可以逐步替换为原生EnTT API。
 */

namespace ecs {

// ==================== 类型别名 ====================

using EntityId = entt::entity;
using Registry = entt::registry;

// INVALID_ENTITY 常量
inline constexpr EntityId INVALID_ENTITY = entt::null;

// ==================== 辅助函数 ====================

/**
 * @brief 检查实体是否有效
 */
inline bool isValid(const Registry& registry, EntityId entity) {
    return registry.valid(entity);
}

/**
 * @brief 安全获取组件（返回指针）
 */
template<typename T>
inline T* tryGet(Registry& registry, EntityId entity) {
    return registry.try_get<T>(entity);
}

/**
 * @brief 安全获取组件（const版本）
 */
template<typename T>
inline const T* tryGet(const Registry& registry, EntityId entity) {
    return registry.try_get<T>(entity);
}

/**
 * @brief 检查实体是否拥有所有指定组件
 */
template<typename... Ts>
inline bool hasAll(const Registry& registry, EntityId entity) {
    return registry.all_of<Ts...>(entity);
}

/**
 * @brief 检查实体是否拥有任一指定组件
 */
template<typename... Ts>
inline bool hasAny(const Registry& registry, EntityId entity) {
    return registry.any_of<Ts...>(entity);
}

// ==================== forEach适配器 ====================

/**
 * @brief 兼容旧版forEach API的适配器
 * 
 * 用法：
 * forEach<HealthComponent, TransformComponent>(registry, 
 *     [](EntityId e, HealthComponent& h, TransformComponent& t) {
 *         // ...
 *     });
 */
template<typename... Ts, typename Func>
void forEach(Registry& registry, Func&& func) {
    auto view = registry.view<Ts...>();
    
    // EnTT的view迭代
    view.each([&](auto entity, Ts&... components) {
        func(entity, components...);
    });
}

// ==================== World适配类（可选） ====================

/**
 * @brief 兼容旧版World API的适配器类
 * 
 * 如果不想立即修改所有代码，可以用这个类包装Registry。
 * 后续可以逐步替换为直接使用Registry。
 */
class World {
private:
    Registry _registry;

public:
    World() = default;

    // 创建实体
    EntityId createEntity() {
        return _registry.create();
    }

    // 销毁实体
    void destroyEntity(EntityId entity) {
        _registry.destroy(entity);
    }

    // 添加组件（支持构造参数）
    template<typename T, typename... Args>
    T& addComponent(EntityId entity, Args&&... args) {
        return _registry.emplace<T>(entity, std::forward<Args>(args)...);
    }

    // 获取组件
    template<typename T>
    T* getComponent(EntityId entity) {
        return _registry.try_get<T>(entity);
    }

    // 获取组件（const版本）
    template<typename T>
    const T* getComponent(EntityId entity) const {
        return _registry.try_get<T>(entity);
    }

    // 移除组件
    template<typename T>
    void removeComponent(EntityId entity) {
        _registry.remove<T>(entity);
    }

    // 检查组件是否存在
    template<typename T>
    bool hasComponent(EntityId entity) const {
        return _registry.all_of<T>(entity);
    }

    // forEach迭代
    template<typename... Ts, typename Func>
    void forEach(Func&& func) {
        ecs::forEach<Ts...>(_registry, std::forward<Func>(func));
    }

    // 获取底层Registry（方便直接使用EnTT API）
    Registry& getRegistry() { return _registry; }
    const Registry& getRegistry() const { return _registry; }

    // 清空所有实体
    void clear() {
        _registry.clear();
    }

    // 获取实体数量
    size_t entityCount() const {
        return _registry.alive();
    }

    // 实体是否有效
    bool isValid(EntityId entity) const {
        return _registry.valid(entity);
    }
};

// ==================== NodeEntityMap（保持不变） ====================

/**
 * @brief Cocos2d Node到Entity的映射
 * 单例模式，O(1)查找
 */
class NodeEntityMap {
public:
    static NodeEntityMap& getInstance() {
        static NodeEntityMap instance;
        return instance;
    }

    void registerNode(cocos2d::Node* node, EntityId entity) {
        if (node) {
            _nodeToEntity[node] = entity;
        }
    }

    void unregisterNode(cocos2d::Node* node) {
        if (node) {
            _nodeToEntity.erase(node);
        }
    }

    EntityId findEntity(cocos2d::Node* node) const {
        auto it = _nodeToEntity.find(node);
        return (it != _nodeToEntity.end()) ? it->second : INVALID_ENTITY;
    }

    void clear() {
        _nodeToEntity.clear();
    }

private:
    NodeEntityMap() = default;
    std::unordered_map<cocos2d::Node*, EntityId> _nodeToEntity;

public:
    NodeEntityMap(const NodeEntityMap&) = delete;
    NodeEntityMap& operator=(const NodeEntityMap&) = delete;
};

// ==================== System基类适配 ====================

/**
 * @brief System基类（兼容旧版）
 */
class ISystem {
protected:
    Registry* _registry = nullptr;

public:
    virtual ~ISystem() = default;

    virtual const char* getName() const = 0;
    virtual int getPriority() const { return 0; }
    virtual void update(float delta) = 0;

    void setRegistry(Registry* registry) { _registry = registry; }
    Registry* getRegistry() { return _registry; }
};

// ==================== 使用示例 ====================

/*
// 示例1: 使用适配层的World类（最小修改）
ecs::World world;
auto entity = world.createEntity();
world.addComponent<HealthComponent>(entity, 100);
world.forEach<HealthComponent>([](EntityId e, HealthComponent& h) {
    h.health -= 1;
});

// 示例2: 直接使用EnTT Registry（推荐，性能更好）
ecs::Registry registry;
auto entity = registry.create();
registry.emplace<HealthComponent>(entity, 100);
auto view = registry.view<HealthComponent>();
for(auto e : view) {
    auto& health = view.get<HealthComponent>(e);
    health.health -= 1;
}

// 示例3: 使用适配器函数（中间方案）
ecs::Registry registry;
ecs::forEach<HealthComponent>(registry, [](EntityId e, HealthComponent& h) {
    h.health -= 1;
});
*/

} // namespace ecs
