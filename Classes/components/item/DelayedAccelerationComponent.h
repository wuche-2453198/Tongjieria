#ifndef __ECS_COMPONENT_DELAYEDACCELERATIONCOMPONENT_H__
#define __ECS_COMPONENT_DELAYEDACCELERATIONCOMPONENT_H__

#include "cocos2d.h"

namespace ecs {

struct DelayedAccelerationComponent {
  float delayRemaining = 0.0f;
  float currentSpeed = 0.0f;
  float maxSpeed = 0.0f;
  float acceleration = 0.0f;
  cocos2d::Vec2 direction = cocos2d::Vec2::ZERO;
};

}

#endif
