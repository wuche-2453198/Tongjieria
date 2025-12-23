#ifndef __ECS_COMPONENT_ENEMYTAG_H__
#define __ECS_COMPONENT_ENEMYTAG_H__

#include <string>

namespace ecs {

/**
 * @brief 敌人标记组件
 */
struct EnemyTag
{
  std::string enemyType;
  EnemyTag() = default;
  EnemyTag(const std::string &type) : enemyType(type) {}
};

} // namespace ecs

#endif // __ECS_COMPONENT_ENEMYTAG_H__
