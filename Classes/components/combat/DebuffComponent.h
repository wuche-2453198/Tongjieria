#ifndef __ECS_COMPONENT_DEBUFFCOMPONENT_H__
#define __ECS_COMPONENT_DEBUFFCOMPONENT_H__

namespace ecs {

/**
 * @brief 减益效果组件 - 附加在受影响的实体上
 */
struct DebuffComponent
{
  // 冷冻效果 (Chill) - 减速
  bool hasChillDebuff = false;
  float chillTimer = 0.0f;
  float chillDuration = 0.0f;
  float chillSpeedReduction = 0.0f; // 0.4 = 减速40%

  // 冰冻效果 (Freeze) - 完全无法移动
  bool hasFreezeDebuff = false;
  float freezeTimer = 0.0f;
  float freezeDuration = 0.0f;

  // 中毒效果 (Poison) - 持续伤害
  bool hasPoisonDebuff = false;
  float poisonTimer = 0.0f;
  float poisonDuration = 0.0f;
  float poisonDamagePerSecond = 0.0f; // 每秒伤害
  float poisonTickTimer = 0.0f;       // 伤害计时器

  DebuffComponent() = default;

  void applyChillDebuff(float duration, float speedReduction)
  {
    hasChillDebuff = true;
    chillTimer = 0.0f;
    chillDuration = duration;
    chillSpeedReduction = speedReduction;
  }

  void applyFreezeDebuff(float duration)
  {
    hasFreezeDebuff = true;
    freezeTimer = 0.0f;
    freezeDuration = duration;
  }

  void applyPoisonDebuff(float duration, float damagePerSecond)
  {
    // 如果新毒素更强或当前无毒素，则应用
    if (!hasPoisonDebuff || duration > poisonDuration - poisonTimer)
    {
      hasPoisonDebuff = true;
      poisonTimer = 0.0f;
      poisonDuration = duration;
      poisonDamagePerSecond = damagePerSecond;
      poisonTickTimer = 0.0f;
    }
  }

  bool isFrozen() const { return hasFreezeDebuff && freezeTimer < freezeDuration; }
  bool isChilled() const { return hasChillDebuff && chillTimer < chillDuration; }
  bool isPoisoned() const { return hasPoisonDebuff && poisonTimer < poisonDuration; }

  float getSpeedMultiplier() const
  {
    if (isFrozen())
      return 0.0f;
    if (isChilled())
      return 1.0f - chillSpeedReduction;
    return 1.0f;
  }
};

} // namespace ecs

#endif // __ECS_COMPONENT_DEBUFFCOMPONENT_H__
