#ifndef __ECS_SYSTEM_KINGSLIMEAISYSTEMENTT_H__
#define __ECS_SYSTEM_KINGSLIMEAISYSTEMENTT_H__

#include "ISystemEntt.h"
#include "SystemPriority.h"
#include "../AllComponents.h"
#include "../MonsterFactory.h"
#include "AnimationStateHelper.h"
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
 */
class KingSlimeAISystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "KingSlimeAISystem"; }
    int getPriority() const override { return SystemPriority::AI; }
    
    void setSceneContext(cocos2d::Node* scene) {
        _sceneContext = scene;
    }

private:
    cocos2d::Node* _sceneContext = nullptr;

    void update(float delta) override {
        auto view = _registry->view<KingSlimeComponent, JumpMovementComponent, 
                                   HealthComponent, AggroComponent, 
                                   SpriteStateComponent, RenderComponent, TransformComponent, 
                                   GroundDetectorComponent>();
        
        view.each([delta, this](auto entity, KingSlimeComponent& kingSlime,
                               JumpMovementComponent& jump, HealthComponent& health,
                               AggroComponent& aggro, SpriteStateComponent& state,
                               RenderComponent& render, TransformComponent& transform, GroundDetectorComponent& ground) {
            
            if (!kingSlime.isActive || health.currentHealth <= 0) return;
            
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
            updateTeleportSystem(kingSlime, transform, aggro, ground, animState, delta);
            
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
    
    void updateTeleportSystem(KingSlimeComponent& kingSlime, TransformComponent& transform, 
                             AggroComponent& aggro, GroundDetectorComponent& ground, 
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
            if (aggro.targetEntity != ecs::INVALID_ENTITY && ground.isOnGround) {
                CCLOG("KingSlime: Teleport ready! Executing immediate teleport");
                
                auto view = _registry->view<ecs::SpriteStateComponent, ecs::RenderComponent>();
                for (auto entity : view) {
                    if (_registry->try_get<ecs::KingSlimeComponent>(entity)) {
                        auto& state = _registry->get<ecs::SpriteStateComponent>(entity);
                        auto& render = _registry->get<ecs::RenderComponent>(entity);
                        performTeleport(kingSlime, transform, aggro, state, render, animState);
                        break;
                    }
                }
                
                kingSlime.teleportTimer = 0.0f;
            } else {
                CCLOG("KingSlime: Teleport ready but waiting for conditions (HasTarget: %d, OnGround: %d)", 
                      aggro.targetEntity != ecs::INVALID_ENTITY ? 1 : 0, ground.isOnGround ? 1 : 0);
            }
        }
    }
    
    void performTeleport(KingSlimeComponent& kingSlime, TransformComponent& transform, 
                        AggroComponent& aggro, SpriteStateComponent& state, RenderComponent& render,
                        AnimationStateComponent* animState) {
        auto targetEntity = static_cast<entt::entity>(aggro.targetEntity);
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
            
            auto teleportAction = cocos2d::CallFunc::create([teleportPos, &transform, sprite, &render, currentScale]() {
                transform.position = teleportPos;
                sprite->setPosition(teleportPos);
                
                if (sprite->getPhysicsBody()) {
                    sprite->getPhysicsBody()->setVelocity(cocos2d::Vec2::ZERO);
                }
                
                sprite->setScale(0.1f);
                sprite->setOpacity(0);
                
                CCLOG("KingSlime: Teleported to (%.1f, %.1f) near player (physics synced)", 
                      teleportPos.x, teleportPos.y);
            });
            
            auto scaleAction = cocos2d::ScaleTo::create(0.5f, currentScale);
            auto fadeInAction = cocos2d::FadeIn::create(0.5f);
            auto appearAction = cocos2d::Spawn::create(scaleAction, fadeInAction, nullptr);
            
            auto resetAction = cocos2d::CallFunc::create([&render, &kingSlime, currentScale, animState]() {
                render.opacity = 255;
                render.scale = currentScale;
                kingSlime.isTeleporting = false;
                
                // 重新启用RenderComponent同步
                render.enableSync = true;
                
                // 设置传送入动画状态，然后回到待机
                kingSlime.aiState = KingSlimeComponent::IDLE;
                if (animState) {
                    AnimationStateHelper::updateAnimationState(*animState,
                        AnimationStateHelper::kingSlimeStateToAnimationState(kingSlime.aiState), true);
                }
            });
            
            auto teleportSequence = cocos2d::Sequence::create(
                disappearAction,
                teleportAction,
                appearAction,
                resetAction,
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
        
        auto& factory = MonsterFactory::getInstance();
        
        std::string configPath = "config/slimes/" + slimeType + ".json";
        factory.loadSingleConfig(configPath);
        
        if (factory.getConfig(slimeType)) {
            auto slimeEntityId = factory.createMonster(*_registry, slimeType, spawnPos.x, spawnPos.y, _sceneContext);
            if (slimeEntityId != ecs::INVALID_ENTITY) {
                CCLOG("KingSlime: Spawned %s at (%.1f, %.1f)", slimeType.c_str(), 
                      spawnPos.x, spawnPos.y);
            } else {
                CCLOG("ERROR: Failed to create %s minion!", slimeType.c_str());
            }
        } else {
            CCLOG("ERROR: Failed to load %s config!", slimeType.c_str());
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
