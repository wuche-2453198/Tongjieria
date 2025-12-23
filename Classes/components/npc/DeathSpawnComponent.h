#ifndef __ECS_COMPONENT_DEATHSPAWNCOMPONENT_H__
#define __ECS_COMPONENT_DEATHSPAWNCOMPONENT_H__

#include <string>

namespace ecs {

/**
 * @brief 死亡生成组件 - 死亡时生成其他实体
 */
struct DeathSpawnComponent
{
  std::string spawnType;
  int minCount = 1;
  int maxCount = 3;
  float spawnRadius = 30.0f;

  DeathSpawnComponent() = default;
  DeathSpawnComponent(const std::string &type, int min, int max)
      : spawnType(type), minCount(min), maxCount(max) {}
};

} // namespace ecs

#endif // __ECS_COMPONENT_DEATHSPAWNCOMPONENT_H__
