#ifndef __ECS_SYSTEM_BATAISYSTEMENTT_H__
#define __ECS_SYSTEM_BATAISYSTEMENTT_H__

#include "systems/npc/OptimizedAISystemBase.h"
#include "systems/core/SystemPriority.h"
#include "components/AllComponents.h"
#include "cocos2d.h"
#include <algorithm>
#include <cstdlib>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace ecs {

class BatAISystemEntt : public OptimizedAISystemBase {
public:
  const char* getName() const override { return "BatAISystem"; }
  int getPriority() const override { return SystemPriority::MOVEMENT; }

  void update(float delta) override {
    if (!_registry) return;

    incrementFrameCounter();

    auto view = _registry->view<BatMovementComponent, AggroComponent,
                                SpriteStateComponent, RenderComponent, TransformComponent>();

    view.each([this, delta](auto entity,
                           BatMovementComponent& bat,
                           AggroComponent& aggro,
                           SpriteStateComponent& state,
                           RenderComponent& render,
                           TransformComponent& transform) {
      if (!state.spriteCreated || !state.spriteHandle) return;

      auto* stateFlags = _registry->try_get<EntityStateFlags>(entity);
      if (!shouldUpdateEntity(entity, stateFlags)) {
        return;
      }

      auto* sprite = static_cast<cocos2d::Sprite*>(state.spriteHandle);
      auto* body = sprite->getPhysicsBody();
      if (!body) return;

      bool hasTarget = false;
      cocos2d::Vec2 targetPos = cocos2d::Vec2::ZERO;

      if (aggro.hasAggro && aggro.targetEntity != INVALID_ENTITY) {
        targetPos = transform.position + aggro.directionToTarget * aggro.distanceToTarget;
        hasTarget = true;
      }

      if (bat.chaseCooldownTimer > 0.0f) {
        bat.chaseCooldownTimer -= delta;
        if (bat.chaseCooldownTimer < 0.0f) bat.chaseCooldownTimer = 0.0f;
      }

      if (bat.isChasing) {
        bat.chaseTimer += delta;
        if (bat.chaseTimer >= bat.chaseDuration) {
          bat.isChasing = false;
          bat.chaseTimer = 0.0f;
          bat.chaseCooldownTimer = bat.chaseCooldown;
        }
      } else if (hasTarget) {
        if (bat.chaseCooldownTimer <= 0.0f && aggro.distanceToTarget <= aggro.aggroRange * 1.2f) {
          bat.isChasing = true;
          bat.chaseTimer = 0.0f;
        }
      }

      bat.wobblePhase += bat.wobbleFrequency * delta;
      bat.wanderTimer += delta;

      if (bat.wanderTimer >= bat.wanderChangeInterval) {
        bat.wanderTimer = 0.0f;
        float baseAngle = bat.currentAngle;
        if (hasTarget) {
          baseAngle = atan2(aggro.directionToTarget.y, aggro.directionToTarget.x);
        }
        float randOffset = (((float)rand() / RAND_MAX) * 2.0f - 1.0f) * 1.2f;
        bat.wanderAngle = baseAngle + randOffset;
      }

      if (hasTarget) {
        cocos2d::Vec2 toTarget = targetPos - transform.position;
        if (!toTarget.isZero()) {
          toTarget.normalize();
          float targetAngle = atan2(toTarget.y, toTarget.x);
          if (bat.isChasing) {
            bat.targetAngle = targetAngle;
          } else {
            float wanderMix = 0.55f;
            float blended = (1.0f - wanderMix) * targetAngle + wanderMix * bat.wanderAngle;
            bat.targetAngle = blended;
          }
        }
      } else {
        bat.targetAngle = bat.wanderAngle;
      }

      bat.smoothTurn(delta);

      float wobbleOffset = sin(bat.wobblePhase) * bat.wobbleAmplitude;
      float effectiveAngle = bat.currentAngle + wobbleOffset;

      bat.currentVelocity = body->getVelocity();

      float targetSpeed = bat.isChasing ? bat.maxSpeed : bat.flySpeed;
      cocos2d::Vec2 desiredVelocity;
      desiredVelocity.x = cos(effectiveAngle) * targetSpeed;
      desiredVelocity.y = sin(effectiveAngle) * targetSpeed;

      float lerpFactor = std::min(1.0f, bat.acceleration * delta / std::max(1.0f, bat.flySpeed));
      cocos2d::Vec2 newVelocity;
      newVelocity.x = bat.currentVelocity.x + (desiredVelocity.x - bat.currentVelocity.x) * lerpFactor;
      newVelocity.y = bat.currentVelocity.y + (desiredVelocity.y - bat.currentVelocity.y) * lerpFactor;

      float newSpeed = newVelocity.length();
      if (newSpeed > bat.maxSpeed * 1.5f) {
        newVelocity.normalize();
        newVelocity *= bat.maxSpeed * 1.5f;
      }

      if (newSpeed < 40.0f && newSpeed > 0.1f) {
        newVelocity.normalize();
        newVelocity *= 40.0f;
      }

      body->setVelocity(newVelocity);
      transform.position = sprite->getPosition();

      if (newVelocity.x != 0.0f) {
        render.flipX = newVelocity.x > 0.0f;
      }
    });
  }
};

} // namespace ecs

#endif // __ECS_SYSTEM_BATAISYSTEMENTT_H__
