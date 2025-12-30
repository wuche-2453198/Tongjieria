#ifndef __ECS_SYSTEM_ENTITYDESTRUCTIONMANAGER_H__
#define __ECS_SYSTEM_ENTITYDESTRUCTIONMANAGER_H__

#include <entt/entt.hpp>
#include <unordered_set>
#include "components/Entity.h"

namespace ecs {

/**
 * @brief 实体延迟销毁管理器 - 管理实体的延迟销毁
 * 
 * 单例模式，负责在系统更新周期结束后统一处理实体销毁，
 * 避免在系统遍历期间访问无效实体。
 * 
 * Requirements: 1.1, 1.2, 1.3, 1.4, 1.5
 */
class EntityDestructionManager {
public:
    /**
     * @brief 获取单例实例
     */
    static EntityDestructionManager& getInstance() {
        static EntityDestructionManager instance;
        return instance;
    }

    // 禁止拷贝和移动
    EntityDestructionManager(const EntityDestructionManager&) = delete;
    EntityDestructionManager& operator=(const EntityDestructionManager&) = delete;
    EntityDestructionManager(EntityDestructionManager&&) = delete;
    EntityDestructionManager& operator=(EntityDestructionManager&&) = delete;

    /**
     * @brief 将实体加入销毁队列
     * @param registry EnTT 注册表
     * @param entity 要销毁的实体
     * 
     * 实体会被加入队列，在 processQueue() 调用时统一销毁。
     * 重复添加同一实体会被自动去重（unordered_set）。
     * 
     * Requirements: 1.1, 1.3
     */
    void queueDestruction(entt::registry& registry, entt::entity entity);

    /**
     * @brief 检查实体是否待销毁
     * @param entity 要检查的实体
     * @return 如果实体在销毁队列中返回 true
     * 
     * Requirements: 1.5
     */
    bool isPendingDestruction(entt::entity entity) const;

    /**
     * @brief 处理销毁队列（在所有系统更新后调用）
     * @param registry EnTT 注册表
     * 
     * 遍历队列中的所有实体，验证有效性后销毁。
     * 处理完成后清空队列。
     * 
     * Requirements: 1.2, 1.4
     */
    void processQueue(entt::registry& registry);

    /**
     * @brief 清空队列（场景切换时调用）
     * 
     * 不销毁实体，仅清空队列。
     */
    void clear();

    /**
     * @brief 获取当前队列大小
     * @return 队列中待销毁的实体数量
     */
    size_t getQueueSize() const { return _destructionQueue.size(); }

private:
    EntityDestructionManager() = default;
    ~EntityDestructionManager() = default;

    // 销毁队列 - 使用 unordered_set 自动去重
    std::unordered_set<entt::entity> _destructionQueue;
};

/**
 * @brief 安全获取实体组件的辅助函数
 * @tparam T 组件类型
 * @param registry EnTT 注册表
 * @param entityId 实体ID
 * @return 组件指针，如果实体无效或待销毁则返回 nullptr
 * 
 * Requirements: 2.1, 2.2, 2.3
 */
template<typename T>
T* safeGetComponent(entt::registry& registry, EntityId entityId) {
    if (entityId == INVALID_ENTITY) return nullptr;
    
    auto entity = static_cast<entt::entity>(entityId);
    if (!registry.valid(entity)) return nullptr;
    if (EntityDestructionManager::getInstance().isPendingDestruction(entity)) return nullptr;
    
    return registry.try_get<T>(entity);
}

/**
 * @brief 验证实体引用是否有效
 * @param registry EnTT 注册表
 * @param entityId 实体ID
 * @return 如果实体有效且不在销毁队列中返回 true
 * 
 * Requirements: 2.1, 2.2, 2.3
 */
inline bool isEntityReferenceValid(entt::registry& registry, EntityId entityId) {
    if (entityId == INVALID_ENTITY) return false;
    
    auto entity = static_cast<entt::entity>(entityId);
    if (!registry.valid(entity)) return false;
    if (EntityDestructionManager::getInstance().isPendingDestruction(entity)) return false;
    
    return true;
}

} // namespace ecs

#endif // __ECS_SYSTEM_ENTITYDESTRUCTIONMANAGER_H__
