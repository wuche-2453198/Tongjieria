#ifndef __ECS_COMPONENT_HEALTHCOMPONENT_H__
#define __ECS_COMPONENT_HEALTHCOMPONENT_H__

#include <algorithm>

namespace ecs {

/**
 * @brief 生命值组件
 */
struct HealthComponent
{
  float maxHealth = 100.0f;
  float currentHealth = 100.0f;
  bool isDead = false;
  float invincibleTime = 0.0f;  // 无敌时间
  float invincibleTimer = 0.0f; // 无敌计时器

  HealthComponent() = default;
  HealthComponent(float max) : maxHealth(max), currentHealth(max) {}

  void takeDamage(float damage)
  {
    if (invincibleTimer > 0)
      return;
    currentHealth = std::max(0.0f, currentHealth - damage);
    if (currentHealth <= 0)
    {
      isDead = true;
    }
  }

  void heal(float amount)
  {
    currentHealth = std::min(maxHealth, currentHealth + amount);
  }

  float getHealthPercent() const
  {
    return maxHealth > 0 ? currentHealth / maxHealth : 0.0f;
  }
};

} // namespace ecs

#endif // __ECS_COMPONENT_HEALTHCOMPONENT_H__
