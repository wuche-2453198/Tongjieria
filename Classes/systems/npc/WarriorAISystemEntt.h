#ifndef __ECS_SYSTEM_WARRIORAISYSTEMENTT_H__
#define __ECS_SYSTEM_WARRIORAISYSTEMENTT_H__

#include "systems/npc/OptimizedAISystemBase.h"
#include "systems/core/SystemPriority.h"
#include "components/AllComponents.h"
#include "cocos2d.h"
#include <cmath>

namespace ecs {

/**
 * @brief 战士AI系统 - 管理僵尸等行走类怪物的移动和AI逻辑
 * 
 * 行为特点：
 * - 行走追踪玩家
 * - 跳过洞和障碍物
 * - 尝试垂直对齐目标高度
 * - 追击失败时后退重试
 * 
 * 优化特性（继承自 OptimizedAISystemBase）：
 * - 离屏实体降频更新
 * - 空闲实体降频更新
 * - 远距离实体使用简化 AI
 * 
 * Requirements: 5.1, 5.5, 5.6
 */
class WarriorAISystemEntt : public OptimizedAISystemBase {
public:
    const char* getName() const override { return "WarriorAISystem"; }
    int getPriority() const override { return SystemPriority::MOVEMENT; }

    void update(float delta) override {
        // 增加帧计数器
        incrementFrameCounter();
        
        auto view = _registry->view<WarriorMovementComponent, GroundDetectorComponent,
                                     AggroComponent, SpriteStateComponent, RenderComponent, TransformComponent>();
        
        view.each([delta, this](auto entity, WarriorMovementComponent& warrior,
                               GroundDetectorComponent& ground, AggroComponent& aggro,
                               SpriteStateComponent& state, RenderComponent& render, TransformComponent& transform) {
            if (!state.spriteCreated || !state.spriteHandle) return;
            
            // 优化：检查是否应该更新此实体
            auto* stateFlags = _registry->try_get<EntityStateFlags>(entity);
            if (!shouldUpdateEntity(entity, stateFlags)) {
                return; // 跳过此帧的更新
            }
            
            auto* sprite = static_cast<cocos2d::Sprite*>(state.spriteHandle);
            
            // 检测落地并判断跳跃是否成功
            if (ground.isOnGround && warrior.isJumping) {
                warrior.onLand();
                
                // 检测跳跃是否失败（水平位移很小）
                if (warrior.wasJumping) {
                    if (warrior.checkJumpFailed(transform.position)) {
                        warrior.onJumpFailed();
                        CCLOG("Entity %u: Jump failed (fail count: %d)", 
                              entt::to_integral(entity), warrior.jumpFailCount);
                    } else {
                        warrior.onJumpSuccess();
                    }
                }
            }
            warrior.wasJumping = warrior.isJumping;
            
            // 更新跳跃冷却
            if (ground.isOnGround) {
                warrior.updateJumpCooldown(delta);
            }
            
            // 更新障碍物跳跃冷却
            if (ground.isOnGround && warrior.obstacleJumpTimer > 0) {
                warrior.obstacleJumpTimer -= delta;
            }
            
            // 更新后退状态
            warrior.updateRetreat(delta);
            
            auto* body = sprite->getPhysicsBody();
            if (!body) return;
            cocos2d::Vec2 currentVelocity = body->getVelocity();
            bool shouldJump = false;
            float targetHeightDiff = 0.0f;
            float horizontalDistToTarget = 9999.0f;
            float totalDistToTarget = 9999.0f;
            
            // 计算与目标的距离
            if (aggro.hasAggro && aggro.targetEntity != INVALID_ENTITY) {
                auto targetEntity = static_cast<entt::entity>(aggro.targetEntity);
                auto* targetTransform = _registry->try_get<TransformComponent>(targetEntity);
                if (targetTransform) {
                    horizontalDistToTarget = std::abs(targetTransform->position.x - transform.position.x);
                    float verticalDist = std::abs(targetTransform->position.y - transform.position.y);
                    totalDistToTarget = std::sqrt(horizontalDistToTarget * horizontalDistToTarget + 
                                                  verticalDist * verticalDist);
                }
            }
            
            // 更新反应跳跃计时器
            if (warrior.pendingReactionJump) {
                warrior.targetJumpReactionTimer += delta;
                if (warrior.targetJumpReactionTimer >= warrior.targetJumpReactionTime) {
                    warrior.pendingReactionJump = false;
                    warrior.targetJumpReactionTimer = 0.0f;
                    if (ground.isOnGround && warrior.canJump() && 
                        horizontalDistToTarget < warrior.jumpDetectionRange) {
                        executeJump(warrior, sprite, ground, transform.position);
                    }
                }
            }
            
            // 确定移动方向和目标
            if (aggro.hasAggro && aggro.targetEntity != INVALID_ENTITY) {
                auto targetEntity = static_cast<entt::entity>(aggro.targetEntity);
                auto* targetTransform = _registry->try_get<TransformComponent>(targetEntity);
                if (targetTransform) {
                    
                    // 计算高度差
                    targetHeightDiff = targetTransform->position.y - transform.position.y;
                    
                    // ===== 战士AI：后退重试机制（只在连续跳跃失败时触发） =====
                    if (warrior.isRetreating) {
                        // 后退时反向移动
                        float dirX = targetTransform->position.x - transform.position.x;
                        warrior.currentDirection = dirX > 0 ? -1 : 1;  // 反向
                    } else {
                        // 正常追击
                        float dirX = targetTransform->position.x - transform.position.x;
                        if (std::abs(dirX) > 5.0f) {
                            warrior.currentDirection = dirX > 0 ? 1 : -1;
                        }
                        
                        // 检测是否需要后退（连续多次跳跃失败）
                        if (warrior.shouldRetreat() && ground.isOnGround) {
                            warrior.startRetreat();
                            CCLOG("Entity %u: Multiple jump failures, retreating to retry", entt::to_integral(entity));
                        }
                    }
                    
                    // 检测目标是否刚跳起来（跟随跳跃）
                    if (warrior.targetJumpEnabled && !warrior.pendingReactionJump &&
                        horizontalDistToTarget < warrior.jumpDetectionRange && !warrior.isRetreating) {
                        float targetCurrentY = targetTransform->position.y;
                        float targetYDelta = targetCurrentY - warrior.targetLastY;
                        
                        if (targetYDelta > 20.0f && warrior.targetLastY > 0) {
                            warrior.pendingReactionJump = true;
                            warrior.targetJumpReactionTimer = 0.0f;
                        }
                    }
                    warrior.targetLastY = targetTransform->position.y;
                    
                    // ===== 战士AI：垂直对齐 - 目标在高处时跳跃 =====
                    if (warrior.verticalAlignEnabled && !warrior.isRetreating &&
                        targetHeightDiff > warrior.verticalAlignThreshold && 
                        ground.isOnGround && warrior.canJump() &&
                        horizontalDistToTarget < warrior.jumpDetectionRange) {
                        float dirX = targetTransform->position.x - transform.position.x;
                        warrior.currentDirection = dirX > 0 ? 1 : -1;
                        shouldJump = true;
                    }
                }
            } else {
                // 无仇恨目标：巡逻模式（只在遇到障碍物跳跃失败时换向）
                warrior.currentDirection = warrior.patrolDirection;
                render.flipX = warrior.currentDirection < 0;
                
                // 巡逻时如果连续跳跃失败，换方向
                if (warrior.shouldRetreat() && ground.isOnGround) {
                    warrior.patrolDirection *= -1;
                    warrior.currentDirection = warrior.patrolDirection;
                    warrior.jumpFailCount = 0;
                    warrior.retreatCooldownTimer = warrior.retreatCooldown;
                    CCLOG("Entity %u: Patrol direction changed due to obstacle", entt::to_integral(entity));
                }
                
                warrior.targetLastY = 0.0f;
                warrior.pendingReactionJump = false;
            }
            
            // 检测障碍物卡住（也用于跳过洞）
            if (warrior.shouldObstacleJump(transform.position, delta) && ground.isOnGround) {
                shouldJump = true;
            }
            
            // ===== 战士AI：即时障碍物/洞检测 =====
            if (warrior.gapJumpEnabled && ground.isOnGround && 
                warrior.initialized && warrior.canJump() && !shouldJump && !warrior.isRetreating) {
                float actualVelX = std::abs(currentVelocity.x);
                float expectedVelX = warrior.walkSpeed;
                
                // 速度差检测（卡住或前方有洞）
                if (warrior.isWalking && expectedVelX > 10.0f && 
                    actualVelX < expectedVelX * warrior.actualSpeedRatio) {
                    shouldJump = true;
                }
            }
            
            // 执行跳跃
            if (shouldJump && ground.isOnGround && warrior.canJump()) {
                executeJump(warrior, sprite, ground, transform.position);
            }
            
            // 计算目标水平速度
            float moveSpeed = warrior.isRetreating ? warrior.walkSpeed * 0.6f : warrior.walkSpeed;
            float targetVelX = moveSpeed * warrior.currentDirection;
            
            // 应用水平移动和朝向
            if (ground.isOnGround) {
                // 地面：直接设置速度
                warrior.expectedSpeed = warrior.walkSpeed;
                currentVelocity.x = targetVelX;
                body->setVelocity(currentVelocity);
                warrior.isWalking = true;
            } else {
                cocos2d::Vec2 airVelocity = body->getVelocity();

                if (!warrior.isJumping && airVelocity.y <= 0.0f) {
                    airVelocity.x = 0.0f;
                    body->setVelocity(airVelocity);
                } else if (warrior.isJumping) {
                    float targetVelXAir = warrior.walkSpeed * warrior.currentDirection;
                    if (airVelocity.y > 0.0f && std::abs(airVelocity.x) < std::abs(targetVelXAir) * 0.5f) {
                        airVelocity.x = targetVelXAir;
                        body->setVelocity(airVelocity);
                    }
                }
            }
            
            // 更新朝向
            if (std::abs(targetVelX) > 1.0f) {
                bool movingRight = targetVelX > 0;
                render.flipX = movingRight;
            }
            
            // 更新跳跃状态
            warrior.isJumping = !ground.isOnGround;
        });
    }

private:
    void executeJump(WarriorMovementComponent& warrior, cocos2d::Sprite* sprite,
                    GroundDetectorComponent& ground, const cocos2d::Vec2& currentPos) {
        auto* body = sprite->getPhysicsBody();
        if (!body) return;
        cocos2d::Vec2 currentVel = body->getVelocity();
        
        // 记录跳跃前位置，用于后续判断跳跃是否成功
        warrior.onJumpStart(currentPos);
        
        // 如果当前水平速度很小（被卡住），使用AI期望的方向速度
        float jumpHorizontalVel = currentVel.x;
        if (std::abs(jumpHorizontalVel) < warrior.walkSpeed * 0.3f) {
            // 被卡住时，给予AI期望方向的水平速度
            jumpHorizontalVel = warrior.walkSpeed * warrior.currentDirection;
        }
        
        cocos2d::Vec2 jumpVel(jumpHorizontalVel, warrior.jumpForce);
        body->setVelocity(jumpVel);
        
        warrior.onJump();
        warrior.obstacleJumpTimer = warrior.obstacleJumpCooldown;
        warrior.stuckTime = 0.0f;
        ground.isOnGround = false;
        ground.groundContactCount = 0;
        warrior.isJumping = true;
    }
};

// 为了向后兼容，保留别名
using WalkMovementSystemEntt = WarriorAISystemEntt;

} // namespace ecs

#endif // __ECS_SYSTEM_WARRIORAISYSTEMENTT_H__
