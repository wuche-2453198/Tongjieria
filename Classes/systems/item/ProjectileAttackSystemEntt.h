#ifndef __ECS_SYSTEM_PROJECTILEATTACKSYSTEMENTT_H__
#define __ECS_SYSTEM_PROJECTILEATTACKSYSTEMENTT_H__

#include "systems/core/ISystemEntt.h"
#include "systems/core/SystemPriority.h"
#include "systems/core/EntityPoolManager.h"
#include "components/AllComponents.h"
#include "systems/render/SpriteManager.h"
#include "cocos2d.h"
#include <cmath>
#include <unordered_set>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace ecs {

/**
 * @brief 投射物攻击系统 - 处理投射物发射
 * 
 * 优化：使用 EntityPoolManager 进行实体复用，避免频繁创建/销毁
 * Requirements: 4.1, 4.2
 */
class ProjectileAttackSystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "ProjectileAttackSystem"; }
    int getPriority() const override { return SystemPriority::AI + 10; }

    /**
     * @brief 初始化投射物池
     * @param initialPoolSize 初始池大小
     */
    void initializePool(size_t initialPoolSize = 50) {
        if (_registry && !_poolInitialized) {
            EntityPoolManager::getInstance().preallocateProjectiles(*_registry, initialPoolSize);
            _poolInitialized = true;
            CCLOG("ProjectileAttackSystem: Initialized projectile pool with %zu entities", initialPoolSize);
        }
    }

    void update(float delta) override {
        // 延迟初始化池（确保 registry 已设置）
        if (!_poolInitialized && _registry) {
            initializePool();
        }

        auto view = _registry->view<ProjectileAttackComponent, AggroComponent, 
                                     TransformComponent, SpriteStateComponent, RenderComponent,
                                     GroundDetectorComponent, JumpMovementComponent>();
        
        view.each([delta, this](auto entity, ProjectileAttackComponent& attack,
                               AggroComponent& aggro, TransformComponent& transform,
                               SpriteStateComponent& state, RenderComponent& render,
                               GroundDetectorComponent& ground, JumpMovementComponent& jump) {
            
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
                // 更新朝向
                if (aggro.directionToTarget.x != 0) {
                    render.flipX = aggro.directionToTarget.x < 0;
                }
                
                // 只有在地面上且可以发射时才发射
                if (attack.canFire && ground.isOnGround) {
                    fireProjectiles(entity, attack, transform, aggro, state);
                    attack.canFire = false;
                    attack.fireTimer = 0.0f;
                }
            }
        });
    }

private:
    // 缓存已注册的投射物资源ID，避免重复注册
    std::unordered_set<std::string> _registeredProjectileResources;
    bool _poolInitialized = false;
    
    void fireProjectiles(entt::entity owner, ProjectileAttackComponent& attack,
                        TransformComponent& transform, AggroComponent& aggro,
                        SpriteStateComponent& ownerState) {
        
        cocos2d::Node* parentNode = nullptr;
        if (ownerState.spriteCreated && ownerState.spriteHandle) {
            auto* sprite = static_cast<cocos2d::Sprite*>(ownerState.spriteHandle);
            parentNode = sprite->getParent();
        }
        if (!parentNode) return;
        
        // 使用投射物精灵路径作为资源ID（复用相同纹理的资源）
        std::string projResourceId = "projectile_" + attack.projectileSpritePath;
        
        // 只在首次使用时注册资源
        if (_registeredProjectileResources.find(projResourceId) == _registeredProjectileResources.end()) {
            ecs::SpriteResourceDescriptor projDescriptor;
            projDescriptor.resourceId = projResourceId;
            projDescriptor.spritePath = attack.projectileSpritePath;
            ecs::SpriteManager::getInstance().registerResource(projDescriptor);
            _registeredProjectileResources.insert(projResourceId);
            CCLOG("ProjectileAttackSystem: Registered projectile resource '%s'", projResourceId.c_str());
        }
        
        // 发射方向固定为正上方散开
        float baseAngle = 90.0f;

        float spreadStep = attack.projectileCount > 1 ?
                          attack.horizontalSpread / (attack.projectileCount - 1) : 0;
        float startAngle = baseAngle - spreadStep * (attack.projectileCount - 1) * 0.5f;
        
        auto& poolManager = EntityPoolManager::getInstance();
        
        for (int i = 0; i < attack.projectileCount; i++) {
            float angle = startAngle + spreadStep * i;
            float radians = angle * M_PI / 180.0f;
            
            cocos2d::Vec2 velocity;
            velocity.x = cos(radians) * attack.projectileSpeed;
            velocity.y = sin(radians) * attack.verticalImpulse;
            
            // 从池中获取投射物实体
            entt::entity projectile = poolManager.acquireProjectile(*_registry);
            
            // 配置 TransformComponent
            auto* projTransform = _registry->try_get<TransformComponent>(projectile);
            if (projTransform) {
                projTransform->position = transform.position;
                projTransform->velocity = velocity;
                projTransform->previousPosition = transform.position;
            } else {
                auto& newTransform = _registry->emplace<TransformComponent>(projectile);
                newTransform.position = transform.position;
                newTransform.velocity = velocity;
            }
            
            // 配置 ProjectileComponent
            auto* projComp = _registry->try_get<ProjectileComponent>(projectile);
            if (projComp) {
                projComp->owner = entt::to_integral(owner);
                projComp->damage = attack.projectileDamage;
                projComp->lifetime = attack.projectileLifetime;
                projComp->hasHit = false;
                projComp->chillChance = attack.chillChance;
                projComp->chillDuration = attack.chillDuration;
                projComp->chillSpeedReduction = attack.chillSpeedReduction;
                projComp->freezeChance = attack.freezeChance;
                projComp->freezeDuration = attack.freezeDuration;
                projComp->poisonChance1 = attack.poisonChance1;
                projComp->poisonDuration1 = attack.poisonDuration1;
                projComp->poisonDamage1 = attack.poisonDamage1;
                projComp->poisonChance2 = attack.poisonChance2;
                projComp->poisonDuration2 = attack.poisonDuration2;
                projComp->poisonDamage2 = attack.poisonDamage2;
            } else {
                auto& newProj = _registry->emplace<ProjectileComponent>(projectile);
                newProj.owner = entt::to_integral(owner);
                newProj.damage = attack.projectileDamage;
                newProj.lifetime = attack.projectileLifetime;
                newProj.chillChance = attack.chillChance;
                newProj.chillDuration = attack.chillDuration;
                newProj.chillSpeedReduction = attack.chillSpeedReduction;
                newProj.freezeChance = attack.freezeChance;
                newProj.freezeDuration = attack.freezeDuration;
                newProj.poisonChance1 = attack.poisonChance1;
                newProj.poisonDuration1 = attack.poisonDuration1;
                newProj.poisonDamage1 = attack.poisonDamage1;
                newProj.poisonChance2 = attack.poisonChance2;
                newProj.poisonDuration2 = attack.poisonDuration2;
                newProj.poisonDamage2 = attack.poisonDamage2;
            }
            
            // 配置 RenderComponent
            auto* projRender = _registry->try_get<RenderComponent>(projectile);
            if (projRender) {
                projRender->spriteResourceId = projResourceId;
                projRender->scale = 1.0f;
                projRender->zOrder = 2;
                projRender->visible = true;
                projRender->rotation = 0.0f;
                projRender->flipX = false;
                projRender->flipY = false;
            } else {
                auto& newRender = _registry->emplace<RenderComponent>(projectile);
                newRender.spriteResourceId = projResourceId;
                newRender.scale = 1.0f;
                newRender.zOrder = 2;
                newRender.visible = true;
            }
            
            // 配置 ParentNodeComponent
            auto* projParent = _registry->try_get<ParentNodeComponent>(projectile);
            if (projParent) {
                projParent->parentNode = parentNode;
            } else {
                auto& newParent = _registry->emplace<ParentNodeComponent>(projectile);
                newParent.parentNode = parentNode;
            }
            
            // 配置 SpriteStateComponent
            // 对于池化实体，精灵和物理体应该被复用（物理体被禁用而非移除）
            bool hasSpriteState = _registry->all_of<SpriteStateComponent>(projectile);
            CCLOG("ProjectileAttackSystem: Projectile entity %u, hasSpriteState=%d, resourceId='%s'",
                  entt::to_integral(projectile), hasSpriteState ? 1 : 0, projResourceId.c_str());
            
            if (hasSpriteState) {
                auto* projState = _registry->try_get<SpriteStateComponent>(projectile);
                if (projState && projState->spriteCreated && projState->spriteHandle) {
                    auto* sprite = static_cast<cocos2d::Sprite*>(projState->spriteHandle);
                    if (sprite && sprite->getParent()) {
                        // 复用精灵
                        sprite->setVisible(true);
                        sprite->setPosition(transform.position);
                        
                        // 检查物理体
                        auto* body = sprite->getPhysicsBody();
                        if (body) {
                            // 重新启用物理体并应用速度
                            body->setEnabled(true);
                            body->setVelocity(velocity);
                            CCLOG("ProjectileAttackSystem: Reusing sprite and physics body for projectile %u", entt::to_integral(projectile));
                        } else {
                            // 物理体不存在（不应该发生），需要重新创建
                            _registry->remove<SpriteStateComponent>(projectile);
                            sprite->removeFromParent();
                            CCLOG("ProjectileAttackSystem: No physics body found, recreating sprite for projectile %u", entt::to_integral(projectile));
                        }
                    } else {
                        // 精灵无效，移除组件让RenderSystem重新创建
                        _registry->remove<SpriteStateComponent>(projectile);
                        CCLOG("ProjectileAttackSystem: Invalid sprite, recreating for projectile %u", entt::to_integral(projectile));
                    }
                } else {
                    // 精灵未创建，移除组件让RenderSystem创建
                    _registry->remove<SpriteStateComponent>(projectile);
                    CCLOG("ProjectileAttackSystem: Empty SpriteStateComponent, recreating for projectile %u", entt::to_integral(projectile));
                }
            } else {
                CCLOG("ProjectileAttackSystem: Projectile %u has no SpriteStateComponent, RenderSystem will create sprite", entt::to_integral(projectile));
            }
            
            // 验证组件状态
            bool hasRender = _registry->all_of<RenderComponent>(projectile);
            bool hasParent = _registry->all_of<ParentNodeComponent>(projectile);
            bool hasSpriteStateAfter = _registry->all_of<SpriteStateComponent>(projectile);
            CCLOG("ProjectileAttackSystem: After setup - entity %u: hasRender=%d, hasParent=%d, hasSpriteState=%d",
                  entt::to_integral(projectile), hasRender ? 1 : 0, hasParent ? 1 : 0, hasSpriteStateAfter ? 1 : 0);
            
            // 配置 PhysicsBodyComponent（优化：使用圆形物理体）
            auto* projPhysics = _registry->try_get<PhysicsBodyComponent>(projectile);
            if (projPhysics) {
                projPhysics->shape = PhysicsBodyComponent::BodyShape::Circle;
                projPhysics->radius = std::min(attack.projectileSpriteWidth, attack.projectileSpriteHeight) / 2.0f;
                projPhysics->density = 0.1f;
                projPhysics->restitution = 0.0f;
                projPhysics->friction = 0.0f;
                projPhysics->dynamic = true;
                projPhysics->rotationEnabled = true;
                projPhysics->gravityEnabled = attack.useGravity;
                projPhysics->velocityLimit = std::max(500.0f, attack.projectileSpeed * 2.0f);
                projPhysics->contactTestBitmask = 0xFFFFFFFF;
                projPhysics->collisionBitmask = 0x0001;
                projPhysics->categoryBitmask = 0x0004;
                projPhysics->group = -2;
                projPhysics->needsCreation = true;  // 标记需要创建物理体
            } else {
                auto& newPhysics = _registry->emplace<PhysicsBodyComponent>(projectile);
                newPhysics.shape = PhysicsBodyComponent::BodyShape::Circle;
                newPhysics.radius = std::min(attack.projectileSpriteWidth, attack.projectileSpriteHeight) / 2.0f;
                newPhysics.density = 0.1f;
                newPhysics.restitution = 0.0f;
                newPhysics.friction = 0.0f;
                newPhysics.dynamic = true;
                newPhysics.rotationEnabled = true;
                newPhysics.gravityEnabled = attack.useGravity;
                newPhysics.velocityLimit = std::max(500.0f, attack.projectileSpeed * 2.0f);
                newPhysics.contactTestBitmask = 0xFFFFFFFF;
                newPhysics.collisionBitmask = 0x0001;
                newPhysics.categoryBitmask = 0x0004;
                newPhysics.group = -2;
            }
            
            // 配置 InitialVelocityComponent
            auto* initialVel = _registry->try_get<InitialVelocityComponent>(projectile);
            if (initialVel) {
                initialVel->velocity = velocity;
                initialVel->applied = false;
            } else {
                auto& newVel = _registry->emplace<InitialVelocityComponent>(projectile);
                newVel.velocity = velocity;
            }
        }
    }
};

} // namespace ecs

#endif // __ECS_SYSTEM_PROJECTILEATTACKSYSTEMENTT_H__
