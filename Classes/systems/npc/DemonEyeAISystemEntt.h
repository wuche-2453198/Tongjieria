#ifndef __ECS_SYSTEM_DEMONEYEAISYSTEMENTT_H__
#define __ECS_SYSTEM_DEMONEYEAISYSTEMENTT_H__

#include "systems/npc/OptimizedAISystemBase.h"
#include "systems/core/SystemPriority.h"
#include "components/AllComponents.h"
#include "systems/core/AnimationStateHelper.h"
#include "cocos2d.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace ecs {

/**
 * @brief 恶魔眼AI系统 - 飞行追踪类怪物AI
 * 
 * 行为特点：
 * - 飞行追踪玩家（无重力）
 * - 缓慢转向，转弯速率较慢
 * - 撞墙/物块时弧形回弹
 * - 被击退时弧形轨迹回弹
 * 
 * 优化特性（继承自 OptimizedAISystemBase）：
 * - 离屏实体降频更新
 * - 空闲实体降频更新
 * - 远距离实体使用简化 AI
 * 
 * Requirements: 5.1, 5.5, 5.6
 */
class DemonEyeAISystemEntt : public OptimizedAISystemBase {
public:
    const char* getName() const override { return "DemonEyeAISystem"; }
    int getPriority() const override { return SystemPriority::MOVEMENT; }

    void update(float delta) override {
        // 增加帧计数器
        incrementFrameCounter();
        
        auto view = _registry->view<DemonEyeMovementComponent, AggroComponent,
                                     SpriteStateComponent, RenderComponent, TransformComponent>();
        
        view.each([delta, this](entt::entity entity, DemonEyeMovementComponent& demon,
                               AggroComponent& aggro, SpriteStateComponent& state,
                               RenderComponent& render, TransformComponent& transform) {
            
            // 获取动画状态组件（如果存在）
            auto* animState = _registry->try_get<AnimationStateComponent>(entity);
            
            if (!state.spriteCreated || !state.spriteHandle) return;
            
            // 优化：检查是否应该更新此实体
            auto* stateFlags = _registry->try_get<EntityStateFlags>(entity);
            if (!shouldUpdateEntity(entity, stateFlags)) {
                return; // 跳过此帧的更新
            }
            
            auto* sprite = static_cast<cocos2d::Sprite*>(state.spriteHandle);

            // 获取目标位置（如果有仇恨）
            cocos2d::Vec2 targetPos = cocos2d::Vec2::ZERO;
            bool hasTarget = false;
            
            // 验证aggro组件状态
            if (aggro.hasAggro && aggro.targetEntity != INVALID_ENTITY) {
                targetPos = transform.position + aggro.directionToTarget * aggro.distanceToTarget;
                hasTarget = true;
            }

            // 更新冲刺冷却
            if (demon.dashCooldownTimer > 0.0f) {
                demon.dashCooldownTimer -= delta;
            }
            
            if (!hasTarget) {
                // 无目标：巡逻
                if (demon.aiState != DemonEyeMovementComponent::HOVERING) {
                    demon.aiState = DemonEyeMovementComponent::HOVERING;
                    // 更新动画状态
                    if (animState) {
                        AnimationStateHelper::updateAnimationState(*animState,
                            AnimationStateHelper::demonEyeStateToAnimationState(demon.aiState));
                    }
                }
                updatePatrol(demon, delta);
                demon.smoothTurn(delta);
            } else {
                // 有目标：根据状态执行AI
                cocos2d::Vec2 toTarget = targetPos - transform.position;
                float dist = toTarget.length();
                
                switch (demon.aiState) {
                    case DemonEyeMovementComponent::HOVERING: {
                        // 盘旋状态：更激进地追踪玩家
                        if (demon.aiTimer >= 0.0f) {
                            cocos2d::Vec2 desiredDir = toTarget;
                            if (!desiredDir.isZero()) desiredDir.normalize();
                            demon.targetAngle = atan2(desiredDir.y, desiredDir.x);
                            
                            // 检查是否满足冲刺条件
                            bool isAbove = transform.position.y > targetPos.y + 30.0f;
                            bool horizontalInRange = abs(toTarget.x) < 250.0f;
                            bool verticalInRange = toTarget.y < -30.0f && toTarget.y > -300.0f;
                            
                            if (isAbove && horizontalInRange && verticalInRange && demon.dashCooldownTimer <= 0.0f) {
                                demon.aiTimer += delta;
                                if (demon.aiTimer > 0.6f) {
                                    demon.aiState = DemonEyeMovementComponent::DASHING;
                                    demon.aiTimer = 0.0f;
                                    // 更新动画状态为冲刺
                                    if (animState) {
                                        AnimationStateHelper::updateAnimationState(*animState,
                                            AnimationStateHelper::demonEyeStateToAnimationState(demon.aiState));
                                    }
                                    auto targetEntity = static_cast<entt::entity>(aggro.targetEntity);
                                    auto* targetTransform = _registry->try_get<TransformComponent>(targetEntity);
                                    if (targetTransform) {
                                        cocos2d::Vec2 predictedPos = targetPos + aggro.directionToTarget * 100.0f;
                                        cocos2d::Vec2 dashDirection = (predictedPos - transform.position).getNormalized();
                                        demon.targetAngle = atan2(dashDirection.y, dashDirection.x);
                                        demon.currentAngle = demon.targetAngle;
                                    }
                                }
                            } else {
                                demon.aiTimer = 0.0f;
                            }
                        } else {
                            demon.aiTimer += delta;
                        }
                        
                        // 动态调整转弯速率
                        float originalTurnRate = demon.turnRate;
                        bool isPreparingDash = (demon.aiTimer > 0.0f && demon.aiTimer < 0.6f);
                        bool closeToTarget = hasTarget && aggro.distanceToTarget < 400.0f;
                        
                        if (isPreparingDash && closeToTarget) {
                            demon.turnRate *= 3.0f;
                        } else if (closeToTarget) {
                            demon.turnRate *= 2.0f;
                        }
                        
                        demon.smoothTurn(delta);
                        demon.turnRate = originalTurnRate;
                        break;
                    }
                    
                    case DemonEyeMovementComponent::DASHING: {
                        demon.aiTimer += delta;
                        
                        if (demon.aiTimer < 0.2f) {
                            // 加速阶段
                        } else if (demon.aiTimer < 1.2f) {
                            // 追踪阶段
                            float angleToPlayer = atan2(toTarget.y, toTarget.x);
                            float currentDiff = DemonEyeMovementComponent::angleDifference(angleToPlayer, demon.currentAngle);
                            float dashTurnRate = demon.turnRate * 1.0f;
                            
                            if (dist < 150.0f) {
                                dashTurnRate *= 2.0f;
                            }
                            
                            if (abs(currentDiff) < dashTurnRate * delta) {
                                demon.currentAngle = angleToPlayer;
                            } else {
                                demon.currentAngle += (currentDiff > 0 ? 1 : -1) * dashTurnRate * delta;
                            }
                        } else {
                            // 收尾阶段
                            float angleToPlayer = atan2(toTarget.y, toTarget.x);
                            float currentDiff = DemonEyeMovementComponent::angleDifference(angleToPlayer, demon.currentAngle);
                            float dashTurnRate = demon.turnRate * 0.3f;
                            if (abs(currentDiff) < dashTurnRate * delta) demon.currentAngle = angleToPlayer;
                            else demon.currentAngle += (currentDiff > 0 ? 1 : -1) * dashTurnRate * delta;
                        }
                        
                        bool passedPlayer = (transform.position.y < targetPos.y - 80.0f);
                        bool tooFar = dist > 500.0f;
                        bool tooClose = dist < 30.0f && demon.aiTimer > 0.3f;
                        
                        if (passedPlayer || demon.aiTimer > 2.5f || tooFar || tooClose) {
                            demon.aiState = DemonEyeMovementComponent::HOVERING;
                            demon.aiTimer = 0.0f;
                            demon.dashCooldownTimer = demon.dashCooldown;
                            // 更新动画状态回到盘旋
                            if (animState) {
                                AnimationStateHelper::updateAnimationState(*animState,
                                    AnimationStateHelper::demonEyeStateToAnimationState(demon.aiState));
                            }
                        }
                        break;
                    }
                }
            }
            
            // 更新摆动效果
            demon.wobblePhase += demon.wobbleFrequency * delta;
            float wobbleOffset = sin(demon.wobblePhase) * demon.wobbleAmplitude;
            float effectiveAngle = demon.currentAngle + wobbleOffset;
            
            auto* body = sprite->getPhysicsBody();
            if (!body) return;
            
            cocos2d::Vec2 physicsVelocity = body->getVelocity();
            demon.currentVelocity = physicsVelocity;
            float currentSpeedSq = demon.currentVelocity.lengthSquared();
            
            if (currentSpeedSq < 100.0f) {
                demon.currentVelocity.x = cos(demon.currentAngle) * demon.flySpeed;
                demon.currentVelocity.y = sin(demon.currentAngle) * demon.flySpeed;
                body->setVelocity(demon.currentVelocity);
                return;
            }
            
            float targetSpeed = demon.maxSpeed;
            float angularVelocity = std::abs(body->getAngularVelocity());
            if (angularVelocity > 0.1f) {
                float speedReduction = std::min(1.0f, (angularVelocity * angularVelocity) / 0.25f);
                targetSpeed *= (1.0f - speedReduction);
            }
            
            float angleDiff = std::abs(DemonEyeMovementComponent::angleDifference(
                demon.targetAngle, demon.currentAngle));
            if (angleDiff > 1.0f) targetSpeed *= 0.85f;
            
            cocos2d::Vec2 desiredVelocity;
            desiredVelocity.x = cos(effectiveAngle) * targetSpeed;
            desiredVelocity.y = sin(effectiveAngle) * targetSpeed;
            
            float lerpFactor = std::min(1.0f, demon.acceleration * delta / demon.flySpeed);
            
            cocos2d::Vec2 newVelocity;
            newVelocity.x = demon.currentVelocity.x + (desiredVelocity.x - demon.currentVelocity.x) * lerpFactor;
            newVelocity.y = demon.currentVelocity.y + (desiredVelocity.y - demon.currentVelocity.y) * lerpFactor;
            
            float newSpeed = newVelocity.length();
            if (newSpeed > demon.maxSpeed * 1.5f) {
                newVelocity.normalize();
                newVelocity *= demon.maxSpeed * 1.5f;
            }
            
            if (newSpeed < 50.0f && newSpeed > 0.1f) {
                newVelocity.normalize();
                newVelocity *= 50.0f;
            }
            
            body->setVelocity(newVelocity);
            transform.position = sprite->getPosition();
            demon.currentVelocity = body->getVelocity();
            
            // 更新旋转和朝向（根据速度方向）
            if (demon.currentVelocity.lengthSquared() > 100.0f) {
                float velocityAngle = atan2(demon.currentVelocity.y, demon.currentVelocity.x);
                float angleDiff = std::abs(DemonEyeMovementComponent::angleDifference(velocityAngle, demon.currentAngle));
                if (angleDiff > 1.57f) {
                    demon.currentAngle = velocityAngle;
                    demon.targetAngle = velocityAngle;
                    
                    if (demon.aiState == DemonEyeMovementComponent::DASHING) {
                        demon.aiState = DemonEyeMovementComponent::HOVERING;
                        demon.dashCooldownTimer = demon.dashCooldown;
                        // 更新动画状态
                        if (animState) {
                            AnimationStateHelper::updateAnimationState(*animState,
                                AnimationStateHelper::demonEyeStateToAnimationState(demon.aiState));
                        }
                    }
                    
                    demon.aiTimer = -0.4f;
                }
                
                // 设置旋转角度（让恶魔眼根据飞行方向旋转）
                float displayAngle = velocityAngle * 180.0f / M_PI;
                render.rotation = -displayAngle + 180.0f;
                
                // 设置垂直翻转（向左飞时翻转）
                bool movingLeft = demon.currentVelocity.x < 0;
                render.flipY = movingLeft;
            }
        });
    }

private:
    void updatePatrol(DemonEyeMovementComponent& demon, float delta) {
        demon.patrolTimer += delta;
        
        if (demon.patrolTimer >= demon.patrolChangeInterval) {
            demon.patrolTimer = 0.0f;
            demon.patrolAngle = ((float)rand() / RAND_MAX) * 2 * M_PI;
            CCLOG("DemonEye: New patrol angle: %.2f", demon.patrolAngle);
        }
        
        demon.targetAngle = demon.patrolAngle;
    }
};

} // namespace ecs

#endif // __ECS_SYSTEM_DEMONEYEAISYSTEMENTT_H__
