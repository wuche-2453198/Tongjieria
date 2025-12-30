#ifndef __ECS_SYSTEM_ANGRYBONESAISYSTEMENTT_H__
#define __ECS_SYSTEM_ANGRYBONESAISYSTEMENTT_H__

#include "systems/npc/OptimizedAISystemBase.h"
#include "systems/core/SystemPriority.h"
#include "components/AllComponents.h"
#include "cocos2d.h"
#include <cmath>

namespace ecs {

class AngryBonesAISystemEntt : public OptimizedAISystemBase {
public:
  const char* getName() const override { return "AngryBonesAISystem"; }
  int getPriority() const override { return SystemPriority::MOVEMENT; }

  void update(float delta) override {
    if (!_registry) return;

    incrementFrameCounter();

    auto view = _registry->view<WarriorMovementComponent, PounceAttackComponent,
                                GroundDetectorComponent, AggroComponent,
                                SpriteStateComponent, RenderComponent, TransformComponent>();

    view.each([this, delta](entt::entity entity,
                           WarriorMovementComponent& warrior,
                           PounceAttackComponent& pounce,
                           GroundDetectorComponent& ground,
                           AggroComponent& aggro,
                           SpriteStateComponent& state,
                           RenderComponent& render,
                           TransformComponent& transform) {
      if (!state.spriteCreated || !state.spriteHandle) return;

      auto* stateFlags = _registry->try_get<EntityStateFlags>(entity);
      if (!shouldUpdateEntity(entity, stateFlags)) {
        return;
      }

      if (pounce.cooldownTimer > 0.0f) {
        pounce.cooldownTimer -= delta;
        if (pounce.cooldownTimer < 0.0f) pounce.cooldownTimer = 0.0f;
      }

      if (!aggro.hasAggro || aggro.targetEntity == INVALID_ENTITY) {
        return;
      }

      auto targetEntity = static_cast<entt::entity>(aggro.targetEntity);
      if (!_registry->valid(targetEntity)) return;

      auto* targetTransform = _registry->try_get<TransformComponent>(targetEntity);
      if (!targetTransform) return;

      float dx = targetTransform->position.x - transform.position.x;
      float dy = targetTransform->position.y - transform.position.y;
      float absDx = std::abs(dx);

      bool closeToTarget = absDx <= pounce.triggerDistance;
      bool aboveTarget = dy >= pounce.aboveTargetHeight;

      if (!ground.isOnGround) {
        return;
      }

      if (pounce.cooldownTimer > 0.0f) {
        return;
      }

      if (!(closeToTarget || aboveTarget)) {
        return;
      }

      auto* sprite = static_cast<cocos2d::Sprite*>(state.spriteHandle);
      auto* body = sprite->getPhysicsBody();
      if (!body) return;

      float direction = dx >= 0.0f ? 1.0f : -1.0f;
      float maxSpeed = body->getVelocityLimit();

      cocos2d::Vec2 vel = body->getVelocity();
      vel.x = direction * maxSpeed;
      vel.y = pounce.verticalImpulse;
      body->setVelocity(vel);

      render.flipX = direction > 0.0f;

      pounce.cooldownTimer = pounce.cooldown;
      ground.isOnGround = false;
      ground.groundContactCount = 0;
      warrior.isJumping = true;
    });
  }
};

} // namespace ecs

#endif // __ECS_SYSTEM_ANGRYBONESAISYSTEMENTT_H__
