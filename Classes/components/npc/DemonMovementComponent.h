#ifndef __ECS_COMPONENT_DEMONMOVEMENTCOMPONENT_H__
#define __ECS_COMPONENT_DEMONMOVEMENTCOMPONENT_H__

#include "cocos2d.h"
#include <cmath>
#include <cstdlib>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace ecs {

struct DemonMovementComponent {
  float flySpeed = 180.0f;
  float maxSpeed = 420.0f;
  float acceleration = 600.0f;
  float turnRate = 2.5f;

  cocos2d::Vec2 currentVelocity = cocos2d::Vec2::ZERO;
  float currentAngle = 0.0f;
  float targetAngle = 0.0f;

  DemonMovementComponent() {
    currentAngle = ((float)rand() / RAND_MAX) * 2.0f * (float)M_PI;
    targetAngle = currentAngle;
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

}

#endif
