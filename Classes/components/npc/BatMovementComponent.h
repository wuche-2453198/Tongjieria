#ifndef __ECS_COMPONENT_BATMOVEMENTCOMPONENT_H__
#define __ECS_COMPONENT_BATMOVEMENTCOMPONENT_H__

#include "cocos2d.h"
#include <cmath>
#include <cstdlib>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace ecs {

struct BatMovementComponent {
  float flySpeed = 170.0f;
  float maxSpeed = 360.0f;
  float acceleration = 240.0f;
  float turnRate = 2.4f;

  float wobbleAmplitude = 0.25f;
  float wobbleFrequency = 3.0f;
  float wobblePhase = 0.0f;

  float wanderChangeInterval = 0.8f;
  float wanderTimer = 0.0f;
  float wanderAngle = 0.0f;

  float chaseCooldown = 0.9f;
  float chaseCooldownTimer = 0.0f;
  float chaseDuration = 0.55f;
  float chaseTimer = 0.0f;
  bool isChasing = false;

  cocos2d::Vec2 currentVelocity = cocos2d::Vec2::ZERO;
  float currentAngle = 0.0f;
  float targetAngle = 0.0f;

  BatMovementComponent() {
    currentAngle = ((float)rand() / RAND_MAX) * 2.0f * (float)M_PI;
    targetAngle = currentAngle;
    wanderAngle = currentAngle;
    wobblePhase = ((float)rand() / RAND_MAX) * 2.0f * (float)M_PI;
    chaseCooldownTimer = ((float)rand() / RAND_MAX) * chaseCooldown;
  }

  static float angleDifference(float target, float current) {
    float diff = target - current;
    while (diff > (float)M_PI) diff -= 2.0f * (float)M_PI;
    while (diff < -(float)M_PI) diff += 2.0f * (float)M_PI;
    return diff;
  }

  float smoothTurn(float delta) {
    float diff = angleDifference(targetAngle, currentAngle);
    float maxTurn = turnRate * delta;

    if (std::abs(diff) <= maxTurn) {
      currentAngle = targetAngle;
    } else {
      currentAngle += (diff > 0 ? maxTurn : -maxTurn);
    }

    while (currentAngle > (float)M_PI) currentAngle -= 2.0f * (float)M_PI;
    while (currentAngle < -(float)M_PI) currentAngle += 2.0f * (float)M_PI;

    return currentAngle;
  }
};

} // namespace ecs

#endif // __ECS_COMPONENT_BATMOVEMENTCOMPONENT_H__
