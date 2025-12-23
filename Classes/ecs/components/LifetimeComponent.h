#ifndef __ECS_COMPONENT_LIFETIMECOMPONENT_H__
#define __ECS_COMPONENT_LIFETIMECOMPONENT_H__

#include "../Entity.h"
#include <functional>

namespace ecs {

/**
 * @brief 生命周期组件 - 自动销毁
 */
struct LifetimeComponent
{
  float lifetime = 1.0f;
  float elapsed = 0.0f;
  std::function<void(EntityId)> onExpire;

  LifetimeComponent() = default;
  LifetimeComponent(float time) : lifetime(time) {}

  bool isExpired() const { return elapsed >= lifetime; }
};

} // namespace ecs

#endif // __ECS_COMPONENT_LIFETIMECOMPONENT_H__
