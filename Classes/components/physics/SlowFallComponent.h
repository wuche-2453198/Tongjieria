#ifndef __ECS_COMPONENT_SLOWFALLCOMPONENT_H__
#define __ECS_COMPONENT_SLOWFALLCOMPONENT_H__

namespace ecs {

/**
 * @brief 缓降组件 - 用于伞史莱姆等下落时有空气阻力的实体
 *
 * 当实体下落时，施加向上的阻力，减缓下落速度
 */
struct SlowFallComponent
{
  float maxFallSpeed = 100.0f;     // 最大下落速度（正值）
  float fallDamping = 0.85f;       // 下落阻尼系数 (0-1, 越小阻力越大)
  float horizontalDamping = 0.95f; // 水平阻尼系数 (下落时水平方向的空气阻力)
  bool isActive = true;            // 是否启用缓降

  SlowFallComponent() = default;
  SlowFallComponent(float maxSpeed, float fallDamp, float horzDamp = 0.95f)
      : maxFallSpeed(maxSpeed), fallDamping(fallDamp), horizontalDamping(horzDamp) {}
};

} // namespace ecs

#endif // __ECS_COMPONENT_SLOWFALLCOMPONENT_H__
