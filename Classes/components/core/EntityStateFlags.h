#ifndef __ECS_COMPONENT_ENTITYSTATEFLAGS_H__
#define __ECS_COMPONENT_ENTITYSTATEFLAGS_H__

namespace ecs {

/**
 * @brief 实体状态标记组件 - 用于优化系统更新频率
 * 
 * 该组件存储实体的运行时状态信息，供 AI 系统、动画系统等
 * 根据状态决定是否需要更新或降低更新频率。
 * 
 * Requirements: 5.1, 5.5, 5.6
 */
struct EntityStateFlags
{
    bool isOnScreen = true;       // 是否在屏幕内（离屏实体可降低更新频率）
    bool isActive = true;         // 是否激活（非激活实体跳过更新）
    bool isIdle = false;          // 是否空闲（空闲实体可降低更新频率）
    float distanceToPlayer = 0.0f; // 到玩家的距离（用于 LOD 判断）
    int lastUpdateFrame = 0;      // 上次更新的帧号（用于降频更新）

    EntityStateFlags() = default;
};

} // namespace ecs

#endif // __ECS_COMPONENT_ENTITYSTATEFLAGS_H__
