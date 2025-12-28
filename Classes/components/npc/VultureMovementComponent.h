#ifndef __ECS_COMPONENT_VULTUREMOVEMENTCOMPONENT_H__
#define __ECS_COMPONENT_VULTUREMOVEMENTCOMPONENT_H__

#include "cocos2d.h"
#include <cstdlib>

namespace ecs {

struct VultureMovementComponent {
  enum State {
    IDLE,
    HOVERING,
    DASHING
  };

  State aiState = IDLE;
  float aiTimer = 0.0f;

  bool activated = false;

  float flySpeed = 160.0f;
  float maxSpeed = 400.0f;
  float acceleration = 300.0f;

  float activationRange = 200.0f;
  float hoverDistance = 120.0f;
  float dashTriggerDistance = 80.0f;
  float dashReturnDistance = 120.0f;

  float dashDuration = 0.6f;
  float dashCooldown = 1.0f;
  float dashCooldownTimer = 0.0f;

  float wobbleAmplitude = 10.0f;
  float wobbleFrequency = 2.0f;
  float wobblePhase = 0.0f;

  cocos2d::Vec2 currentVelocity = cocos2d::Vec2::ZERO;

  float lastHealth = -1.0f;

  float debugLogTimer = 0.0f;
  int debugLogCount = 0;

  VultureMovementComponent() {
    wobblePhase = ((float)rand() / RAND_MAX) * 6.28318530718f;
  }
};

} // namespace ecs

#endif // __ECS_COMPONENT_VULTUREMOVEMENTCOMPONENT_H__
