#ifndef __ECS_COMPONENT_COMBATCOMPONENT_H__
#define __ECS_COMPONENT_COMBATCOMPONENT_H__

namespace ecs {

/**
 * @brief 战斗组件 - 攻击参数
 */
struct CombatComponent
{
  float attackRange = 50.0f;   // 攻击范围
  float attackDamage = 10.0f;  // 攻击伤害
  float attackCooldown = 1.0f; // 攻击冷却
  float attackTimer = 0.0f;    // 攻击计时器
  bool canAttack = true;       // 是否可以攻击

  CombatComponent() = default;
  CombatComponent(float range, float damage, float cooldown)
      : attackRange(range), attackDamage(damage), attackCooldown(cooldown) {}
};

} // namespace ecs

#endif // __ECS_COMPONENT_COMBATCOMPONENT_H__
