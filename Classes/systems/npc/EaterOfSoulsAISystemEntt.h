#ifndef __ECS_SYSTEM_EATEROFSOULSAISYSTEMENTT_H__
#define __ECS_SYSTEM_EATEROFSOULSAISYSTEMENTT_H__

#include "systems/npc/OptimizedAISystemBase.h"
#include "systems/core/SystemPriority.h"
#include "components/AllComponents.h"
#include "cocos2d.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace ecs {

/**
 * @brief 噬魂怪AI系统 - 飞行敌怪AI
 * 
 * 行为特点：
 * - 先在一定距离外绕着玩家转圈
 * - 周期性冲向玩家
 * - 飞行（无重力）
 * 
 * 优化特性（继承自 OptimizedAISystemBase）：
 * - 离屏实体降频更新
 * - 空闲实体降频更新
 * - 远距离实体使用简化 AI
 * 
 * Requirements: 5.1, 5.5, 5.6
 */
class EaterOfSoulsAISystemEntt : public OptimizedAISystemBase {
public:
    const char* getName() const override { return "EaterOfSoulsAISystem"; }
    int getPriority() const override { return SystemPriority::MOVEMENT; }

    void update(float delta) override {
        // 增加帧计数器
        incrementFrameCounter();
        
        auto view = _registry->view<EaterOfSoulsMovementComponent, AggroComponent,
                                     SpriteStateComponent, RenderComponent, TransformComponent>();
        
        view.each([delta, this](entt::entity entity, EaterOfSoulsMovementComponent& eater,
                               AggroComponent& aggro, SpriteStateComponent& state,
                               RenderComponent& render, TransformComponent& transform) {
            
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
            if (eater.chargeCooldown > 0.0f) {
                eater.chargeCooldown -= delta;
            }
            
            if (!hasTarget) {
                // 无目标：随机飞行巡逻
                if (eater.aiState != EaterOfSoulsMovementComponent::CIRCLING) {
                    eater.aiState = EaterOfSoulsMovementComponent::CIRCLING;
                }
                updatePatrol(eater, delta);
                eater.smoothTurn(delta);
            } else {
                // 有目标：根据状态执行AI
                cocos2d::Vec2 toTarget = targetPos - transform.position;
                float dist = toTarget.length();
                
                switch (eater.aiState) {
                    case EaterOfSoulsMovementComponent::CIRCLING: {
                        // 绕圈状态：围绕玩家转圈
                        updateCircling(eater, transform, targetPos, delta);
                        
                        // 检查是否应该冲刺
                        if (eater.chargeCooldown <= 0.0f && dist > 100.0f && dist < 400.0f) {
                            eater.aiState = EaterOfSoulsMovementComponent::CHARGING;
                            eater.aiTimer = 0.0f;
                            eater.chargeCooldown = eater.chargeInterval;
                            
                            // 预测玩家位置并设置冲刺方向
                            cocos2d::Vec2 chargeDirection = toTarget.getNormalized();
                            eater.targetAngle = atan2(chargeDirection.y, chargeDirection.x);
                            eater.currentAngle = eater.targetAngle;
                            
                            CCLOG("EaterOfSouls: Starting charge at distance %.1f", dist);
                        }
                        break;
                    }
                    
                    case EaterOfSoulsMovementComponent::CHARGING: {
                        // 冲刺状态：直线冲向玩家
                        eater.aiTimer += delta;
                        
                        if (eater.aiTimer < eater.chargeDuration) {
                            // 冲刺期间持续追踪玩家
                            cocos2d::Vec2 chargeDirection = toTarget.getNormalized();
                            float angleToPlayer = atan2(chargeDirection.y, chargeDirection.x);
                            
                            // 在冲刺过程中允许一定的转向
                            float currentDiff = EaterOfSoulsMovementComponent::angleDifference(angleToPlayer, eater.currentAngle);
                            float chargeTurnRate = eater.turnRate * 0.8f; // 冲刺时转向稍慢
                            
                            if (std::abs(currentDiff) < chargeTurnRate * delta) {
                                eater.currentAngle = angleToPlayer;
                            } else {
                                eater.currentAngle += (currentDiff > 0 ? 1 : -1) * chargeTurnRate * delta;
                            }
                            
                            eater.targetAngle = eater.currentAngle;
                        }
                        
                        // 检查是否应该结束冲刺
                        bool tooClose = dist < 50.0f;
                        bool tooFar = dist > 600.0f;
                        bool timeUp = eater.aiTimer >= eater.chargeDuration;
                        
                        if (tooClose || tooFar || timeUp) {
                            eater.aiState = EaterOfSoulsMovementComponent::CIRCLING;
                            eater.aiTimer = 0.0f;
                            // 重置绕圈角度到当前位置
                            cocos2d::Vec2 offset = transform.position - targetPos;
                            eater.circleAngle = atan2(offset.y, offset.x);
                            
                            CCLOG("EaterOfSouls: Ending charge (close:%d far:%d time:%d)", tooClose, tooFar, timeUp);
                        }
                        break;
                    }
                }
            }
            
            // 更新摆动效果
            eater.wobblePhase += eater.wobbleFrequency * delta;
            float wobbleOffset = sin(eater.wobblePhase) * eater.wobbleAmplitude;
            float effectiveAngle = eater.currentAngle + wobbleOffset;
            
            auto* body = sprite->getPhysicsBody();
            if (!body) return;
            
            cocos2d::Vec2 physicsVelocity = body->getVelocity();
            eater.currentVelocity = physicsVelocity;
            float currentSpeedSq = eater.currentVelocity.lengthSquared();
            
            // 如果速度太低，直接设置速度
            if (currentSpeedSq < 100.0f) {
                eater.currentVelocity.x = cos(eater.currentAngle) * eater.flySpeed;
                eater.currentVelocity.y = sin(eater.currentAngle) * eater.flySpeed;
                body->setVelocity(eater.currentVelocity);
                return;
            }
            
            // 根据状态设置目标速度
            float targetSpeed = eater.flySpeed;
            if (eater.aiState == EaterOfSoulsMovementComponent::CHARGING) {
                targetSpeed = eater.chargeSpeed;
            } else {
                targetSpeed = eater.maxSpeed;
            }
            
            // 计算期望速度
            cocos2d::Vec2 desiredVelocity;
            desiredVelocity.x = cos(effectiveAngle) * targetSpeed;
            desiredVelocity.y = sin(effectiveAngle) * targetSpeed;
            
            // 平滑插值到期望速度
            float lerpFactor = std::min(1.0f, eater.acceleration * delta / eater.flySpeed);
            
            cocos2d::Vec2 newVelocity;
            newVelocity.x = eater.currentVelocity.x + (desiredVelocity.x - eater.currentVelocity.x) * lerpFactor;
            newVelocity.y = eater.currentVelocity.y + (desiredVelocity.y - eater.currentVelocity.y) * lerpFactor;
            
            // 限制最大速度
            float newSpeed = newVelocity.length();
            float maxSpeedLimit = (eater.aiState == EaterOfSoulsMovementComponent::CHARGING) 
                                  ? eater.chargeSpeed * 1.2f 
                                  : eater.maxSpeed * 1.2f;
            
            if (newSpeed > maxSpeedLimit) {
                newVelocity.normalize();
                newVelocity *= maxSpeedLimit;
            }
            
            // 确保最小速度
            if (newSpeed < 50.0f && newSpeed > 0.1f) {
                newVelocity.normalize();
                newVelocity *= 50.0f;
            }
            
            body->setVelocity(newVelocity);
            transform.position = sprite->getPosition();
            eater.currentVelocity = body->getVelocity();
            
            // 更新朝向：贴图左边是头部，始终朝向玩家
            if (hasTarget) {
                cocos2d::Vec2 toTarget = targetPos - transform.position;
                if (toTarget.lengthSquared() > 1.0f) {
                    // 计算从噬魂怪到玩家的角度
                    float angleToPlayer = atan2(toTarget.y, toTarget.x);
                    // 转换为度数
                    float angleDegrees = angleToPlayer * 180.0f / M_PI;
                    
                    // 反转旋转方向：玩家顺时针移动时，噬魂怪也应该顺时针旋转
                    // 使用负号来反转旋转方向
                    render.rotation = -angleDegrees - 90.0f;
                    render.flipY = false;
                    render.flipX = false;
                }
            } else if (eater.currentVelocity.lengthSquared() > 100.0f) {
                // 无目标时根据速度方向朝向
                float velocityAngle = atan2(eater.currentVelocity.y, eater.currentVelocity.x);
                float angleDegrees = velocityAngle * 180.0f / M_PI;
                render.rotation = angleDegrees;
                render.flipY = false;
                render.flipX = false;
            }
        });
    }

private:
    /**
     * @brief 更新绕圈行为
     */
    void updateCircling(EaterOfSoulsMovementComponent& eater, 
                       TransformComponent& transform,
                       const cocos2d::Vec2& targetPos, 
                       float delta) {
        // 更新绕圈角度
        eater.circleAngle += eater.circleSpeed * delta;
        
        // 标准化角度
        while (eater.circleAngle > M_PI)
            eater.circleAngle -= 2 * M_PI;
        while (eater.circleAngle < -M_PI)
            eater.circleAngle += 2 * M_PI;
        
        // 添加半径变化（使绕圈更自然）
        float radiusVariation = sin(eater.circleAngle * 2.0f) * eater.circleRadiusVariation;
        float effectiveRadius = eater.circleRadius + radiusVariation;
        
        // 计算目标绕圈位置
        cocos2d::Vec2 circleOffset;
        circleOffset.x = cos(eater.circleAngle) * effectiveRadius;
        circleOffset.y = sin(eater.circleAngle) * effectiveRadius;
        
        cocos2d::Vec2 targetCirclePos = targetPos + circleOffset;
        
        // 计算朝向目标绕圈位置的方向
        cocos2d::Vec2 toCirclePos = targetCirclePos - transform.position;
        if (!toCirclePos.isZero()) {
            toCirclePos.normalize();
            eater.targetAngle = atan2(toCirclePos.y, toCirclePos.x);
        }
        
        // 平滑转向
        eater.smoothTurn(delta);
    }
    
    /**
     * @brief 更新巡逻行为（无目标时）
     */
    void updatePatrol(EaterOfSoulsMovementComponent& eater, float delta) {
        eater.aiTimer += delta;
        
        if (eater.aiTimer >= 2.0f) {
            eater.aiTimer = 0.0f;
            eater.targetAngle = ((float)rand() / RAND_MAX) * 2 * M_PI;
        }
    }
};

} // namespace ecs

#endif // __ECS_SYSTEM_EATEROFSOULSAISYSTEMENTT_H__
