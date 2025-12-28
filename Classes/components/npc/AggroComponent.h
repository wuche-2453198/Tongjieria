#ifndef __ECS_COMPONENT_AGGROCOMPONENT_H__
#define __ECS_COMPONENT_AGGROCOMPONENT_H__

#include "components/Entity.h"
#include "cocos2d.h"
#include <string>

namespace ecs {

/**
 * @brief 仇恨系统组件 - 管理索敌和仇恨逻辑
 *
 * 功能:
 * - 目标检测与追踪
 * - 仇恨状态切换
 * - 距离计算
 */
struct AggroComponent
{
  EntityId targetEntity = INVALID_ENTITY; // 当前目标实体

  // 仇恨距离配置
  float aggroRange = 500.0f;   // 进入仇恨距离
  float deaggroRange = 600.0f; // 脱离仇恨距离

  // 仇恨状态
  bool hasAggro = false;                                 // 是否有仇恨目标
  float distanceToTarget = 99999.0f;                     // 与目标距离
  cocos2d::Vec2 directionToTarget = cocos2d::Vec2::ZERO; // 指向目标方向

  // 目标标签（用于查找目标）
  std::string targetTag = "Player"; // 默认索敌玩家

  AggroComponent() = default;
  AggroComponent(float aggro, float deaggro)
      : aggroRange(aggro), deaggroRange(deaggro) {}

  bool shouldEnterAggro() const
  {
    return !hasAggro && distanceToTarget <= aggroRange;
  }

  bool shouldExitAggro() const
  {
    return hasAggro && distanceToTarget > deaggroRange;
  }
};

} // namespace ecs

#endif // __ECS_COMPONENT_AGGROCOMPONENT_H__
