#ifndef __ECS_SYSTEM_PRIORITY_H__
#define __ECS_SYSTEM_PRIORITY_H__

namespace ecs {

/**
 * @brief 系统执行优先级（数值越小越先执行）
 */
namespace SystemPriority {
constexpr int INPUT = 0;
constexpr int PHYSICS = 100;
constexpr int COLLISION = 200;
constexpr int AI = 300;
constexpr int MOVEMENT = 400;
constexpr int ANIMATION = 500;
constexpr int CLEANUP = 600;
constexpr int RENDER = 600;
}

} // namespace ecs

#endif // __ECS_SYSTEM_PRIORITY_H__
