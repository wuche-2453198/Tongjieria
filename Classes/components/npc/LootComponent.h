#ifndef __ECS_COMPONENT_LOOTCOMPONENT_H__
#define __ECS_COMPONENT_LOOTCOMPONENT_H__

#include <string>
#include <vector>

namespace ecs {

/**
 * @brief 掉落物组件
 */
struct LootComponent
{
  struct DropItem
  {
    std::string itemId;
    int minCount;
    int maxCount;
    float dropChance;
  };

  std::vector<DropItem> dropTable;

  void addDrop(const std::string &id, int minC, int maxC, float chance)
  {
    dropTable.push_back({id, minC, maxC, chance});
  }
};

} // namespace ecs

#endif // __ECS_COMPONENT_LOOTCOMPONENT_H__
