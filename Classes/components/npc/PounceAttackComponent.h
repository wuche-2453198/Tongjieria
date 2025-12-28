#ifndef __ECS_COMPONENT_POUNCEATTACKCOMPONENT_H__
#define __ECS_COMPONENT_POUNCEATTACKCOMPONENT_H__

namespace ecs {

struct PounceAttackComponent {
  float triggerDistance = 140.0f;
  float aboveTargetHeight = 30.0f;
  float horizontalSpeedMultiplier = 2.0f;
  float verticalImpulse = 520.0f;
  float cooldown = 1.0f;
  float cooldownTimer = 0.0f;
};

} // namespace ecs

#endif // __ECS_COMPONENT_POUNCEATTACKCOMPONENT_H__
