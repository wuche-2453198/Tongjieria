#ifndef __ECS_SYSTEM_PROJECTILEATTACKSYSTEMENTT_H__
#define __ECS_SYSTEM_PROJECTILEATTACKSYSTEMENTT_H__

#include "systems/core/ISystemEntt.h"
#include "systems/core/SystemPriority.h"
#include "components/AllComponents.h"
#include "systems/render/SpriteManager.h"
#include "cocos2d.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace ecs {

/**
 * @brief 投射物攻击系统 - 处理投射物发射
 */
class ProjectileAttackSystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "ProjectileAttackSystem"; }
    int getPriority() const override { return SystemPriority::AI + 10; }

    void update(float delta) override {
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
                // 阻止跳跃
                jump.readyToJump = false;
                jump.jumpTimer = 0.0f;
                
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
    void fireProjectiles(entt::entity owner, ProjectileAttackComponent& attack,
                        TransformComponent& transform, AggroComponent& aggro,
                        SpriteStateComponent& ownerState) {
        
        cocos2d::Node* parentNode = nullptr;
        if (ownerState.spriteCreated && ownerState.spriteHandle) {
            auto* sprite = static_cast<cocos2d::Sprite*>(ownerState.spriteHandle);
            parentNode = sprite->getParent();
        }
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
            
            // 新架构：注册投射物精灵资源
            std::string projResourceId = "projectile_" + std::to_string(entt::to_integral(projectile));
            ecs::SpriteResourceDescriptor projDescriptor;
            projDescriptor.resourceId = projResourceId;
            projDescriptor.spritePath = attack.projectileSpritePath;
            ecs::SpriteManager::getInstance().registerResource(projDescriptor);
            
            // 添加渲染组件
            auto& projRender = _registry->emplace<RenderComponent>(projectile);
            projRender.spriteResourceId = projResourceId;
            projRender.scale = 1.0f;
            projRender.zOrder = 2;
            projRender.visible = true;
            
            // 添加父节点组件
            auto& projParent = _registry->emplace<ParentNodeComponent>(projectile);
            projParent.parentNode = parentNode;
            
            // 添加物理体组件
            auto& projPhysics = _registry->emplace<PhysicsBodyComponent>(projectile);
            projPhysics.shape = PhysicsBodyComponent::BodyShape::Box;
            projPhysics.width = attack.projectileSpriteWidth;
            projPhysics.height = attack.projectileSpriteHeight;
            projPhysics.density = 0.1f;
            projPhysics.restitution = 0.0f;
            projPhysics.friction = 0.0f;
            projPhysics.dynamic = true;
            projPhysics.rotationEnabled = true;
            projPhysics.gravityEnabled = attack.useGravity;
            projPhysics.contactTestBitmask = 0xFFFFFFFF;
            projPhysics.collisionBitmask = 0x0001;
            projPhysics.categoryBitmask = 0x0004;
            projPhysics.group = -2;
            
            // 添加初始速度组件，RenderSystem会在创建物理体后应用
            auto& initialVel = _registry->emplace<InitialVelocityComponent>(projectile);
            initialVel.velocity = velocity;
            
            CCLOG("Fired Ice Spike %d at angle %.1f, velocity (%.1f, %.1f)", 
                  i, angle, velocity.x, velocity.y);
        }
        
        CCLOG("Entity %u: Fired %d ice spikes!", entt::to_integral(owner), attack.projectileCount);
    }
};

} // namespace ecs

#endif // __ECS_SYSTEM_PROJECTILEATTACKSYSTEMENTT_H__
