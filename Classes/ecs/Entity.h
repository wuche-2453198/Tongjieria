#ifndef __ECS_ENTITY_H__
#define __ECS_ENTITY_H__

#include <cstdint>
#include <limits>

namespace ecs {

/**
 * @brief 实体ID类型 - 轻量级实体表示
 *
 * 实体只是一个唯一的ID，不包含任何数据或行为
 * 组件通过实体ID进行关联
 */
using EntityId = uint32_t;

/**
 * @brief 无效实体ID常量
 */
constexpr EntityId INVALID_ENTITY = std::numeric_limits<EntityId>::max();

/**
 * @brief 组件类型ID
 */
using ComponentTypeId = uint32_t;

/**
 * @brief 组件类型ID生成器
 *
 * 使用模板特化为每种组件类型生成唯一ID
 */
class ComponentTypeIdGenerator {
public:
  template <typename T> static ComponentTypeId getTypeId() {
    static ComponentTypeId id = nextId++;
    return id;
  }

  static ComponentTypeId getTypeCount() { return nextId; }

private:
  static inline ComponentTypeId nextId = 0;
};

/**
 * @brief 获取组件类型ID的便捷函数
 */
template <typename T> inline ComponentTypeId getComponentTypeId() {
  return ComponentTypeIdGenerator::getTypeId<T>();
}

} // namespace ecs

#endif // __ECS_ENTITY_H__
