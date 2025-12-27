#ifndef __ECS_SYSTEM_ANTLIONAISYSTEMENTT_H__
#define __ECS_SYSTEM_ANTLIONAISYSTEMENTT_H__

#include "systems/npc/OptimizedAISystemBase.h"
#include "systems/core/SystemPriority.h"
#include "components/AllComponents.h"
#include "systems/core/AnimationStateHelper.h"
#include "cocos2d.h"
#include <cmath>
#include <map>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace ecs {

/**
 * @brief 蚁狮AI系统 - 处理蚁狮的静止射击行为
 * 
 * 蚁狮行为特点：
 * - 完全静止不移动
 * - 头部朝向45度角范围内的玩家
 * - 每5秒向玩家发射沙球射弹
 * - 只能向上45度角范围内射击
 * 
 * 优化特性（继承自 OptimizedAISystemBase）：
 * - 离屏实体降频更新
 * - 空闲实体降频更新
 * - 远距离实体使用简化 AI
 * 
 * Requirements: 5.1, 5.5, 5.6
 */
class AntlionAISystemEntt : public OptimizedAISystemBase {
public:
    const char* getName() const override { return "AntlionAISystem"; }
    int getPriority() const override { return SystemPriority::AI; }

    void update(float delta) override {
        if (!_registry) return;
        
        // 增加帧计数器
        incrementFrameCounter();

        auto view = _registry->view<AntlionMovementComponent, TransformComponent, AggroComponent, RenderComponent>();

        view.each([this, delta](auto entity, 
                               AntlionMovementComponent& antlion,
                               TransformComponent& transform,
                               AggroComponent& aggro,
                               RenderComponent& render) {
            
            // 优化：检查是否应该更新此实体
            auto* stateFlags = _registry->try_get<EntityStateFlags>(entity);
            if (!shouldUpdateEntity(entity, stateFlags)) {
                return; // 跳过此帧的更新
            }
            
            // 获取动画状态组件（如果存在）
            auto* animState = _registry->try_get<AnimationStateComponent>(entity);
            
            // 蚁狮通过PhysicsBodyComponent配置保持静止
            // 物理体的velocityLimit和高阻尼确保其不会移动
            
            // 更新计时器
            antlion.aiTimer += delta;
            antlion.shootCooldown -= delta;
            if (antlion.shootCooldown < 0) antlion.shootCooldown = 0;

            // 检查是否有仇恨目标
            bool hasValidTarget = false;
            cocos2d::Vec2 targetPos = cocos2d::Vec2::ZERO;
            
            if (aggro.hasAggro && aggro.targetEntity != INVALID_ENTITY) {
                auto targetEntityHandle = static_cast<entt::entity>(aggro.targetEntity);
                if (_registry->valid(targetEntityHandle)) {
                    auto* targetTransform = _registry->try_get<TransformComponent>(targetEntityHandle);
                    if (targetTransform) {
                        targetPos = targetTransform->position;
                        float distance = transform.position.distance(targetPos);
                        
                        if (distance <= antlion.detectionRange) {
                            // 只要在仇恨范围内就锁定目标，360度全方位跟踪
                            cocos2d::Vec2 toTarget = targetPos - transform.position;
                            float angleToTarget = atan2(toTarget.y, toTarget.x);
                            float angleDegrees = angleToTarget * 180.0f / M_PI;
                            
                            // 标准化角度到[0, 360]
                            while (angleDegrees < 0) angleDegrees += 360.0f;
                            while (angleDegrees >= 360.0f) angleDegrees -= 360.0f;
                            
                            
                            // 在仇恨范围内就锁定目标（360度全方位）
                            hasValidTarget = true;
                            antlion.targetPosition = targetPos;
                            antlion.hasTarget = true;
                        }
                    }
                }
            }

            if (!hasValidTarget) {
                antlion.hasTarget = false;
                if (antlion.aiState != AntlionMovementComponent::IDLE) {
                    antlion.aiState = AntlionMovementComponent::IDLE;
                    antlion.aiTimer = 0.0f;
                    // 更新动画状态到待机
                    if (animState) {
                        AnimationStateHelper::updateAnimationState(*animState,
                            AnimationStateHelper::antlionStateToAnimationState(antlion.aiState));
                    }
                }
            }

            // 状态机处理
            switch (antlion.aiState) {
                case AntlionMovementComponent::IDLE:
                    updateIdle(antlion, transform, render, delta);
                    break;
                case AntlionMovementComponent::TRACKING:
                    updateTracking(antlion, transform, render, delta);
                    break;
                case AntlionMovementComponent::SHOOTING:
                    updateShooting(entity, antlion, transform, render, delta);
                    break;
            }

            // 状态转换
            if (hasValidTarget) {
                if (antlion.aiState == AntlionMovementComponent::IDLE) {
                    antlion.aiState = AntlionMovementComponent::TRACKING;
                    antlion.aiTimer = 0.0f;
                    // 更新动画状态到追踪
                    if (animState) {
                        AnimationStateHelper::updateAnimationState(*animState,
                            AnimationStateHelper::antlionStateToAnimationState(antlion.aiState));
                    }
                }
                
                // 检查是否可以射击
                
                if (antlion.aiState == AntlionMovementComponent::TRACKING && 
                    antlion.shootCooldown <= 0 &&
                    antlion.aiTimer >= 0.5f) { // 追踪0.5秒后就可以射击
                    
                    antlion.aiState = AntlionMovementComponent::SHOOTING;
                    antlion.aiTimer = 0.0f;
                    antlion.shootCooldown = antlion.shootInterval;
                    
                    // 更新动画状态为射击（强制切换）
                    if (animState) {
                        AnimationStateHelper::updateAnimationState(*animState,
                            AnimationStateHelper::antlionStateToAnimationState(antlion.aiState), true);
                    }
                    
                    // 创建沙球射弹
                    createSandBallProjectile(entity, transform.position, antlion.targetPosition, antlion);
                }
            }
        });
    }

private:
    /**
     * @brief 更新待机状态
     */
    void updateIdle(AntlionMovementComponent& antlion, TransformComponent& transform, 
                   RenderComponent& render, float delta) {
        // 头部缓慢回到中性位置（朝上90度）
        float targetAngle = M_PI / 2.0f; // 90度，朝上
        float angleDiff = targetAngle - antlion.headRotation;
        
        // 标准化角度差
        while (angleDiff > M_PI) angleDiff -= 2 * M_PI;
        while (angleDiff < -M_PI) angleDiff += 2 * M_PI;
        
        // 平滑回到中性位置
        float maxRotation = antlion.rotationSpeed * delta;
        if (abs(angleDiff) > maxRotation) {
            angleDiff = (angleDiff > 0) ? maxRotation : -maxRotation;
        }
        
        antlion.headRotation += angleDiff;
        // 修复：反转旋转方向以匹配Cocos2D坐标系  
        render.rotation = -(antlion.headRotation * 180.0f / M_PI - 90.0f); // 减去90度因为sprite默认朝右
    }

    /**
     * @brief 更新追踪状态
     */
    void updateTracking(AntlionMovementComponent& antlion, TransformComponent& transform,
                       RenderComponent& render, float delta) {
        if (!antlion.hasTarget) return;

        // 计算到目标的向量（360度全方位跟踪）
        cocos2d::Vec2 toTarget = antlion.targetPosition - transform.position;
        float targetAngle = atan2(toTarget.y, toTarget.x);
        float targetAngleDegrees = targetAngle * 180.0f / M_PI;
        
        // 标准化到[0, 360]
        while (targetAngleDegrees < 0) targetAngleDegrees += 360.0f;
        while (targetAngleDegrees >= 360.0f) targetAngleDegrees -= 360.0f;
        
        
        // 限制头部摆动角度在45-135度范围内（但仍然360度跟踪）
        float clampedAngleDegrees = targetAngleDegrees;
        if (targetAngleDegrees < 45.0f || targetAngleDegrees > 315.0f) {
            // 右侧区域（0-45度和315-360度），锁定在45度
            clampedAngleDegrees = 45.0f;
        } else if (targetAngleDegrees > 135.0f && targetAngleDegrees < 315.0f) {
            // 左侧区域（135-315度），锁定在135度
            clampedAngleDegrees = 135.0f;
        }
        // 否则在45-135度范围内，直接使用目标角度
        
        
        float clampedAngle = clampedAngleDegrees * M_PI / 180.0f;
        
        // 平滑转向
        float angleDiff = clampedAngle - antlion.headRotation;
        
        // 标准化角度差到[-π, π]
        while (angleDiff > M_PI) angleDiff -= 2 * M_PI;
        while (angleDiff < -M_PI) angleDiff += 2 * M_PI;
        
        
        // 应用旋转速度限制
        float maxRotation = antlion.rotationSpeed * delta;
        if (abs(angleDiff) > maxRotation) {
            angleDiff = (angleDiff > 0) ? maxRotation : -maxRotation;
        }
        
        antlion.headRotation += angleDiff;
        
        // 将头部旋转应用到渲染组件（减去90度因为sprite默认朝右，我们要朝上）
        // 修复：反转旋转方向以匹配Cocos2D坐标系
        float renderRotation = -(antlion.headRotation * 180.0f / M_PI - 90.0f);
        render.rotation = renderRotation;
        
    }

    /**
     * @brief 更新射击状态
     */
    void updateShooting(entt::entity entity, AntlionMovementComponent& antlion, TransformComponent& transform,
                       RenderComponent& render, float delta) {
        // 射击状态持续时间（0.6秒 = 4帧 × 0.15秒/帧）
        const float shootStateDuration = 0.6f;
        
        if (antlion.aiTimer >= shootStateDuration) {
            // 射击状态完成，强制回到追踪状态或待机状态
            AntlionMovementComponent::AIState newState;
            if (antlion.hasTarget) {
                newState = AntlionMovementComponent::TRACKING;
            } else {
                newState = AntlionMovementComponent::IDLE;
            }
            
            
            antlion.aiState = newState;
            antlion.aiTimer = 0.0f;
            
            // 获取动画状态组件并强制切换动画
            auto* animState = _registry->try_get<AnimationStateComponent>(entity);
            if (animState) {
                // 强制切换动画，确保从射击动画跳回循环动画
                AnimationStateHelper::updateAnimationState(*animState,
                    AnimationStateHelper::antlionStateToAnimationState(newState), true);
                
                // 额外确保：重置AnimationComponent的播放状态
                auto* anim = _registry->try_get<AnimationComponent>(entity);
                if (anim) {
                    anim->reset();
                    anim->isPlaying = true;
                }
            }
        }
    }

    /**
     * @brief 开始射击状态
     */
    void startShootingAnimation(RenderComponent& render) {
    }

    /**
     * @brief 计算45度射击所需的速度
     * @param startPos 发射位置
     * @param targetPos 目标位置
     * @param angleDegrees 发射角度（度数）
     * @param gravity 重力加速度
     * @return 所需初始速度，如果无解返回-1
     */
    float calculateRequiredVelocityFor45DegreeShot(const cocos2d::Vec2& startPos, const cocos2d::Vec2& targetPos, 
                                                   float angleDegrees, float gravity) {
        cocos2d::Vec2 displacement = targetPos - startPos;
        float x = displacement.x;
        float y = displacement.y;
        float angleRad = angleDegrees * M_PI / 180.0f;
        
        
        // 对于左侧目标(135°)，需要特殊处理坐标系
        float effectiveX, effectiveAngle;
        if (angleDegrees > 90.0f) {
            // 左侧射击：将135°转换为相对坐标系下的45°
            effectiveX = -x;  // 反转水平距离
            effectiveAngle = 180.0f - angleDegrees;  // 135° -> 45°
        } else {
            // 右侧射击：直接使用
            effectiveX = x;
            effectiveAngle = angleDegrees;
        }
        
        float effectiveAngleRad = effectiveAngle * M_PI / 180.0f;
        float cosAngle = cos(effectiveAngleRad);
        float tanAngle = tan(effectiveAngleRad);
        
        
        // 使用有效坐标计算弹道
        // 抛物线公式：y = x*tan(θ) - (gx²)/(2v²cos²(θ))
        // 求解v² = (gx²)/(2cos²(θ)(x*tan(θ) - y))
        float denominator = 2 * cosAngle * cosAngle * (effectiveX * tanAngle - y);
        
        
        if (denominator <= 0) {
            return -1.0f;
        }
        
        float vSquared = (gravity * effectiveX * effectiveX) / denominator;
        
        if (vSquared <= 0) {
            return -1.0f;
        }
        
        float requiredVelocity = sqrt(vSquared);
        
        return requiredVelocity;
    }

    /**
     * @brief 创建沙球射弹
     */
    void createSandBallProjectile(entt::entity antlionEntity, const cocos2d::Vec2& startPos, const cocos2d::Vec2& targetPos,
                                 const AntlionMovementComponent& antlion) {
        if (!_registry) return;

        // 计算目标位置和距离（考虑射弹发射位置偏移）
        cocos2d::Vec2 projectileStartPos = startPos + cocos2d::Vec2(0, 13); // 与实际发射位置一致
        cocos2d::Vec2 targetHeadPos = targetPos + cocos2d::Vec2(0, 25); // 目标玩家头部位置
        cocos2d::Vec2 direction = targetHeadPos - projectileStartPos;
        float distance = direction.length();
        
        
        // 智能射击算法：根据目标相对位置选择策略
        float shootAngle;
        cocos2d::Vec2 velocity;
        float directAngle = atan2(direction.y, direction.x);
        float directAngleDegrees = directAngle * 180.0f / M_PI;
        
        // 标准化角度到[0, 360]
        while (directAngleDegrees < 0) directAngleDegrees += 360.0f;
        while (directAngleDegrees >= 360.0f) directAngleDegrees -= 360.0f;
        
        // 精确定义“低位目标”：仅对水平线或以下的目标使用45度约束
        // 水平线以上（如121°）应该使用直接瞄准
        // 左侧低位：135°-225° (水平左侧到正下方到水平右侧)
        // 右侧低位：315°-360° 和 0°-45° (水平右侧到正下方到水平左侧)
        bool isTargetLowPosition = (directAngleDegrees >= 135.0f && directAngleDegrees <= 225.0f) ||  // 左侧低位
                                   (directAngleDegrees >= 315.0f || directAngleDegrees <= 45.0f);   // 右侧低位
        
        
        if (isTargetLowPosition) {
            // 目标在水平线或下方：强制使用45度角，通过调整速度命中
            bool isLeftTarget = direction.x < 0;
            
            // 左右都使用等效的45度角
            float displayAngle = isLeftTarget ? 135.0f : 45.0f;  // 用于显示和计算
            float effectiveAngle = 45.0f;  // 用于实际应用速度（左右都是45度）
            shootAngle = effectiveAngle * M_PI / 180.0f;
            
            // 使用抛物线公式计算所需速度（传入显示角度用于日志）
            float requiredSpeed = calculateRequiredVelocityFor45DegreeShot(projectileStartPos, targetHeadPos, 
                                                                         displayAngle, antlion.projectileGravity);
            
            // 计算最终速度
            float finalSpeed;
            if (requiredSpeed > 0 && requiredSpeed <= antlion.projectileSpeed * 2.5f) {
                finalSpeed = requiredSpeed;
            } else {
                // 所需速度过大，使用默认速度
                finalSpeed = antlion.projectileSpeed * 1.4f;
            }
            
            // 关键修复：左右都使用45度角应用速度，但左侧需要反向X速度
            if (isLeftTarget) {
                // 左侧：使用45度角但X速度反向
                velocity.x = -finalSpeed * cos(shootAngle);  // 反向X速度
                velocity.y = finalSpeed * sin(shootAngle);   // 正向Y速度
            } else {
                // 右侧：直接使用45度角
                velocity.x = finalSpeed * cos(shootAngle);
                velocity.y = finalSpeed * sin(shootAngle);
            }
        } else if (distance <= 400.0f) {
            // 近距离直接瞄准：使用直接角度，高速度确保命中
            shootAngle = directAngle;
            float adjustedSpeed = antlion.projectileSpeed * 1.1f;  // 提高速度确保威力
            velocity.x = adjustedSpeed * cos(shootAngle);
            velocity.y = adjustedSpeed * sin(shootAngle);
        } else {
            // 远距离抛物线射击：使用弹道计算，大幅提高速度
            shootAngle = antlion.calculateBallisticAngle(projectileStartPos, targetHeadPos, 
                                                        antlion.projectileSpeed * 2.0f,  // 远距离大幅提高速度
                                                        antlion.projectileGravity);
            
            if (shootAngle >= 0 && antlion.isAngleInShootRange(shootAngle)) {
                float longRangeSpeed = antlion.projectileSpeed * 2.0f;  // 远距离双倍速度
                velocity.x = longRangeSpeed * cos(shootAngle);
                velocity.y = longRangeSpeed * sin(shootAngle);
            } else {
                // 弹道计算失败，回退到预测瞄准，使用高速
                float predictedAngle = atan2(direction.y + 80.0f, direction.x);  // 更大向上偏移补偿重力
                shootAngle = predictedAngle;
                float fallbackSpeed = antlion.projectileSpeed * 1.8f;  // 高速回退模式
                velocity.x = fallbackSpeed * cos(shootAngle);
                velocity.y = fallbackSpeed * sin(shootAngle);
            }
        }
        
        // 最终角度检查
        if (!antlion.isAngleInShootRange(shootAngle)) {
            return;
        }

        // 获取蚁狮的父节点（用于射弹）
        cocos2d::Node* parentNode = nullptr;
        auto* antlionState = _registry->try_get<SpriteStateComponent>(antlionEntity);
        if (antlionState && antlionState->spriteCreated && antlionState->spriteHandle) {
            auto* antlionSprite = static_cast<cocos2d::Sprite*>(antlionState->spriteHandle);
            parentNode = antlionSprite->getParent();
        }
        
        if (!parentNode) {
            return;
        }
        
        // 创建射弹实体
        auto projectileEntity = _registry->create();
        
        // 位置组件
        auto& projectileTransform = _registry->emplace<TransformComponent>(projectileEntity);
        projectileTransform.position = startPos + cocos2d::Vec2(0, 13); // 从嘴部发射（降低位置）
        
        // 渲染组件
        auto& projectileRender = _registry->emplace<RenderComponent>(projectileEntity);
        projectileRender.spriteResourceId = "Sand_Ball";
        projectileRender.scale = 1.0f;
        projectileRender.zOrder = 2;  // 在怪物上层
        projectileRender.visible = true;
        
        
        // 父节点组件（关键：RenderSystem需要这个来创建sprite）
        auto& projParent = _registry->emplace<ParentNodeComponent>(projectileEntity);
        projParent.parentNode = parentNode;
        projParent.attachedToParent = false;
        
        // 射弹组件
        auto& projectile = _registry->emplace<ProjectileComponent>(projectileEntity);
        projectile.owner = entt::to_integral(antlionEntity);
        projectile.damage = 15.0f;
        projectile.lifetime = 5.0f;
        
        // 物理体组件（将通过RenderSystem创建物理体）
        auto& physicsBody = _registry->emplace<PhysicsBodyComponent>(projectileEntity);
        physicsBody.shape = PhysicsBodyComponent::BodyShape::Circle;
        physicsBody.radius = 7.0f;
        physicsBody.density = 0.1f;
        physicsBody.restitution = 0.3f;
        physicsBody.friction = 0.1f;
        physicsBody.dynamic = true;
        physicsBody.rotationEnabled = true;
        physicsBody.gravityEnabled = true;
        physicsBody.velocityLimit = 1000.0f;
        physicsBody.categoryBitmask = 0x0008;
        physicsBody.collisionBitmask = 0x0001;
        physicsBody.contactTestBitmask = 0x0001;  // 修复：必须匹配地面的categoryBitmask
        physicsBody.group = -2;
        
        // 初始速度组件（RenderSystem会在创建物理体后应用）
        auto& initialVel = _registry->emplace<InitialVelocityComponent>(projectileEntity);
        initialVel.velocity = velocity;
        initialVel.applied = false;
        
    }
};

} // namespace ecs

#endif // __ECS_SYSTEM_ANTLIONAISYSTEMENTT_H__
