#ifndef __ECS_SYSTEM_KINGSLIMEAISYSTEMENTT_H__
#define __ECS_SYSTEM_KINGSLIMEAISYSTEMENTT_H__

#include "systems/npc/OptimizedAISystemBase.h"
#include "systems/core/SystemPriority.h"
#include "components/AllComponents.h"
#include "core/factory/monster/MonsterMasterFactory.h"
#include "systems/core/AnimationStateHelper.h"
#include "cocos2d.h"
#include <cmath>
#include <unordered_map>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace ecs {

/**
 * @brief 史莱姆王AI系统 - 管理Boss的复杂行为模式
 * 
 * 特性：
 * - 3次小跳 + 1次大跳的跳跃模式
 * - 血量越低跳跃冷却越短
 * - 15秒传送机制（仅在地面时）
 * - 随血量掉落生成小史莱姆
 * - 体型随血量缩放
 * 
 * 优化特性（继承自 OptimizedAISystemBase）：
 * - 离屏实体降频更新
 * - 空闲实体降频更新
 * - 远距离实体使用简化 AI
 * 
 * 注意：Boss 通常不应该被优化跳过，因为它们是重要的游戏元素
 * 
 * Requirements: 5.1, 5.5, 5.6
 */
class KingSlimeAISystemEntt : public OptimizedAISystemBase {
public:
    const char* getName() const override { return "KingSlimeAISystem"; }
    int getPriority() const override { return SystemPriority::AI; }
    
    void setSceneContext(cocos2d::Node* scene) {
        _sceneContext = scene;
    }

private:
    cocos2d::Node* _sceneContext = nullptr;

    void update(float delta) override {
        // 增加帧计数器
        incrementFrameCounter();
        
        auto view = _registry->view<KingSlimeComponent, JumpMovementComponent, 
                                   HealthComponent, AggroComponent, 
                                   SpriteStateComponent, RenderComponent, TransformComponent, 
                                   GroundDetectorComponent>();
        
        view.each([delta, this](auto entity, KingSlimeComponent& kingSlime,
                               JumpMovementComponent& jump, HealthComponent& health,
                               AggroComponent& aggro, SpriteStateComponent& state,
                               RenderComponent& render, TransformComponent& transform, GroundDetectorComponent& ground) {
            
            if (!kingSlime.isActive || health.currentHealth <= 0) return;
            
            // 注意：Boss 不应用优化跳过，因为它们是重要的游戏元素
            // 但我们仍然可以使用状态标记进行其他优化
            
            // 获取动画状态组件（如果存在）
            auto* animState = _registry->try_get<AnimationStateComponent>(entity);
            
            float healthPercent = health.currentHealth / health.maxHealth;
            
            // 更新血量阶段和缩放（传送中不更新缩放，让动画控制）
            kingSlime.updateStage(healthPercent);
            if (!kingSlime.isTeleporting) {
                updateBossScaling(render, kingSlime);
            }
            
            // 更新跳跃冷却（血量越低跳得越快）
            float cooldownMultiplier = kingSlime.getJumpCooldownMultiplier(healthPercent);
            jump.jumpCooldown = 1.5f * cooldownMultiplier;
            
            // 传送系统更新
            updateTeleportSystem(entity, kingSlime, transform, aggro, ground, state, render, animState, delta);
            
            // 史莱姆生成系统
            updateSlimeSpawning(kingSlime, healthPercent, transform, state);
            
            // 跳跃AI - 基于现有跳跃系统扩展
            updateJumpPattern(entity, kingSlime, jump, aggro, state, healthPercent);
        });
    }

    void updateBossScaling(RenderComponent& render, KingSlimeComponent& kingSlime) {
        // 新架构：直接更新RenderComponent的scale
        render.scale = kingSlime.currentScale;
    }
    
    void updateTeleportSystem(entt::entity entity, KingSlimeComponent& kingSlime, TransformComponent& transform, 
                             AggroComponent& aggro, GroundDetectorComponent& ground, 
                             SpriteStateComponent& state, RenderComponent& render,
                             AnimationStateComponent* animState, float delta) {
        kingSlime.teleportTimer += delta;
        
        static float debugTimer = 0.0f;
        debugTimer += delta;
        if (debugTimer >= 5.0f) {
            CCLOG("KingSlime: Teleport status - Timer: %.1f/%.1f, HasAggro: %d, Target: %u, OnGround: %d", 
                  kingSlime.teleportTimer, kingSlime.teleportInterval, 
                  aggro.targetEntity != ecs::INVALID_ENTITY ? 1 : 0, aggro.targetEntity, ground.isOnGround ? 1 : 0);
            debugTimer = 0.0f;
        }
        
        if (kingSlime.teleportTimer >= kingSlime.teleportInterval) {
            if (aggro.targetEntity != ecs::INVALID_ENTITY && ground.isOnGround && !kingSlime.isTeleporting) {
                CCLOG("KingSlime: Teleport ready! Executing immediate teleport");
                
                performTeleport(entity, kingSlime, transform, aggro, state, render, animState);
                
                kingSlime.teleportTimer = 0.0f;
            } else {
                CCLOG("KingSlime: Teleport ready but waiting for conditions (HasTarget: %d, OnGround: %d, IsTeleporting: %d)", 
                      aggro.targetEntity != ecs::INVALID_ENTITY ? 1 : 0, ground.isOnGround ? 1 : 0, kingSlime.isTeleporting ? 1 : 0);
            }
        }
    }
    
    void performTeleport(entt::entity entity, KingSlimeComponent& kingSlime, TransformComponent& transform, 
                        AggroComponent& aggro, SpriteStateComponent& state, RenderComponent& render,
                        AnimationStateComponent* animState) {
        auto targetEntity = static_cast<entt::entity>(aggro.targetEntity);
        
        // Validate entity before accessing it
        if (!_registry->valid(targetEntity)) {
            CCLOG("KingSlime: Target entity is invalid, cannot perform teleport");
            return;
        }
        
        auto* targetTransform = _registry->try_get<TransformComponent>(targetEntity);
        
        if (targetTransform && state.spriteCreated && state.spriteHandle) {
            auto* sprite = static_cast<cocos2d::Sprite*>(state.spriteHandle);
            float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;
            cocos2d::Vec2 offset = {
                cosf(angle) * kingSlime.teleportRange,
                sinf(angle) * kingSlime.teleportRange
            };
            
            cocos2d::Vec2 teleportPos = targetTransform->position + offset;
            
            float groundHeight = 100.0f;
            float bossHeight = 152.0f;
            float safeHeight = groundHeight + bossHeight + 50.0f;
            
            teleportPos.y = std::max(teleportPos.y, safeHeight);
            
            CCLOG("KingSlime: Calculated teleport position (%.1f, %.1f), ground safe height: %.1f", 
                  teleportPos.x, teleportPos.y, safeHeight);
            
            float currentScale = kingSlime.currentScale;
            
            sprite->stopAllActions();
            kingSlime.isTeleporting = true;
            
            // 禁用RenderComponent同步，让Action系统控制动画
            render.enableSync = false;
            
            // 设置传送出动画状态
            kingSlime.aiState = KingSlimeComponent::TELEPORTING_OUT;
            if (animState) {
                AnimationStateHelper::updateAnimationState(*animState,
                    AnimationStateHelper::kingSlimeStateToAnimationState(kingSlime.aiState), true);
            }
            
            auto shrinkAction = cocos2d::ScaleTo::create(0.5f, 0.1f);
            auto fadeOutAction = cocos2d::FadeOut::create(0.5f);
            auto disappearAction = cocos2d::Spawn::create(shrinkAction, fadeOutAction, nullptr);
            
            // Teleport action - directly modify sprite without lambda
            auto teleportAction = cocos2d::CallFunc::create([teleportPos, sprite]() {
                if (sprite) {
                    sprite->setPosition(teleportPos);
                    if (sprite->getPhysicsBody()) {
                        sprite->getPhysicsBody()->setVelocity(cocos2d::Vec2::ZERO);
                    }
                    sprite->setScale(0.1f);
                    sprite->setOpacity(0);
                    CCLOG("KingSlime: Teleported to (%.1f, %.1f)", teleportPos.x, teleportPos.y);
                }
            });

            auto* registryPtr = _registry;
            auto switchToTeleportInState = cocos2d::CallFunc::create([registryPtr, entity]() {
                if (!registryPtr) return;
                if (!registryPtr->valid(entity)) return;

                auto* kingSlimePtr = registryPtr->try_get<KingSlimeComponent>(entity);
                if (kingSlimePtr) {
                    kingSlimePtr->aiState = KingSlimeComponent::TELEPORTING_IN;
                }

                if (auto* animStatePtr = registryPtr->try_get<AnimationStateComponent>(entity)) {
                    AnimationStateHelper::updateAnimationState(*animStatePtr,
                        AnimationStateHelper::kingSlimeStateToAnimationState(
                            kingSlimePtr ? kingSlimePtr->aiState : KingSlimeComponent::TELEPORTING_IN),
                        true);
                }
            });
            
            auto scaleAction = cocos2d::ScaleTo::create(0.5f, currentScale);
            auto fadeInAction = cocos2d::FadeIn::create(0.5f);
            auto appearAction = cocos2d::Spawn::create(scaleAction, fadeInAction, nullptr);
            
            auto finishAction = cocos2d::CallFunc::create([registryPtr, entity, sprite, currentScale]() {
                if (sprite) {
                    sprite->setScale(currentScale);
                    sprite->setOpacity(255);
                    CCLOG("KingSlime: Teleport animation complete");
                }

                if (!registryPtr) return;
                if (!registryPtr->valid(entity)) return;

                if (auto* renderPtr = registryPtr->try_get<RenderComponent>(entity)) {
                    renderPtr->enableSync = true;
                    renderPtr->scale = currentScale;
                    renderPtr->opacity = 255;
                }

                auto* kingSlimePtr = registryPtr->try_get<KingSlimeComponent>(entity);
                if (kingSlimePtr) {
                    kingSlimePtr->isTeleporting = false;
                    kingSlimePtr->aiState = KingSlimeComponent::IDLE;
                }

                if (auto* animStatePtr = registryPtr->try_get<AnimationStateComponent>(entity)) {
                    AnimationStateHelper::updateAnimationState(*animStatePtr,
                        AnimationStateHelper::kingSlimeStateToAnimationState(KingSlimeComponent::IDLE),
                        true);
                }
            });
            
            auto teleportSequence = cocos2d::Sequence::create(
                disappearAction,
                teleportAction,
                switchToTeleportInState,
                appearAction,
                finishAction,
                nullptr
            );
            
            sprite->runAction(teleportSequence);
            
            CCLOG("KingSlime: Starting teleport animation to (%.1f, %.1f)", 
                  teleportPos.x, teleportPos.y);
        }
    }
    
    void updateSlimeSpawning(KingSlimeComponent& kingSlime, float healthPercent, 
                           TransformComponent& transform, SpriteStateComponent& state) {
        if (kingSlime.shouldSpawnSlime(healthPercent)) {
            if (kingSlime.slimesSpawned < kingSlime.totalSlimesToSpawn) {
                std::string slimeType = kingSlime.getRandomSlimeType();
                spawnSlimeMinion(slimeType, transform.position, kingSlime.currentScale, &state);
                kingSlime.slimesSpawned++;
                
                CCLOG("KingSlime: Spawned %s (%d/%d total) at scale %.2f", slimeType.c_str(), 
                      kingSlime.slimesSpawned, kingSlime.totalSlimesToSpawn, kingSlime.currentScale);
            }
        }
    }
    
    void spawnSlimeMinion(const std::string& slimeType, cocos2d::Vec2 bossPosition, float bossScale = 1.0f, SpriteStateComponent* bossState = nullptr) {
        if (!_sceneContext || !_registry) {
            CCLOG("ERROR: Cannot spawn slime - missing scene context or registry");
            return;
        }
        
        cocos2d::Vec2 spawnPos = bossPosition;
        
        if (bossState && bossState->spriteCreated && bossState->spriteHandle) {
            auto* sprite = static_cast<cocos2d::Sprite*>(bossState->spriteHandle);
            cocos2d::Rect boundingBox = sprite->getBoundingBox();
            float bottomY = boundingBox.getMinY();
            float minX = boundingBox.getMinX() + boundingBox.size.width * 0.1f;
            float maxX = boundingBox.getMaxX() - boundingBox.size.width * 0.1f;
            float randomX = minX + (static_cast<float>(rand()) / RAND_MAX * (maxX - minX));
            
            spawnPos = {randomX, bottomY - 10.0f};
            
            CCLOG("KingSlime: Spawn position calculated from sprite bounding box - X: %.1f, Y: %.1f, BBox: (%.1f, %.1f) to (%.1f, %.1f)",
                  spawnPos.x, spawnPos.y, boundingBox.getMinX(), boundingBox.getMinY(), 
                  boundingBox.getMaxX(), boundingBox.getMaxY());
        } else {
            spawnPos = bossPosition;
        }
        
        auto& factory = MonsterMasterFactory::getInstance();
        
        auto slimeEntityId = factory.createMonster(*_registry, slimeType, spawnPos.x, spawnPos.y, _sceneContext);
        if (slimeEntityId != ecs::INVALID_ENTITY) {
            CCLOG("KingSlime: Spawned %s at (%.1f, %.1f)", slimeType.c_str(), 
                  spawnPos.x, spawnPos.y);
        } else {
            CCLOG("ERROR: Failed to create %s minion!", slimeType.c_str());
        }
    }
    
    void updateJumpPattern(entt::entity entity, KingSlimeComponent& kingSlime, JumpMovementComponent& jump, 
                          AggroComponent& aggro, SpriteStateComponent& state, float healthPercent) {
        static std::unordered_map<entt::entity, float> lastJumpTimer;
        
        float lastTimer = lastJumpTimer[entity];
        float currentTimer = jump.jumpTimer;
        
        float threshold = jump.jumpCooldown * 0.9f;
        
        if (currentTimer >= threshold && lastTimer < threshold) {
            float forceMultiplier = kingSlime.getJumpForceMultiplier(healthPercent);
            
            bool isSmallJump = kingSlime.executeNextJump();
            
            if (!isSmallJump) {
                jump.maxHorizontalImpulse = kingSlime.baseHorizontalImpulse * kingSlime.bigJumpMultiplier * forceMultiplier;
                jump.maxVerticalImpulse = kingSlime.baseVerticalImpulse * kingSlime.bigJumpMultiplier * forceMultiplier;
                kingSlime.isDoingBigJump = true;
                
                CCLOG("KingSlime: Setting BIG JUMP parameters (%.1f, %.1f) HP:%.0f%% ForceMultiplier:%.2fx", 
                      jump.maxHorizontalImpulse, jump.maxVerticalImpulse, healthPercent * 100, forceMultiplier);
            } else {
                jump.maxHorizontalImpulse = kingSlime.baseHorizontalImpulse * forceMultiplier;
                jump.maxVerticalImpulse = kingSlime.baseVerticalImpulse * forceMultiplier;
                kingSlime.isDoingBigJump = false;
                
                CCLOG("KingSlime: Setting small jump %d/%d parameters (%.1f, %.1f) HP:%.0f%% ForceMultiplier:%.2fx", 
                      kingSlime.jumpCount, kingSlime.smallJumpsPerCycle,
                      jump.maxHorizontalImpulse, jump.maxVerticalImpulse, healthPercent * 100, forceMultiplier);
            }
        }
        
        lastJumpTimer[entity] = currentTimer;
    }
};

} // namespace ecs

#endif // __ECS_SYSTEM_KINGSLIMEAISYSTEMENTT_H__
