#ifndef __ECS_COMPONENT_POOLEDENTITY_H__
#define __ECS_COMPONENT_POOLEDENTITY_H__

#include <string>

namespace ecs {

/**
 * @brief 池化标记组件 - 标识实体属于对象池
 * 
 * 该组件用于标记实体是否来自对象池，以及其池类型。
 * 当实体被"销毁"时，实际上会被归还到池中而非真正销毁。
 * 
 * Requirements: 4.1, 4.2
 */
struct PooledEntity
{
    bool inUse = false;           // 是否正在使用（false 表示在池中等待复用）
    std::string poolType;         // 池类型（如 "projectile", "particle" 等）

    PooledEntity() = default;
    PooledEntity(const std::string& type) : inUse(true), poolType(type) {}
};

} // namespace ecs

#endif // __ECS_COMPONENT_POOLEDENTITY_H__
