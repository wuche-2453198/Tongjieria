#ifndef __ECS_COMPONENT_DEMONATTACKCOMPONENT_H__
#define __ECS_COMPONENT_DEMONATTACKCOMPONENT_H__

namespace ecs {

struct DemonAttackComponent {
  float volleyCooldown = 12.0f;
  float cooldownTimer = 0.0f;

  int shotsPerVolley = 4;
  float shotInterval = 0.2f;

  bool inVolley = false;
  int shotsFired = 0;
  float shotTimer = 0.0f;

  float fireRange = 900.0f;

  float projectileDamage = 14.0f;
  float projectileLifetime = 6.0f;
  float projectileMaxSpeed = 550.0f;
  float projectileAcceleration = 1200.0f;
  float projectileDelay = 2.0f;
};

}

#endif
