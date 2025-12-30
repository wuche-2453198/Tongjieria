#ifndef __ECS_SYSTEM_VULTUREAISYSTEMENTT_H__
#define __ECS_SYSTEM_VULTUREAISYSTEMENTT_H__

#include "systems/npc/OptimizedAISystemBase.h"
#include "systems/core/SystemPriority.h"
#include "systems/core/EntityDestructionManager.h"
#include "components/AllComponents.h"
#include "core/factory/monster/MonsterMasterFactory.h"
#include "cocos2d.h"
#include <cmath>
#include <algorithm>
#include <vector>

namespace ecs {

class VultureAISystemEntt : public OptimizedAISystemBase {
public:
    const char* getName() const override { return "VultureAISystem"; }
    int getPriority() const override { return SystemPriority::MOVEMENT; }

    void update(float delta) override {
        if (!_registry) return;

        incrementFrameCounter();

        struct PendingFlyingVultureSpawn {
            cocos2d::Vec2 position = cocos2d::Vec2::ZERO;
            cocos2d::Node* parentNode = nullptr;
            HealthComponent health;
            AggroComponent aggro;
            bool flipX = false;
            float rotation = 0.0f;
        };

        std::vector<PendingFlyingVultureSpawn> pendingFlyingSpawns;

        auto view = _registry->view<VultureMovementComponent, AggroComponent, HealthComponent,
                                    SpriteStateComponent, RenderComponent, TransformComponent, PhysicsBodyComponent>();

        view.each([this, delta, &pendingFlyingSpawns](entt::entity entity,
                               VultureMovementComponent& vulture,
                               AggroComponent& aggro,
                               HealthComponent& health,
                               SpriteStateComponent& state,
                               RenderComponent& render,
                               TransformComponent& transform,
                               PhysicsBodyComponent& physics) {
            if (!state.spriteCreated || !state.spriteHandle) return;
            if (health.currentHealth <= 0.0f || health.isDead) return;

            auto* stateFlags = _registry->try_get<EntityStateFlags>(entity);
            if (!shouldUpdateEntity(entity, stateFlags)) {
                return;
            }

            auto* animState = _registry->try_get<AnimationStateComponent>(entity);

            if (vulture.lastHealth < 0.0f) {
                vulture.lastHealth = health.currentHealth;
            } else if (health.currentHealth < vulture.lastHealth) {
                vulture.activated = true;
                vulture.lastHealth = health.currentHealth;
            } else {
                vulture.lastHealth = health.currentHealth;
            }

            cocos2d::Vec2 targetPos = cocos2d::Vec2::ZERO;
            bool hasTarget = false;
            if (aggro.targetEntity != INVALID_ENTITY) {
                auto targetEntity = static_cast<entt::entity>(aggro.targetEntity);
                if (_registry->valid(targetEntity)) {
                    auto* targetTransform = _registry->try_get<TransformComponent>(targetEntity);
                    if (targetTransform) {
                        targetPos = targetTransform->position;
                        hasTarget = true;
                    }
                }
            }

            if (!hasTarget && !aggro.directionToTarget.isZero() && aggro.distanceToTarget < 90000.0f) {
                targetPos = transform.position + aggro.directionToTarget * aggro.distanceToTarget;
                hasTarget = true;
            }

            if (vulture.aiState == VultureMovementComponent::IDLE) {
                if (hasTarget) {
                    if (aggro.distanceToTarget <= aggro.aggroRange) {
                        // Vulture 贴图默认朝左，目标在右侧时需要翻转
                        render.flipX = aggro.directionToTarget.x > 0.0f;
                    }

                    if (aggro.distanceToTarget <= vulture.activationRange) {
                        vulture.activated = true;
                    }
                }

                if (vulture.activated && hasTarget) {
                    auto* sprite = static_cast<cocos2d::Sprite*>(state.spriteHandle);
                    cocos2d::Vec2 spawnPos = transform.position;
                    cocos2d::Node* parentNode = nullptr;
                    if (sprite) {
                        spawnPos = sprite->getPosition();
                        parentNode = sprite->getParent();
                    }
                    if (!parentNode) {
                        if (auto* parentComp = _registry->try_get<ParentNodeComponent>(entity)) {
                            parentNode = parentComp->parentNode;
                        }
                    }

                    PendingFlyingVultureSpawn spawn;
                    spawn.position = spawnPos;
                    spawn.parentNode = parentNode;
                    spawn.health = health;
                    spawn.aggro = aggro;
                    spawn.flipX = render.flipX;
                    spawn.rotation = render.rotation;
                    pendingFlyingSpawns.push_back(spawn);

                    // 立即隐藏 Idle 形态，避免一帧内出现两个贴图重叠
                    render.visible = false;

                    EntityDestructionManager::getInstance().queueDestruction(*_registry, entity);
                    return;
                } else {
                    if (animState && animState->currentState != "idle") {
                        animState->setState("idle", true);
                    }
                    auto* sprite = static_cast<cocos2d::Sprite*>(state.spriteHandle);
                    auto* body = sprite->getPhysicsBody();
                    if (body) {
                        // Idle时允许重力下落
                        cocos2d::Vec2 v = body->getVelocity();
                        body->setVelocity(cocos2d::Vec2(0.0f, v.y));
                    }
                    return;
                }
            }

            vulture.debugLogTimer += delta;

            if (!hasTarget || !aggro.hasAggro) {
                auto* sprite = static_cast<cocos2d::Sprite*>(state.spriteHandle);
                auto* body = sprite->getPhysicsBody();
                if (body) {
                    if (vulture.debugLogTimer >= 0.5f && vulture.debugLogCount < 20) {
                        vulture.debugLogTimer = 0.0f;
                        vulture.debugLogCount++;

                        cocos2d::Vec2 spritePos = sprite->getPosition();
                        cocos2d::Vec2 spriteWorldCenter = sprite->convertToWorldSpace(
                            cocos2d::Vec2(sprite->getContentSize().width * 0.5f, sprite->getContentSize().height * 0.5f));
                        cocos2d::Vec2 anchor = sprite->getAnchorPoint();
                        cocos2d::Size cs = sprite->getContentSize();
                        cocos2d::Vec2 parentPos = sprite->getParent() ? sprite->getParent()->getPosition() : cocos2d::Vec2::ZERO;
                        cocos2d::Vec2 bodyVel = body->getVelocity();
                        cocos2d::Vec2 bodyPos = body->getPosition();
                        cocos2d::Vec2 bodyOff = body->getPositionOffset();
                        cocos2d::Vec2 bodyMinusSprite = bodyPos - spritePos;
                        int actions = sprite->getNumberOfRunningActions();
                        bool hasWorld = (body->getWorld() != nullptr);

                        CCLOG("[VultureDbg] e=%u state=%d act=%d hasTarget=%d dist=%.1f dir=(%.2f,%.2f)\n"
                              "  transform=(%.1f,%.1f) sprite=(%.1f,%.1f) spriteWorldCenter=(%.1f,%.1f) parentPos=(%.1f,%.1f) anchor=(%.2f,%.2f) cs=(%.1f,%.1f)\n"
                              "  bodyPos=(%.1f,%.1f) bodyOff=(%.1f,%.1f) body-sprite=(%.1f,%.1f)\n"
                              "  vel=(%.1f,%.1f) actions=%d hasWorld=%d enabled=%d dyn=%d grav=%d sensor=%d cat=0x%X col=0x%X grp=%d needsCreation=%d",
                              entt::to_integral(entity), (int)vulture.aiState, (int)vulture.activated, (int)hasTarget,
                              aggro.distanceToTarget, aggro.directionToTarget.x, aggro.directionToTarget.y,
                              transform.position.x, transform.position.y, spritePos.x, spritePos.y,
                              spriteWorldCenter.x, spriteWorldCenter.y, parentPos.x, parentPos.y, anchor.x, anchor.y, cs.width, cs.height,
                              bodyPos.x, bodyPos.y, bodyOff.x, bodyOff.y, bodyMinusSprite.x, bodyMinusSprite.y,
                              bodyVel.x, bodyVel.y, actions, (int)hasWorld,
                              (int)body->isEnabled(), (int)body->isDynamic(), (int)body->isGravityEnabled(),
                              (int)(body->getFirstShape() ? body->getFirstShape()->isSensor() : false),
                              body->getCategoryBitmask(), body->getCollisionBitmask(), body->getGroup(), (int)physics.needsCreation);
                    }

                    body->setVelocity(cocos2d::Vec2::ZERO);
                }
                vulture.currentVelocity = cocos2d::Vec2::ZERO;
                transform.velocity = cocos2d::Vec2::ZERO;
                return;
            }

            if (vulture.dashCooldownTimer > 0.0f) {
                vulture.dashCooldownTimer = std::max(0.0f, vulture.dashCooldownTimer - delta);
            }

            if (vulture.aiState != VultureMovementComponent::IDLE) {
                vulture.wobblePhase += vulture.wobbleFrequency * delta;
            }

            cocos2d::Vec2 toTarget = targetPos - transform.position;
            float horizontalDist = std::abs(toTarget.x);

            if (vulture.aiState == VultureMovementComponent::HOVERING) {
                if (horizontalDist <= vulture.dashTriggerDistance && vulture.dashCooldownTimer <= 0.0f) {
                    vulture.aiState = VultureMovementComponent::DASHING;
                    vulture.aiTimer = 0.0f;
                    if (animState && animState->currentState != "dashing") {
                        animState->setState("dashing");
                    }
                }
            } else if (vulture.aiState == VultureMovementComponent::DASHING) {
                vulture.aiTimer += delta;

                if (vulture.aiTimer >= vulture.dashDuration) {
                    vulture.aiState = VultureMovementComponent::HOVERING;
                    vulture.aiTimer = 0.0f;
                    vulture.dashCooldownTimer = vulture.dashCooldown;
                    if (animState && animState->currentState != "hovering") {
                        animState->setState("hovering", true);
                    }
                }

                if (horizontalDist >= vulture.dashReturnDistance) {
                    vulture.aiState = VultureMovementComponent::HOVERING;
                    vulture.aiTimer = 0.0f;
                    vulture.dashCooldownTimer = vulture.dashCooldown;
                    if (animState && animState->currentState != "hovering") {
                        animState->setState("hovering", true);
                    }
                }
            }

            float wobbleY = std::sin(vulture.wobblePhase) * vulture.wobbleAmplitude;
            float wobbleVelY = std::cos(vulture.wobblePhase) * vulture.wobbleAmplitude * vulture.wobbleFrequency;

            cocos2d::Vec2 desiredVelocity = cocos2d::Vec2::ZERO;
            if (vulture.aiState == VultureMovementComponent::HOVERING) {
                cocos2d::Vec2 desiredPos(targetPos.x, targetPos.y + vulture.hoverDistance + wobbleY);
                cocos2d::Vec2 toDesired = desiredPos - transform.position;
                if (!toDesired.isZero()) {
                    toDesired.normalize();
                    desiredVelocity = toDesired * vulture.flySpeed;
                }
                desiredVelocity.y += wobbleVelY;
                if (animState && animState->currentState != "hovering") {
                    animState->setState("hovering");
                }
            } else if (vulture.aiState == VultureMovementComponent::DASHING) {
                cocos2d::Vec2 toDash = (targetPos + cocos2d::Vec2(0.0f, wobbleY * 0.5f)) - transform.position;
                if (!toDash.isZero()) {
                    toDash.normalize();
                    desiredVelocity = toDash * vulture.flySpeed;
                }
                desiredVelocity.y += wobbleVelY * 0.5f;
                if (animState && animState->currentState != "dashing") {
                    animState->setState("dashing");
                }
            }

            if (desiredVelocity.x != 0.0f) {
                // Vulture 贴图默认朝左，向右飞时需要翻转
                render.flipX = desiredVelocity.x > 0.0f;
            }

            auto* sprite = static_cast<cocos2d::Sprite*>(state.spriteHandle);
            auto* body = sprite->getPhysicsBody();
            if (!body) return;

            cocos2d::Vec2 desiredVelocityWorld = desiredVelocity;
            float parentScaleX = 1.0f;
            float parentScaleY = 1.0f;
            if (auto* parentNode = sprite->getParent()) {
                parentScaleX = parentNode->getScaleX();
                parentScaleY = parentNode->getScaleY();

                cocos2d::Vec2 w0 = parentNode->convertToWorldSpace(cocos2d::Vec2::ZERO);
                cocos2d::Vec2 w1 = parentNode->convertToWorldSpace(desiredVelocity);
                desiredVelocityWorld = w1 - w0;
            }

            if (vulture.debugLogTimer >= 0.5f && vulture.debugLogCount < 20) {
                vulture.debugLogTimer = 0.0f;
                vulture.debugLogCount++;

                int cat = body->getCategoryBitmask();
                int col = body->getCollisionBitmask();
                int grp = body->getGroup();
                bool enabled = body->isEnabled();
                bool dyn = body->isDynamic();
                bool grav = body->isGravityEnabled();
                bool sensor = false;
                if (auto* shape = body->getFirstShape()) {
                    sensor = shape->isSensor();
                }

                cocos2d::Vec2 spritePos = sprite->getPosition();
                cocos2d::Vec2 spriteWorldCenter = sprite->convertToWorldSpace(
                    cocos2d::Vec2(sprite->getContentSize().width * 0.5f, sprite->getContentSize().height * 0.5f));
                cocos2d::Vec2 anchor = sprite->getAnchorPoint();
                cocos2d::Size cs = sprite->getContentSize();
                cocos2d::Vec2 parentPos = sprite->getParent() ? sprite->getParent()->getPosition() : cocos2d::Vec2::ZERO;
                cocos2d::Vec2 bodyVel = body->getVelocity();
                cocos2d::Vec2 bodyPos = body->getPosition();
                cocos2d::Vec2 bodyOff = body->getPositionOffset();
                cocos2d::Vec2 bodyMinusSprite = bodyPos - spritePos;
                int actions = sprite->getNumberOfRunningActions();
                bool hasWorld = (body->getWorld() != nullptr);
                cocos2d::Vec2 dir = aggro.directionToTarget;
                float dist = aggro.distanceToTarget;

                CCLOG("[VultureDbg] e=%u state=%d act=%d hasTarget=%d dist=%.1f dir=(%.2f,%.2f)\n"
                      "  transform=(%.1f,%.1f) sprite=(%.1f,%.1f) spriteWorldCenter=(%.1f,%.1f) parentPos=(%.1f,%.1f) anchor=(%.2f,%.2f) cs=(%.1f,%.1f)\n"
                      "  bodyPos=(%.1f,%.1f) bodyOff=(%.1f,%.1f) body-sprite=(%.1f,%.1f)\n"
                      "  target=(%.1f,%.1f)\n"
                      "  desiredP=(%.1f,%.1f) desiredW=(%.1f,%.1f) parentScale=(%.2f,%.2f)\n"
                      "  vel=(%.1f,%.1f) actions=%d hasWorld=%d enabled=%d dyn=%d grav=%d sensor=%d cat=0x%X col=0x%X grp=%d needsCreation=%d",
                      entt::to_integral(entity), (int)vulture.aiState, (int)vulture.activated, (int)hasTarget,
                      dist, dir.x, dir.y,
                      transform.position.x, transform.position.y, spritePos.x, spritePos.y,
                      spriteWorldCenter.x, spriteWorldCenter.y, parentPos.x, parentPos.y, anchor.x, anchor.y, cs.width, cs.height,
                      bodyPos.x, bodyPos.y, bodyOff.x, bodyOff.y, bodyMinusSprite.x, bodyMinusSprite.y,
                      targetPos.x, targetPos.y,
                      desiredVelocity.x, desiredVelocity.y, desiredVelocityWorld.x, desiredVelocityWorld.y, parentScaleX, parentScaleY,
                      bodyVel.x, bodyVel.y, actions, (int)hasWorld, (int)enabled, (int)dyn, (int)grav, (int)sensor,
                      cat, col, grp, (int)physics.needsCreation);
            }

            cocos2d::Vec2 currentVel = body->getVelocity();
            float lerpFactor = std::min(1.0f, (vulture.acceleration * delta) / std::max(1.0f, vulture.flySpeed));

            cocos2d::Vec2 newVel;
            newVel.x = currentVel.x + (desiredVelocityWorld.x - currentVel.x) * lerpFactor;
            newVel.y = currentVel.y + (desiredVelocityWorld.y - currentVel.y) * lerpFactor;

            float speed = newVel.length();
            if (speed > vulture.maxSpeed && speed > 0.01f) {
                newVel.normalize();
                newVel *= vulture.maxSpeed;
            }

            body->setVelocity(newVel);
            vulture.currentVelocity = newVel;

            // 写回Transform速度，供EntityStateUpdateSystem正确判断isIdle/LOD
            transform.velocity = newVel;

            markEntityUpdated(stateFlags);
        });

        for (const auto& spawn : pendingFlyingSpawns) {
            ecs::EntityId newEntityId = MonsterMasterFactory::getInstance().createMonster(
                *_registry, "FlyingVulture", spawn.position.x, spawn.position.y, spawn.parentNode);
            if (newEntityId == ecs::INVALID_ENTITY) {
                continue;
            }

            auto newEntity = static_cast<entt::entity>(newEntityId);
            if (!_registry->valid(newEntity)) {
                continue;
            }

            if (auto* newTransform = _registry->try_get<TransformComponent>(newEntity)) {
                newTransform->position = spawn.position;
                newTransform->previousPosition = spawn.position;
            }
            if (auto* newHealth = _registry->try_get<HealthComponent>(newEntity)) {
                *newHealth = spawn.health;
            }
            if (auto* newAggro = _registry->try_get<AggroComponent>(newEntity)) {
                *newAggro = spawn.aggro;
            }
            if (auto* newRender = _registry->try_get<RenderComponent>(newEntity)) {
                newRender->flipX = spawn.flipX;
                newRender->rotation = spawn.rotation;
            }
        }
    }

private:
    void enterFlyingState(entt::entity entity,
                          VultureMovementComponent& vulture,
                          AnimationStateComponent* animState,
                          SpriteStateComponent& state,
                          RenderComponent& render,
                          PhysicsBodyComponent& physics,
                          bool fromIdle) {
        vulture.aiState = VultureMovementComponent::HOVERING;
        vulture.aiTimer = 0.0f;

        if (animState) {
            animState->setState("hovering", true);
        }

        auto* sprite = static_cast<cocos2d::Sprite*>(state.spriteHandle);
        auto* body = sprite ? sprite->getPhysicsBody() : nullptr;
        float oldHeight = physics.height;
        if (body) {
            body->setGravityEnable(false);
            body->setLinearDamping(0.05f);
            body->setVelocity(cocos2d::Vec2::ZERO);
            body->setAngularVelocity(0.0f);
        }

        physics.gravityEnabled = false;
        physics.linearDamping = 0.05f;
        physics.needsCreation = false;

        if (fromIdle) {
            render.rotation = 0.0f;
        }
    }
};

} // namespace ecs

#endif // __ECS_SYSTEM_VULTUREAISYSTEMENTT_H__
