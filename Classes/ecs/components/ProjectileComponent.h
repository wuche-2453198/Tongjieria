#ifndef __ECS_COMPONENT_PROJECTILECOMPONENT_H__
#define __ECS_COMPONENT_PROJECTILECOMPONENT_H__

#include "../Entity.h"

namespace ecs {

/**
 * @brief 投射物组件 - 投射物实体本身
 */
struct ProjectileComponent
{
  EntityId owner = INVALID_ENTITY; // 发射者实体
  float damage = 8.0f;             // 伤害
  float lifetime = 3.0f;           // 剩余生命周期
  bool hasHit = false;             // 是否已命中

  // 减益效果 - 冰系（继承自发射者）
  float chillChance = 0.0f;
  float chillDuration = 0.0f;
  float chillSpeedReduction = 0.0f;
  float freezeChance = 0.0f;
  float freezeDuration = 0.0f;

  // 减益效果 - 毒系（继承自发射者）
  float poisonChance1 = 0.0f;
  float poisonDuration1 = 0.0f;
  float poisonDamage1 = 0.0f;
  float poisonChance2 = 0.0f;
  float poisonDuration2 = 0.0f;
  float poisonDamage2 = 0.0f;

  ProjectileComponent() = default;
  ProjectileComponent(EntityId ownerEntity, float dmg)
      : owner(ownerEntity), damage(dmg) {}
};

} // namespace ecs

#endif // __ECS_COMPONENT_PROJECTILECOMPONENT_H__
