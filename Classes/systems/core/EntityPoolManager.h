#ifndef __ECS_SYSTEM_ENTITYPOOLMANAGER_H__
#define __ECS_SYSTEM_ENTITYPOOLMANAGER_H__

#include <entt/entt.hpp>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include "cocos2d.h"
#include "components/core/PooledEntity.h"
#include "components/core/TransformComponent.h"
#include "components/render/RenderComponent.h"
#include "components/render/SpriteStateComponent.h"
#include "components/item/ProjectileComponent.h"

namespace ecs {

/**
 * @brief 实体池管理器 - 管理各类实体的对象池
 * 
 * 单例模式，负责预分配和复用实体，避免运行时频繁创建/销毁实体。
 * 
 * Requirements: 4.1, 4.2
 */
class EntityPoolManager {
public:
    /**
     * @brief 获取单例实例
     */
    static EntityPoolManager& getInstance() {
        static EntityPoolManager instance;
        return instance;
    }

    // 禁止拷贝和移动
    EntityPoolManager(const EntityPoolManager&) = delete;
    EntityPoolManager& operator=(const EntityPoolManager&) = delete;
    EntityPoolManager(EntityPoolManager&&) = delete;
    EntityPoolManager& operator=(EntityPoolManager&&) = delete;

    /**
     * @brief 预分配投射物实体
     * @param registry EnTT 注册表
     * @param count 预分配数量
     */
    void preallocateProjectiles(entt::registry& registry, size_t count);

    /**
     * @brief 预分配粒子实体
     * @param registry EnTT 注册表
     * @param count 预分配数量
     */
    void preallocateParticles(entt::registry& registry, size_t count);

    /**
     * @brief 从池中获取投射物实体
     * @param registry EnTT 注册表
     * @return 可用的实体，如果池为空则创建新实体
     */
    entt::entity acquireProjectile(entt::registry& registry);

    /**
     * @brief 归还投射物实体到池中
     * @param registry EnTT 注册表
     * @param entity 要归还的实体
     */
    void releaseProjectile(entt::registry& registry, entt::entity entity);

    /**
     * @brief 从池中获取粒子实体
     * @param registry EnTT 注册表
     * @return 可用的实体
     */
    entt::entity acquireParticle(entt::registry& registry);

    /**
     * @brief 归还粒子实体到池中
     * @param registry EnTT 注册表
     * @param entity 要归还的实体
     */
    void releaseParticle(entt::registry& registry, entt::entity entity);

    /**
     * @brief 重置实体组件到默认状态
     * @param registry EnTT 注册表
     * @param entity 要重置的实体
     * @param poolType 池类型
     */
    void resetEntity(entt::registry& registry, entt::entity entity, const std::string& poolType);

    /**
     * @brief 获取投射物池统计信息
     */
    size_t getProjectilePoolSize() const { return _projectilePool.size(); }
    size_t getProjectileActiveCount() const { return _projectileActiveCount; }
    size_t getProjectileAvailableCount() const { return _projectilePool.size() - _projectileActiveCount; }

    /**
     * @brief 获取粒子池统计信息
     */
    size_t getParticlePoolSize() const { return _particlePool.size(); }
    size_t getParticleActiveCount() const { return _particleActiveCount; }
    size_t getParticleAvailableCount() const { return _particlePool.size() - _particleActiveCount; }

    /**
     * @brief 清理所有池
     * @param registry EnTT 注册表
     */
    void clearAll(entt::registry& registry);

    /**
     * @brief 检查实体是否属于池
     * @param registry EnTT 注册表
     * @param entity 要检查的实体
     * @return 是否是池化实体
     */
    bool isPooledEntity(entt::registry& registry, entt::entity entity) const;

private:
    EntityPoolManager() = default;
    ~EntityPoolManager() = default;

    /**
     * @brief 创建池化投射物实体（内部使用）
     */
    entt::entity createPooledProjectile(entt::registry& registry);

    /**
     * @brief 创建池化粒子实体（内部使用）
     */
    entt::entity createPooledParticle(entt::registry& registry);

    /**
     * @brief 隐藏实体的精灵（归还时调用）
     */
    void hideEntitySprite(entt::registry& registry, entt::entity entity);

    // 投射物池
    std::vector<entt::entity> _projectilePool;
    std::vector<entt::entity> _availableProjectiles;
    size_t _projectileActiveCount = 0;

    // 粒子池
    std::vector<entt::entity> _particlePool;
    std::vector<entt::entity> _availableParticles;
    size_t _particleActiveCount = 0;

    // 池类型常量
    static constexpr const char* POOL_TYPE_PROJECTILE = "projectile";
    static constexpr const char* POOL_TYPE_PARTICLE = "particle";
};

} // namespace ecs

#endif // __ECS_SYSTEM_ENTITYPOOLMANAGER_H__
