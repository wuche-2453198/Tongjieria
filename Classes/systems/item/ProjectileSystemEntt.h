#ifndef __ECS_SYSTEM_PROJECTILESYSTEMENTT_H__
#define __ECS_SYSTEM_PROJECTILESYSTEMENTT_H__

#include "systems/core/ISystemEntt.h"
#include "systems/core/SystemPriority.h"
#include "systems/core/EntityPoolManager.h"
#include "components/AllComponents.h"
#include "cocos2d.h"
#include <vector>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace ecs {

/**
 * @brief 投射物系统 - 更新投射物位置、旋转和生命周期
 * 
 * 优化：使用 EntityPoolManager 进行实体复用，避免频繁创建/销毁
 * Requirements: 4.1, 4.2
 */
class ProjectileSystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "ProjectileSystem"; }
    int getPriority() const override { return SystemPriority::PHYSICS + 5; }

    void update(float delta) override {
        std::vector<entt::entity> toRelease;
        
        auto view = _registry->view<ProjectileComponent, SpriteStateComponent, RenderComponent>();
        
        for (auto entity : view) {
            auto& proj = view.get<ProjectileComponent>(entity);
            auto& state = view.get<SpriteStateComponent>(entity);
            auto& render = view.get<RenderComponent>(entity);
            
            // 跳过未使用的池化实体
            auto* pooled = _registry->try_get<PooledEntity>(entity);
            if (pooled && !pooled->inUse) {
                continue;
            }
            
            // 更新生命周期
            proj.lifetime -= delta;
            if (proj.lifetime <= 0 || proj.hasHit) {
                toRelease.push_back(entity);
                continue;
            }
            
            // 从物理体同步位置和旋转
            if (state.spriteCreated && state.spriteHandle) {
                auto* sprite = static_cast<cocos2d::Sprite*>(state.spriteHandle);
                auto* body = sprite->getPhysicsBody();
                if (body) {
                    auto* delayed = _registry->try_get<DelayedAccelerationComponent>(entity);
                    if (delayed) {
                        if (delayed->delayRemaining > 0.0f) {
                            delayed->delayRemaining -= delta;
                            if (delayed->delayRemaining < 0.0f) {
                                delayed->delayRemaining = 0.0f;
                            }
                            body->setVelocity(cocos2d::Vec2::ZERO);
                        } else {
                            delayed->currentSpeed += delayed->acceleration * delta;
                            if (delayed->currentSpeed > delayed->maxSpeed) {
                                delayed->currentSpeed = delayed->maxSpeed;
                            }
                            body->setVelocity(delayed->direction * delayed->currentSpeed);
                        }
                    }

                    cocos2d::Vec2 velocity = body->getVelocity();
                    
                    // 根据速度方向更新旋转
                    if (!_registry->any_of<NoVelocityRotationTag>(entity)) {
                        if (velocity.lengthSquared() > 1.0f) {
                            float rotAngle = atan2(velocity.y, velocity.x) * 180.0f / M_PI;
                            render.rotation = -rotAngle + 90.0f;
                        }
                    }
                }
            }
        }
        
        // 释放过期的投射物（归还到池或销毁）
        auto& poolManager = EntityPoolManager::getInstance();
        for (auto entity : toRelease) {
            // 检查是否是池化实体
            if (poolManager.isPooledEntity(*_registry, entity)) {
                // 池化实体：归还到池中复用（releaseProjectile 会处理精灵和物理体）
                poolManager.releaseProjectile(*_registry, entity);
            } else {
                // 非池化实体：清理精灵后销毁
                cleanupProjectileSprite(entity);
                _registry->destroy(entity);
            }
        }
    }

private:
    /**
     * @brief 清理投射物精灵和物理体（仅用于非池化实体）
     */
    void cleanupProjectileSprite(entt::entity entity) {
        auto* state = _registry->try_get<SpriteStateComponent>(entity);
        if (state && state->spriteCreated && state->spriteHandle) {
            auto* sprite = static_cast<cocos2d::Sprite*>(state->spriteHandle);
            if (sprite) {
                // 移除物理体
                auto* body = sprite->getPhysicsBody();
                if (body) {
                    sprite->removeComponent(body);
                }
                // 从父节点移除精灵
                sprite->removeFromParent();
            }
        }
    }
};

} // namespace ecs

#endif // __ECS_SYSTEM_PROJECTILESYSTEMENTT_H__
