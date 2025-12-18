#ifndef __ECS_ENTITY_H__
#define __ECS_ENTITY_H__

#include <cstdint>
#include <limits>

namespace ecs {

/**
 * @brief 实体ID类型
 *
 * 使用uint32_t与EnTT的entt::entity兼容
 */
using EntityId = uint32_t;

/**
 * @brief 无效实体ID常量
 */
constexpr EntityId INVALID_ENTITY = std::numeric_limits<EntityId>::max();

} // namespace ecs

#endif // __ECS_ENTITY_H__
