#ifndef __ECS_COMPONENT_PROJECTILEATTACKCOMPONENT_H__
#define __ECS_COMPONENT_PROJECTILEATTACKCOMPONENT_H__

#include <string>

namespace ecs {

/**
 * @brief 投射物攻击组件 - 用于可发射投射物的实体
 *
 * 用于冰雪尖刺史莱姆等远程攻击的怪物
 */
struct ProjectileAttackComponent
{
  // 投射物配置
  std::string projectileSpritePath;     // 投射物贴图路径
  float projectileSpriteWidth = 10.0f;  // 投射物贴图宽度
  float projectileSpriteHeight = 20.0f; // 投射物贴图高度
  int projectileCount = 4;              // 每次发射数量
  float projectileDamage = 8.0f;        // 投射物伤害
  float projectileSpeed = 300.0f;       // 投射物速度
  float projectileLifetime = 3.0f;      // 投射物生命周期

  // 发射配置
  float fireInterval = 0.8f;      // 发射间隔
  float fireRange = 200.0f;       // 发射范围（目标在此范围内才发射）
  float horizontalSpread = 80.0f; // 水平分散角度
  float verticalImpulse = 350.0f; // 垂直冲量（抛物线高度）
  bool useGravity = true;         // 投射物是否受重力影响

  // 状态
  float fireTimer = 0.0f;     // 发射计时器
  bool canFire = true;        // 是否可以发射
  bool targetInRange = false; // 目标是否在发射范围内

  // 减益效果 - 冰系
  float chillChance = 0.0f;         // 冷冻几率
  float chillDuration = 0.0f;       // 冷冻持续时间
  float chillSpeedReduction = 0.0f; // 冷冻减速比例
  float freezeChance = 0.0f;        // 冰冻几率
  float freezeDuration = 0.0f;      // 冰冻持续时间

  // 减益效果 - 毒系
  float poisonChance1 = 0.0f;   // 毒素几率1 (长时间)
  float poisonDuration1 = 0.0f; // 毒素持续时间1
  float poisonDamage1 = 0.0f;   // 毒素每秒伤害1
  float poisonChance2 = 0.0f;   // 毒素几率2 (短时间)
  float poisonDuration2 = 0.0f; // 毒素持续时间2
  float poisonDamage2 = 0.0f;   // 毒素每秒伤害2

  ProjectileAttackComponent() = default;
};

} // namespace ecs

#endif // __ECS_COMPONENT_PROJECTILEATTACKCOMPONENT_H__
