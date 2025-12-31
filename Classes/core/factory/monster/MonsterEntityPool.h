#ifndef __ECS_MONSTER_ENTITY_POOL_H__
#define __ECS_MONSTER_ENTITY_POOL_H__

#include <entt/entt.hpp>
#include <queue>
#include <string>
#include <unordered_map>
#include "components/Entity.h"

/**
 * @class MonsterEntityPool
 * @brief 怪物实体池，负责预分配、复用、统计
 */
class MonsterEntityPool {
public:
  struct Statistics {
    size_t totalAllocated = 0;   // 累计创建的实体数量
    size_t inUse = 0;            // 当前被借出的数量
    size_t available = 0;        // 当前池中可用数量
  };

  static MonsterEntityPool &getInstance() {
    static MonsterEntityPool instance;
    return instance;
  }

  MonsterEntityPool(const MonsterEntityPool &) = delete;
  MonsterEntityPool &operator=(const MonsterEntityPool &) = delete;

  /**
   * @brief 预分配实体
   * @param registry EnTT 注册表
   * @param count 预分配数量
   */
  void preallocate(entt::registry &registry, size_t count);

  /**
   * @brief 获取实体（池空时动态创建）
   */
  ecs::EntityId acquire(entt::registry &registry);

  /**
   * @brief 归还实体到池
   */
  void release(entt::registry &registry, ecs::EntityId id);

  /**
   * @brief 获取池统计信息
   */
  Statistics getStatistics() const;

  /**
   * @brief 重置实体（清理组件，准备复用）
   */
  void resetEntity(entt::registry &registry, ecs::EntityId id);

  /**
   * @brief 激活实体（标记在用，可在此重新附加必要组件）
   */
  void activateEntity(entt::registry &registry, ecs::EntityId id);

  /**
   * @brief 停用实体（从场景中移除/停用）
   */
  void deactivateEntity(entt::registry &registry, ecs::EntityId id);

private:
  MonsterEntityPool() = default;

  std::queue<ecs::EntityId> _free;
  Statistics _stats;
};

#endif // __ECS_MONSTER_ENTITY_POOL_H__
