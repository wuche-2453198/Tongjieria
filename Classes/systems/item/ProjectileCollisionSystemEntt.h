#ifndef __ECS_SYSTEM_PROJECTILECOLLISIONSYSTEMENTT_H__
#define __ECS_SYSTEM_PROJECTILECOLLISIONSYSTEMENTT_H__

#include "systems/core/ISystemEntt.h"
#include "systems/core/SystemPriority.h"
#include "systems/core/EntityDestructionManager.h"
#include "systems/core/EntityPoolManager.h"
#include "components/AllComponents.h"
#include "components/block/block_component.h"
#include "core/assets_manager.h"
#include "core/block_world.h"
#include "cocos2d.h"
#include <vector>

namespace ecs {

/**
 * @brief 投射物碰撞系统 - 处理投射物与障碍物、目标的碰撞
 * 需要通过PhysicsContactHandler在场景中注册使用
 * 
 * 使用延迟销毁机制：
 * - 池化实体使用 EntityPoolManager.releaseProjectile() 归还到池中
 * - 非池化实体使用 EntityDestructionManager.queueDestruction() 延迟销毁
 * 
 * Requirements: 3.4, 1.1
 */
class ProjectileCollisionSystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "ProjectileCollisionSystem"; }
    int getPriority() const override { return SystemPriority::COLLISION; }
    
    void update(float delta) override {
        // 延迟销毁在碰撞回调中标记的投射物
        // 避免在物理回调期间销毁实体导致其他系统访问无效实体
        // 
        // Requirements: 3.4, 1.1
        // - 池化实体使用 EntityPoolManager.releaseProjectile()
        // - 非池化实体使用 EntityDestructionManager.queueDestruction()
        for (auto entity : _entitiesToDestroy) {
            if (_registry && _registry->valid(entity)) {
                auto& poolManager = EntityPoolManager::getInstance();
                
                // 检查是否是池化实体
                if (poolManager.isPooledEntity(*_registry, entity)) {
                    // 池化实体：归还到池中复用
                    poolManager.releaseProjectile(*_registry, entity);
                    CCLOG(">>> PROJECTILE RELEASED: Entity %u returned to pool", 
                          entt::to_integral(entity));
                } else {
                    // 非池化实体：使用延迟销毁队列
                    EntityDestructionManager::getInstance().queueDestruction(*_registry, entity);
                    CCLOG(">>> PROJECTILE QUEUED FOR DESTRUCTION: Entity %u", 
                          entt::to_integral(entity));
                }
            }
        }
        _entitiesToDestroy.clear();
    }

    /**
     * @brief 处理投射物碰撞
     * @param projEntity 投射物实体
     * @param otherEntity 碰撞目标实体
     * @param otherBody 碰撞目标物理体
     * @return true表示碰撞已处理
     */
    bool handleProjectileCollision(ecs::ProjectileComponent* proj, 
                                   entt::entity projEntity,
                                   ecs::EntityId otherEntity,
                                   cocos2d::PhysicsBody* otherBody) {
        return handleProjectileCollision(proj, projEntity, otherEntity, otherBody, nullptr);
    }

    bool handleProjectileCollision(ecs::ProjectileComponent* proj, 
                                   entt::entity projEntity,
                                   ecs::EntityId otherEntity,
                                   cocos2d::PhysicsBody* otherBody,
                                   const cocos2d::Vec2* contactWorldPos) {
        if (!proj || !_registry) return false;
        
        // 已经碰撞过，跳过
        if (proj->hasHit) return true;
        
        // 碰到owner，跳过
        if (otherEntity == proj->owner) return true;
        
        // 碰到非动态物体（地面、墙壁等）
        if (otherBody && !otherBody->isDynamic()) {
            auto* blockPhysicsRef = _registry->ctx().find<BlockPhysicsWorldRef>();
            const bool isBlockPhysicsBody = (blockPhysicsRef && blockPhysicsRef->body && blockPhysicsRef->body == otherBody);

            if (isBlockPhysicsBody) {
                cocos2d::Vec2 worldPos = cocos2d::Vec2::ZERO;
                if (contactWorldPos) {
                    worldPos = *contactWorldPos;
                } else if (auto* transform = _registry->try_get<ecs::TransformComponent>(projEntity)) {
                    worldPos = transform->position;
                }

                auto& blockWorld = _registry->ctx().get<BlockWorld>();
                auto& assetManager = _registry->ctx().get<AssetManager>();

                BlockHandle block = blockWorld.getBlockAtWorldPos(LayerType::BLOCK, worldPos);
                if (!block.isIDVailed()) {
                    block = blockWorld.getBlockAtWorldPos(LayerType::WALL, worldPos);
                }

                if (block.isIDVailed()) {
                    bool projectilePass = false;
                    const auto& config = assetManager.getBlockConfig(block.id.value());
                    if (auto* origin = config.getOrigin(); origin && origin->HasMember("collision") && (*origin)["collision"].IsObject()) {
                        const auto& collisionObj = (*origin)["collision"];
                        if (collisionObj.HasMember("projectilePass") && collisionObj["projectilePass"].IsBool()) {
                            projectilePass = collisionObj["projectilePass"].GetBool();
                        }
                    }

                    if (projectilePass) {
                        return true;
                    }
                }
            }

            proj->hasHit = true;
            CCLOG(">>> PROJECTILE COLLISION: Entity %u hit static body (ground/wall) and disappeared", 
                  entt::to_integral(projEntity));
            CCLOG(">>> COLLISION DEBUG: otherBody isDynamic=%s, categoryBitmask=0x%04X", 
                  otherBody->isDynamic() ? "true" : "false", otherBody->getCategoryBitmask());

            destroyProjectile(projEntity);
            return true;
        }

        if (otherEntity != ecs::INVALID_ENTITY) {
            auto otherEnt = static_cast<entt::entity>(otherEntity);
            if (_registry->valid(otherEnt)) {
                auto* player = _registry->try_get<ecs::PlayerTag>(otherEnt);
                auto* health = _registry->try_get<ecs::HealthComponent>(otherEnt);
                if (player && health) {
                    if (health->invincibleTimer <= 0.0f) {
                        health->takeDamage(proj->damage);
                        health->invincibleTimer = health->invincibleTime;
                    }
                    proj->hasHit = true;
                    destroyProjectile(projEntity);
                    return true;
                }
            }
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
     * @brief 销毁投射物 - 延迟销毁，避免在物理回调期间销毁实体
     */
    void destroyProjectile(entt::entity projEntity) {
        if (!_registry || !_registry->valid(projEntity)) return;
        
        // 标记为待销毁，在update()中统一处理
        _entitiesToDestroy.push_back(projEntity);
    }

private:
    std::vector<entt::entity> _entitiesToDestroy;
};

} // namespace ecs

#endif // __ECS_SYSTEM_PROJECTILECOLLISIONSYSTEMENTT_H__
