#ifndef __ECS_SYSTEM_JUMPMOVEMENTSYSTEMENTT_H__
#define __ECS_SYSTEM_JUMPMOVEMENTSYSTEMENTT_H__

#include "systems/core/ISystemEntt.h"
#include "systems/core/SystemPriority.h"
#include "components/AllComponents.h"
#include "cocos2d.h"

namespace ecs {

/**
 * @brief 跳跃移动系统 - 管理跳跃冷却和执行跳跃
 */
class JumpMovementSystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "JumpMovementSystem"; }
    int getPriority() const override { return SystemPriority::MOVEMENT - 10; }

    void update(float delta) override {
        auto view = _registry->view<JumpMovementComponent, SpriteStateComponent, RenderComponent, GroundDetectorComponent, AggroComponent, TransformComponent>();
        
        view.each([this, delta](auto entity, JumpMovementComponent& jump,
                               SpriteStateComponent& state,
                               RenderComponent& render,
                               GroundDetectorComponent& ground,
                               AggroComponent& aggro,
                               TransformComponent& transform) {
            if (!state.spriteCreated || !state.spriteHandle)
                return;

            auto* sprite = static_cast<cocos2d::Sprite*>(state.spriteHandle);
            auto* body = sprite->getPhysicsBody();

            // 只要在地面就计时冷却
            if (ground.isOnGround) {
                jump.jumpTimer += delta;
                if (jump.jumpTimer >= jump.jumpCooldown) {
                    jump.readyToJump = true;
                }
            }

            // 执行跳跃（如果不在传送中）
            if (jump.readyToJump && ground.isOnGround) {
                // 检查是否有KingSlimeComponent且正在传送
                auto* kingSlime = _registry->try_get<KingSlimeComponent>(entity);
                if (kingSlime && kingSlime->isTeleporting) {
                    // 传送中，跳过冲量应用
                    return;
                }
                
                cocos2d::Vec2 impulse;

                // 更新追踪状态
                jump.isChasing = aggro.hasAggro;

                if (aggro.hasAggro && aggro.targetEntity != INVALID_ENTITY) {
                    // 有仇恨目标：智能追踪跳跃
                    auto targetEntity = static_cast<entt::entity>(aggro.targetEntity);
                    auto* targetTransform = _registry->try_get<TransformComponent>(targetEntity);
                    if (targetTransform) {
                        // 使用transform.position而不是body->getPosition()
                        // body->getPosition()返回的是相对于精灵的偏移量，不是世界坐标
                        float heightDiff = targetTransform->position.y - transform.position.y;
                        impulse = jump.calculateChaseImpulse(aggro.directionToTarget.x,
                                                            heightDiff,
                                                            aggro.distanceToTarget);

                        // 更新朝向
                        render.flipX = aggro.directionToTarget.x < 0;
                    }
                } else {
                    // 无仇恨目标：随机巡逻跳跃
                    impulse = jump.calculateRandomImpulse();
                    render.flipX = jump.randomDirection < 0;
                }

                // 应用冲量
                body->applyImpulse(impulse);
                jump.lastJumpImpulse = impulse;

                // 注意：跳跃动画由AnimationSystem处理，这里不需要手动触发

                // 重置状态
                jump.readyToJump = false;
                jump.jumpTimer = 0.0f;
                ground.isOnGround = false;
                ground.groundContactCount = 0;

                CCLOG("Entity %u: Jump (%.1f, %.1f) %s", entt::to_integral(entity), 
                      impulse.x, impulse.y, jump.isChasing ? "CHASE" : "PATROL");
            }
        });
    }
};

} // namespace ecs

#endif // __ECS_SYSTEM_JUMPMOVEMENTSYSTEMENTT_H__
