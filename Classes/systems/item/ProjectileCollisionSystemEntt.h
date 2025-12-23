#ifndef __ECS_SYSTEM_PROJECTILECOLLISIONSYSTEMENTT_H__
#define __ECS_SYSTEM_PROJECTILECOLLISIONSYSTEMENTT_H__

#include "systems/core/ISystemEntt.h"
#include "systems/core/SystemPriority.h"
#include "components/AllComponents.h"
#include "cocos2d.h"

namespace ecs {

/**
 * @brief 投射物碰撞系统 - 处理投射物与障碍物、目标的碰撞
 * 需要通过PhysicsContactHandler在场景中注册使用
 */
class ProjectileCollisionSystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "ProjectileCollisionSystem"; }
    int getPriority() const override { return SystemPriority::COLLISION; }
    
    void update(float delta) override {
        // 碰撞处理通过handleProjectileCollision方法在物理接触监听器中调用
        // 此update方法为空，因为碰撞处理是事件驱动的
    }

    /**
     * @brief 处理投射物碰撞
     * @param proj 投射物组件
     * @param projEntity 投射物实体
     * @param otherEntity 碰撞目标实体
     * @param otherBody 碰撞目标物理体
     * @return true表示碰撞已处理
     */
    bool handleProjectileCollision(ecs::ProjectileComponent* proj, 
                                   entt::entity projEntity,
                                   ecs::EntityId otherEntity,
                                   cocos2d::PhysicsBody* otherBody) {
        if (!proj || !_registry) return false;
        
        // 已经碰撞过，跳过
        if (proj->hasHit) return true;
        
        // 碰到owner，跳过
        if (otherEntity == proj->owner) return true;
        
        // 碰到非动态物体（地面、墙壁等）直接消失
        if (otherBody && !otherBody->isDynamic()) {
            proj->hasHit = true;
            CCLOG(">>> PROJECTILE COLLISION: Entity %u hit static body (ground/wall) and disappeared", 
                  entt::to_integral(projEntity));
            CCLOG(">>> COLLISION DEBUG: otherBody isDynamic=%s, categoryBitmask=0x%04X", 
                  otherBody->isDynamic() ? "true" : "false", otherBody->getCategoryBitmask());
            
            destroyProjectile(projEntity);
            return true;
        }
        
        // 碰到KingSlime应用伤害
        if (otherEntity != ecs::INVALID_ENTITY) {
            auto otherEnt = static_cast<entt::entity>(otherEntity);
            if (_registry->valid(otherEnt)) {
                auto *kingSlime = _registry->try_get<ecs::KingSlimeComponent>(otherEnt);
                if (kingSlime) {
                    auto *health = _registry->try_get<ecs::HealthComponent>(otherEnt);
                    if (health) {
                        health->currentHealth -= proj->damage;
                        proj->hasHit = true;
                        
                        CCLOG(">>> DAMAGE APPLIED: %.0f damage, KingSlime Health: %.0f/%.0f", 
                              proj->damage, health->currentHealth, health->maxHealth);
                        
                        destroyProjectile(projEntity);
                        return true;
                    }
                }
            }
        }
        
        return false;
    }
    
    /**
     * @brief 销毁投射物
     */
    void destroyProjectile(entt::entity projEntity) {
        if (!_registry || !_registry->valid(projEntity)) return;
        
        // 新架构：直接销毁实体，SpriteDestructionObserver会自动清理sprite
        _registry->destroy(projEntity);
    }
};

} // namespace ecs

#endif // __ECS_SYSTEM_PROJECTILECOLLISIONSYSTEMENTT_H__
