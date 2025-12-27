#ifndef __ECS_SYSTEM_DEMONAISYSTEMENTT_H__
#define __ECS_SYSTEM_DEMONAISYSTEMENTT_H__

#include "systems/npc/OptimizedAISystemBase.h"
#include "systems/core/SystemPriority.h"
#include "systems/core/EntityPoolManager.h"
#include "components/AllComponents.h"
#include "systems/render/SpriteManager.h"
#include "cocos2d.h"
#include <cmath>
#include <unordered_set>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace ecs {

class DemonAISystemEntt : public OptimizedAISystemBase {
public:
  const char *getName() const override { return "DemonAISystem"; }
  int getPriority() const override { return SystemPriority::AI; }

  void setSceneContext(cocos2d::Node *scene) { _sceneContext = scene; }

  void update(float delta) override {
    incrementFrameCounter();

    auto view = _registry->view<DemonMovementComponent, DemonAttackComponent, AggroComponent,
                                SpriteStateComponent, RenderComponent, TransformComponent>();

    view.each([this, delta](auto entity, DemonMovementComponent &move,
                           DemonAttackComponent &attack, AggroComponent &aggro,
                           SpriteStateComponent &state, RenderComponent &render,
                           TransformComponent &transform) {
      if (!state.spriteCreated || !state.spriteHandle) {
        return;
      }

      auto *stateFlags = _registry->try_get<EntityStateFlags>(entity);
      if (!shouldUpdateEntity(entity, stateFlags)) {
        return;
      }

      auto *sprite = static_cast<cocos2d::Sprite *>(state.spriteHandle);
      auto *body = sprite ? sprite->getPhysicsBody() : nullptr;
      if (!body) {
        return;
      }

      bool hasTarget = aggro.hasAggro && aggro.targetEntity != INVALID_ENTITY;
      cocos2d::Vec2 targetPos = cocos2d::Vec2::ZERO;
      if (hasTarget) {
        targetPos = transform.position + aggro.directionToTarget * aggro.distanceToTarget;
      }

      if (attack.cooldownTimer > 0.0f) {
        attack.cooldownTimer -= delta;
        if (attack.cooldownTimer < 0.0f) {
          attack.cooldownTimer = 0.0f;
        }
      }

      if (hasTarget) {
        updateChase(move, delta, aggro.directionToTarget);
        applyMovementPhysics(move, delta, body, transform);

        if (aggro.directionToTarget.x != 0) {
          render.flipX = aggro.directionToTarget.x < 0;
        }

        if (!attack.inVolley) {
          if (attack.cooldownTimer <= 0.0f && aggro.distanceToTarget <= attack.fireRange) {
            attack.inVolley = true;
            attack.shotsFired = 0;
            attack.shotTimer = 0.0f;
          }
        }

        if (attack.inVolley) {
          attack.shotTimer -= delta;
          if (attack.shotTimer <= 0.0f) {
            if (attack.shotsFired < attack.shotsPerVolley) {
              fireScythe(entity, attack, transform, state, targetPos);
              attack.shotsFired += 1;
              attack.shotTimer = attack.shotInterval;
            }
            if (attack.shotsFired >= attack.shotsPerVolley) {
              attack.inVolley = false;
              attack.cooldownTimer = attack.volleyCooldown;
            }
          }
        }
      }

      markEntityUpdated(stateFlags);
    });
  }

private:
  cocos2d::Node *_sceneContext = nullptr;
  std::unordered_set<std::string> _registeredProjectileResources;

  void updateChase(DemonMovementComponent &move, float delta, const cocos2d::Vec2 &dirToTarget) {
    cocos2d::Vec2 desiredDir = dirToTarget;
    if (!desiredDir.isZero()) {
      desiredDir.normalize();
      move.targetAngle = atan2(desiredDir.y, desiredDir.x);
      move.smoothTurn(delta);
    }
  }

  void applyMovementPhysics(DemonMovementComponent &move, float delta, cocos2d::PhysicsBody *body,
                           TransformComponent &transform) {
    cocos2d::Vec2 currentVel = body->getVelocity();
    move.currentVelocity = currentVel;

    cocos2d::Vec2 desiredVel;
    desiredVel.x = cos(move.currentAngle) * move.maxSpeed;
    desiredVel.y = sin(move.currentAngle) * move.maxSpeed;

    float lerpFactor = std::min(1.0f, move.acceleration * delta / std::max(1.0f, move.flySpeed));
    cocos2d::Vec2 newVel = currentVel + (desiredVel - currentVel) * lerpFactor;

    float newSpeed = newVel.length();
    if (newSpeed > move.maxSpeed) {
      newVel.normalize();
      newVel *= move.maxSpeed;
    }

    body->setVelocity(newVel);
    transform.position = static_cast<cocos2d::Sprite *>(body->getNode())->getPosition();
    move.currentVelocity = body->getVelocity();
  }

  void ensureScytheResourceRegistered() {
    const std::string resourceId = "projectile_picture/Projectile/Demon_Scythe_(projectile)";

    if (_registeredProjectileResources.find(resourceId) != _registeredProjectileResources.end()) {
      return;
    }

    ecs::SpriteResourceDescriptor desc;
    desc.resourceId = resourceId;

    desc.framePaths = {
        "picture/Projectile/Demon_Scythe_(projectile)1.png",
        "picture/Projectile/Demon_Scythe_(projectile)2.png",
        "picture/Projectile/Demon_Scythe_(projectile)3.png",
        "picture/Projectile/Demon_Scythe_(projectile)4.png",
        "picture/Projectile/Demon_Scythe_(projectile)5.png",
        "picture/Projectile/Demon_Scythe_(projectile)6.png",
        "picture/Projectile/Demon_Scythe_(projectile)7.png",
        "picture/Projectile/Demon_Scythe_(projectile)8.png",
    };

    ecs::SpriteManager::getInstance().registerResource(desc);
    _registeredProjectileResources.insert(resourceId);
  }

  void fireScythe(entt::entity owner, DemonAttackComponent &attack,
                  TransformComponent &ownerTransform, SpriteStateComponent &ownerState,
                  const cocos2d::Vec2 &targetPos) {
    cocos2d::Node *parentNode = nullptr;
    if (ownerState.spriteCreated && ownerState.spriteHandle) {
      auto *ownerSprite = static_cast<cocos2d::Sprite *>(ownerState.spriteHandle);
      parentNode = ownerSprite ? ownerSprite->getParent() : nullptr;
    }
    if (!parentNode) {
      return;
    }

    ensureScytheResourceRegistered();
    const std::string resourceId = "projectile_picture/Projectile/Demon_Scythe_(projectile)";

    cocos2d::Vec2 toTarget = targetPos - ownerTransform.position;
    if (toTarget.isZero()) {
      return;
    }

    float baseAngle = atan2(toTarget.y, toTarget.x);

    int idx = attack.shotsFired;
    float offsetDeg = 0.0f;
    if (attack.shotsPerVolley > 1) {
      float start = -15.0f;
      float end = 15.0f;
      float t = (attack.shotsPerVolley == 1) ? 0.5f : (float)idx / (float)(attack.shotsPerVolley - 1);
      offsetDeg = start + (end - start) * t;
    }

    float desiredAngle = baseAngle + offsetDeg * (float)M_PI / 180.0f;
    float fireAngle = desiredAngle;

    cocos2d::Vec2 dir(cos(fireAngle), sin(fireAngle));
    if (dir.isZero()) {
      return;
    }
    dir.normalize();

    auto &poolManager = EntityPoolManager::getInstance();
    entt::entity projectile = poolManager.acquireProjectile(*_registry);

    auto *projTransform = _registry->try_get<TransformComponent>(projectile);
    if (projTransform) {
      projTransform->position = ownerTransform.position;
      projTransform->previousPosition = ownerTransform.position;
      projTransform->velocity = cocos2d::Vec2::ZERO;
    } else {
      auto &t = _registry->emplace<TransformComponent>(projectile);
      t.position = ownerTransform.position;
      t.previousPosition = ownerTransform.position;
    }

    auto *projComp = _registry->try_get<ProjectileComponent>(projectile);
    if (projComp) {
      projComp->owner = entt::to_integral(owner);
      projComp->damage = attack.projectileDamage;
      projComp->lifetime = attack.projectileLifetime;
      projComp->hasHit = false;
    } else {
      auto &p = _registry->emplace<ProjectileComponent>(projectile);
      p.owner = entt::to_integral(owner);
      p.damage = attack.projectileDamage;
      p.lifetime = attack.projectileLifetime;
    }

    auto *projRender = _registry->try_get<RenderComponent>(projectile);
    if (projRender) {
      projRender->spriteResourceId = resourceId;
      projRender->scale = 1.0f;
      projRender->zOrder = 2;
      projRender->visible = true;
      projRender->rotation = 0.0f;
      projRender->flipX = false;
      projRender->flipY = false;
      projRender->enableSync = true;
    } else {
      auto &r = _registry->emplace<RenderComponent>(projectile);
      r.spriteResourceId = resourceId;
      r.scale = 1.0f;
      r.zOrder = 2;
      r.visible = true;
    }

    auto *projParent = _registry->try_get<ParentNodeComponent>(projectile);
    if (projParent) {
      projParent->parentNode = parentNode;
      projParent->attachedToParent = false;
    } else {
      auto &p = _registry->emplace<ParentNodeComponent>(projectile);
      p.parentNode = parentNode;
      p.attachedToParent = false;
    }

    bool hasSpriteState = _registry->all_of<SpriteStateComponent>(projectile);
    if (hasSpriteState) {
      auto *projState = _registry->try_get<SpriteStateComponent>(projectile);
      if (projState && projState->spriteCreated && projState->spriteHandle) {
        auto *s = static_cast<cocos2d::Sprite *>(projState->spriteHandle);
        if (s && s->getParent()) {
          s->setVisible(true);
          s->setPosition(ownerTransform.position);

          auto *b = s->getPhysicsBody();
          if (b) {
            b->setEnabled(true);
            b->setVelocity(cocos2d::Vec2::ZERO);
          } else {
            _registry->remove<SpriteStateComponent>(projectile);
            s->removeFromParent();
          }
        } else {
          _registry->remove<SpriteStateComponent>(projectile);
        }
      } else {
        _registry->remove<SpriteStateComponent>(projectile);
      }
    }

    auto *projAnim = _registry->try_get<AnimationComponent>(projectile);
    if (projAnim) {
      projAnim->animationSetId = resourceId;
      projAnim->frameTime = 0.08f;
      projAnim->frameSequence = {1, 2, 3, 4, 5, 6, 7, 8};
      projAnim->isPlaying = true;
      projAnim->loop = true;
      projAnim->reset();
    } else {
      auto &a = _registry->emplace<AnimationComponent>(projectile);
      a.animationSetId = resourceId;
      a.frameTime = 0.08f;
      a.frameSequence = {1, 2, 3, 4, 5, 6, 7, 8};
      a.isPlaying = true;
      a.loop = true;
    }

    auto *projPhysics = _registry->try_get<PhysicsBodyComponent>(projectile);
    if (projPhysics) {
      projPhysics->shape = PhysicsBodyComponent::BodyShape::Circle;
      projPhysics->radius = 14.0f;
      projPhysics->density = 0.1f;
      projPhysics->restitution = 0.0f;
      projPhysics->friction = 0.0f;
      projPhysics->dynamic = true;
      projPhysics->rotationEnabled = false;
      projPhysics->gravityEnabled = false;
      projPhysics->contactTestBitmask = 0xFFFFFFFF;
      projPhysics->collisionBitmask = 0x0005;
      projPhysics->categoryBitmask = 0x0004;
      projPhysics->group = -2;
      projPhysics->needsCreation = true;
    } else {
      auto &p = _registry->emplace<PhysicsBodyComponent>(projectile);
      p.shape = PhysicsBodyComponent::BodyShape::Circle;
      p.radius = 14.0f;
      p.density = 0.1f;
      p.restitution = 0.0f;
      p.friction = 0.0f;
      p.dynamic = true;
      p.rotationEnabled = false;
      p.gravityEnabled = false;
      p.contactTestBitmask = 0xFFFFFFFF;
      p.collisionBitmask = 0x0005;
      p.categoryBitmask = 0x0004;
      p.group = -2;
    }

    auto *initialVel = _registry->try_get<InitialVelocityComponent>(projectile);
    if (initialVel) {
      initialVel->velocity = cocos2d::Vec2::ZERO;
      initialVel->applied = false;
    } else {
      auto &iv = _registry->emplace<InitialVelocityComponent>(projectile);
      iv.velocity = cocos2d::Vec2::ZERO;
      iv.applied = false;
    }

    auto *delayed = _registry->try_get<DelayedAccelerationComponent>(projectile);
    if (delayed) {
      delayed->delayRemaining = attack.projectileDelay;
      delayed->currentSpeed = 0.0f;
      delayed->maxSpeed = attack.projectileMaxSpeed;
      delayed->acceleration = attack.projectileAcceleration;
      delayed->direction = dir;
    } else {
      auto &d = _registry->emplace<DelayedAccelerationComponent>(projectile);
      d.delayRemaining = attack.projectileDelay;
      d.currentSpeed = 0.0f;
      d.maxSpeed = attack.projectileMaxSpeed;
      d.acceleration = attack.projectileAcceleration;
      d.direction = dir;
    }

    if (!_registry->any_of<NoVelocityRotationTag>(projectile)) {
      _registry->emplace<NoVelocityRotationTag>(projectile);
    }
  }
};

} // namespace ecs

#endif
