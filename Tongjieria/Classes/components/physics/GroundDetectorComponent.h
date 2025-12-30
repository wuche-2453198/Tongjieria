#ifndef __ECS_COMPONENT_GROUNDDETECTORCOMPONENT_H__
#define __ECS_COMPONENT_GROUNDDETECTORCOMPONENT_H__

#include <cstdint>
#include <unordered_set>

namespace ecs {

/**
 * @brief 地面检测组件 - 检测是否在地面/静止
 */
struct GroundDetectorComponent
{
  bool isOnGround = false;      // 是否在地面上
  bool isStill = false;         // 是否静止
  float stillThreshold = 10.0f; // 静止判断速度阈值
  int groundContactCount = 0;   // 地面接触计数（用于处理同时接触多个地面）

  std::unordered_set<std::uintptr_t> groundContactKeys;

  GroundDetectorComponent() = default;

  bool isGroundedAndStill() const { return isOnGround && isStill; }
};

} // namespace ecs

#endif // __ECS_COMPONENT_GROUNDDETECTORCOMPONENT_H__
