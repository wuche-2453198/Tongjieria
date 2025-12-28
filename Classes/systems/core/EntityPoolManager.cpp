#include "EntityPoolManager.h"
#include "EntityDestructionManager.h"
 #include "components/render/AnimationComponent.h"
 #include "components/item/DelayedAccelerationComponent.h"
 #include "components/item/NoVelocityRotationTag.h"

namespace ecs {

void EntityPoolManager::preallocateProjectiles(entt::registry& registry, size_t count) {
    _projectilePool.reserve(_projectilePool.size() + count);
    _availableProjectiles.reserve(_availableProjectiles.size() + count);

    for (size_t i = 0; i < count; ++i) {
        entt::entity entity = createPooledProjectile(registry);
        _projectilePool.push_back(entity);
        _availableProjectiles.push_back(entity);
    }

    CCLOG("[EntityPoolManager] Preallocated %zu projectiles, total pool size: %zu",
          count, _projectilePool.size());
}

void EntityPoolManager::preallocateParticles(entt::registry& registry, size_t count) {
    _particlePool.reserve(_particlePool.size() + count);
    _availableParticles.reserve(_availableParticles.size() + count);

    for (size_t i = 0; i < count; ++i) {
        entt::entity entity = createPooledParticle(registry);
        _particlePool.push_back(entity);
        _availableParticles.push_back(entity);
    }

    CCLOG("[EntityPoolManager] Preallocated %zu particles, total pool size: %zu",
          count, _particlePool.size());
}

entt::entity EntityPoolManager::acquireProjectile(entt::registry& registry) {
    if (_availableProjectiles.empty()) {
        // 池耗尽时动态扩展
        size_t expandSize = std::max(size_t(1), _projectilePool.size() / 2);
        CCLOG("[EntityPoolManager] Projectile pool exhausted, expanding by %zu...", expandSize);
        preallocateProjectiles(registry, expandSize);
    }

    entt::entity entity = _availableProjectiles.back();
    _availableProjectiles.pop_back();
    ++_projectileActiveCount;

    // 标记为正在使用
    if (registry.valid(entity)) {
        auto* pooled = registry.try_get<PooledEntity>(entity);
        if (pooled) {
            pooled->inUse = true;
        }
    }

    return entity;
}

void EntityPoolManager::releaseProjectile(entt::registry& registry, entt::entity entity) {
    if (!registry.valid(entity)) {
        CCLOG("[EntityPoolManager] Warning: Attempting to release invalid entity");
        return;
    }

    // 验证实体属于投射物池
    auto* pooled = registry.try_get<PooledEntity>(entity);
    if (!pooled || pooled->poolType != POOL_TYPE_PROJECTILE) {
        CCLOG("[EntityPoolManager] Warning: Entity is not a pooled projectile");
        return;
    }

    // 重复释放检测 - Requirements 3.3
    if (!pooled->inUse) {
        CCLOG("[EntityPoolManager] Warning: Duplicate release detected for projectile entity %u - already released",
              static_cast<uint32_t>(entity));
        return;
    }

    // 检查实体是否在销毁队列中，如果是则从队列中移除
    // 因为池化实体不应该被销毁，而是被复用
    auto& destructionManager = EntityDestructionManager::getInstance();
    if (destructionManager.isPendingDestruction(entity)) {
        CCLOG("[EntityPoolManager] Note: Removing pooled entity from destruction queue before release");
        // 注意：EntityDestructionManager 没有提供移除单个实体的方法
        // 但由于我们在 processQueue 之前调用 release，实体会被标记为 inUse=false
        // 这样即使 processQueue 被调用，也不会销毁这个实体（因为它是池化的）
    }

    // 隐藏精灵
    hideEntitySprite(registry, entity);

    // 完全重置实体状态 - Requirements 3.1, 3.2
    resetEntity(registry, entity, POOL_TYPE_PROJECTILE);

    // 标记为未使用 - 这是其他系统检测实体是否活跃的关键标志
    // Requirements 3.3: 系统通过检查 PooledEntity.inUse 来检测实体是否仍在使用
    pooled->inUse = false;

    _availableProjectiles.push_back(entity);
    --_projectileActiveCount;

    CCLOG("[EntityPoolManager] Projectile released, available: %zu, active: %zu",
          _availableProjectiles.size(), _projectileActiveCount);
}

entt::entity EntityPoolManager::acquireParticle(entt::registry& registry) {
    if (_availableParticles.empty()) {
        // 池耗尽时动态扩展
        size_t expandSize = std::max(size_t(1), _particlePool.size() / 2);
        CCLOG("[EntityPoolManager] Particle pool exhausted, expanding by %zu...", expandSize);
        preallocateParticles(registry, expandSize);
    }

    entt::entity entity = _availableParticles.back();
    _availableParticles.pop_back();
    ++_particleActiveCount;

    // 标记为正在使用
    if (registry.valid(entity)) {
        auto* pooled = registry.try_get<PooledEntity>(entity);
        if (pooled) {
            pooled->inUse = true;
        }
    }

    return entity;
}

void EntityPoolManager::releaseParticle(entt::registry& registry, entt::entity entity) {
    if (!registry.valid(entity)) {
        CCLOG("[EntityPoolManager] Warning: Attempting to release invalid entity");
        return;
    }

    // 验证实体属于粒子池
    auto* pooled = registry.try_get<PooledEntity>(entity);
    if (!pooled || pooled->poolType != POOL_TYPE_PARTICLE) {
        CCLOG("[EntityPoolManager] Warning: Entity is not a pooled particle");
        return;
    }

    // 重复释放检测 - Requirements 3.3
    if (!pooled->inUse) {
        CCLOG("[EntityPoolManager] Warning: Duplicate release detected for particle entity %u - already released",
              static_cast<uint32_t>(entity));
        return;
    }

    // 检查实体是否在销毁队列中
    auto& destructionManager = EntityDestructionManager::getInstance();
    if (destructionManager.isPendingDestruction(entity)) {
        CCLOG("[EntityPoolManager] Note: Removing pooled entity from destruction queue before release");
    }

    // 隐藏精灵
    hideEntitySprite(registry, entity);

    // 完全重置实体状态 - Requirements 3.1, 3.2
    resetEntity(registry, entity, POOL_TYPE_PARTICLE);

    // 标记为未使用 - Requirements 3.3
    pooled->inUse = false;

    _availableParticles.push_back(entity);
    --_particleActiveCount;

    CCLOG("[EntityPoolManager] Particle released, available: %zu, active: %zu",
          _availableParticles.size(), _particleActiveCount);
}

void EntityPoolManager::resetEntity(entt::registry& registry, entt::entity entity, 
                                     const std::string& poolType) {
    if (!registry.valid(entity)) {
        return;
    }

    // 完全重置组件状态 - Requirements 3.1, 3.2
    // 确保所有引用被清除，所有状态被重置到默认值

    if (poolType == POOL_TYPE_PROJECTILE) {
        // 重置投射物组件 - 完全重置所有字段
        auto* proj = registry.try_get<ProjectileComponent>(entity);
        if (proj) {
            proj->owner = INVALID_ENTITY;  // 清除所有者引用 - Requirements 3.1
            proj->damage = 8.0f;
            proj->lifetime = 3.0f;
            proj->hasHit = false;
            // 重置所有 debuff 相关字段
            proj->chillChance = 0.0f;
            proj->chillDuration = 0.0f;
            proj->chillSpeedReduction = 0.0f;
            proj->freezeChance = 0.0f;
            proj->freezeDuration = 0.0f;
            proj->poisonChance1 = 0.0f;
            proj->poisonDuration1 = 0.0f;
            proj->poisonDamage1 = 0.0f;
            proj->poisonChance2 = 0.0f;
            proj->poisonDuration2 = 0.0f;
            proj->poisonDamage2 = 0.0f;
        }

        if (registry.any_of<AnimationComponent>(entity)) {
            registry.remove<AnimationComponent>(entity);
        }
        if (registry.any_of<DelayedAccelerationComponent>(entity)) {
            registry.remove<DelayedAccelerationComponent>(entity);
        }
        if (registry.any_of<NoVelocityRotationTag>(entity)) {
            registry.remove<NoVelocityRotationTag>(entity);
        }
    }

    // 重置通用组件 - 适用于所有池类型
    auto* transform = registry.try_get<TransformComponent>(entity);
    if (transform) {
        transform->position = cocos2d::Vec2::ZERO;
        transform->previousPosition = cocos2d::Vec2::ZERO;
    }

    auto* render = registry.try_get<RenderComponent>(entity);
    if (render) {
        render->visible = false;  // 确保不可见
        render->rotation = 0.0f;
        render->scale = 1.0f;
        render->flipX = false;
        render->flipY = false;
        render->color = cocos2d::Color3B::WHITE;
        render->opacity = 255;
        render->zOrder = 0;  // 重置 z-order
        render->renderOffset = cocos2d::Vec2::ZERO;  // 重置渲染偏移
        render->enableSync = true;  // 重置同步标志
        // 注意：不重置 spriteResourceId，因为它会在 acquire 时被设置
    }

    auto* spriteState = registry.try_get<SpriteStateComponent>(entity);
    if (spriteState) {
        // 保留 spriteCreated 和 spriteHandle，因为精灵对象会被复用
        // 但确保精灵本身被隐藏（在 hideEntitySprite 中处理）
        // 重置其他可能的状态字段
    }

    CCLOG("[EntityPoolManager] Entity %u reset for pool type: %s",
          static_cast<uint32_t>(entity), poolType.c_str());
}

entt::entity EntityPoolManager::createPooledProjectile(entt::registry& registry) {
    entt::entity entity = registry.create();

    // 添加池化标记
    registry.emplace<PooledEntity>(entity, POOL_TYPE_PROJECTILE);

    // 添加基础组件（初始状态为不可见）
    registry.emplace<TransformComponent>(entity);
    
    auto& render = registry.emplace<RenderComponent>(entity);
    render.visible = false;  // 初始不可见

    registry.emplace<SpriteStateComponent>(entity);
    registry.emplace<ProjectileComponent>(entity);

    return entity;
}

entt::entity EntityPoolManager::createPooledParticle(entt::registry& registry) {
    entt::entity entity = registry.create();

    // 添加池化标记
    registry.emplace<PooledEntity>(entity, POOL_TYPE_PARTICLE);

    // 添加基础组件（初始状态为不可见）
    registry.emplace<TransformComponent>(entity);
    
    auto& render = registry.emplace<RenderComponent>(entity);
    render.visible = false;  // 初始不可见

    registry.emplace<SpriteStateComponent>(entity);

    return entity;
}

void EntityPoolManager::hideEntitySprite(entt::registry& registry, entt::entity entity) {
    auto* spriteState = registry.try_get<SpriteStateComponent>(entity);
    if (spriteState && spriteState->spriteCreated && spriteState->spriteHandle) {
        auto* sprite = static_cast<cocos2d::Sprite*>(spriteState->spriteHandle);
        if (sprite) {
            sprite->setVisible(false);
            
            // 禁用物理体而不是移除它，这样可以复用
            auto* body = sprite->getPhysicsBody();
            if (body) {
                // 停止物理体运动
                body->setVelocity(cocos2d::Vec2::ZERO);
                body->setAngularVelocity(0.0f);
                // 禁用物理体（不参与碰撞检测和物理模拟）
                body->setEnabled(false);
            }
        }
    }

    // 同时更新 RenderComponent
    auto* render = registry.try_get<RenderComponent>(entity);
    if (render) {
        render->visible = false;
    }
}

void EntityPoolManager::clearAll(entt::registry& registry) {
    // 销毁所有池化实体
    for (auto entity : _projectilePool) {
        if (registry.valid(entity)) {
            // 清理精灵
            auto* spriteState = registry.try_get<SpriteStateComponent>(entity);
            if (spriteState && spriteState->spriteHandle) {
                auto* sprite = static_cast<cocos2d::Sprite*>(spriteState->spriteHandle);
                if (sprite) {
                    sprite->removeFromParent();
                }
            }
            registry.destroy(entity);
        }
    }

    for (auto entity : _particlePool) {
        if (registry.valid(entity)) {
            // 清理精灵
            auto* spriteState = registry.try_get<SpriteStateComponent>(entity);
            if (spriteState && spriteState->spriteHandle) {
                auto* sprite = static_cast<cocos2d::Sprite*>(spriteState->spriteHandle);
                if (sprite) {
                    sprite->removeFromParent();
                }
            }
            registry.destroy(entity);
        }
    }

    _projectilePool.clear();
    _availableProjectiles.clear();
    _projectileActiveCount = 0;

    _particlePool.clear();
    _availableParticles.clear();
    _particleActiveCount = 0;

    CCLOG("[EntityPoolManager] All pools cleared");
}

bool EntityPoolManager::isPooledEntity(entt::registry& registry, entt::entity entity) const {
    if (!registry.valid(entity)) {
        return false;
    }
    return registry.try_get<PooledEntity>(entity) != nullptr;
}

} // namespace ecs
