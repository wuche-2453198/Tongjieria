#ifndef __ECS_SYSTEM_OPTIMIZEDAISYSTEMBASE_H__
#define __ECS_SYSTEM_OPTIMIZEDAISYSTEMBASE_H__

#include "systems/core/ISystemEntt.h"
#include "systems/core/SystemPriority.h"
#include "components/core/EntityStateFlags.h"
#include "components/core/TransformComponent.h"
#include "cocos2d.h"

namespace ecs {

/**
 * @brief 优化的 AI 系统基类 - 提供基于状态的更新频率优化
 * 
 * 该基类实现了以下优化策略：
 * - 离屏降频：离屏实体每 N 帧更新一次
 * - 空闲降频：空闲实体每 M 帧更新一次
 * - LOD（细节层次）：远距离实体使用简化 AI
 * 
 * 子类需要实现：
 * - updateEntity(): 完整的 AI 更新逻辑
 * - updateEntitySimplified(): 简化的 AI 更新逻辑（可选）
 * 
 * Requirements: 5.1, 5.5, 5.6
 */
class OptimizedAISystemBase : public ISystemEntt {
public:
    virtual ~OptimizedAISystemBase() = default;

    // ========== 配置方法 ==========

    /**
     * @brief 设置离屏更新间隔
     * @param frames 离屏实体每隔多少帧更新一次（默认5帧）
     */
    void setOffScreenUpdateInterval(int frames) { 
        _offScreenInterval = frames > 0 ? frames : 1; 
    }

    /**
     * @brief 设置空闲更新间隔
     * @param frames 空闲实体每隔多少帧更新一次（默认3帧）
     */
    void setIdleUpdateInterval(int frames) { 
        _idleInterval = frames > 0 ? frames : 1; 
    }

    /**
     * @brief 设置 LOD 距离阈值
     * @param distance 超过此距离使用简化 AI（默认800像素）
     */
    void setLODDistance(float distance) { 
        _lodDistance = distance > 0 ? distance : 100.0f; 
    }

    /**
     * @brief 启用/禁用优化
     * @param enabled 是否启用优化（默认启用）
     */
    void setOptimizationEnabled(bool enabled) {
        _optimizationEnabled = enabled;
    }

    /**
     * @brief 获取当前帧计数器
     */
    int getFrameCounter() const { return _frameCounter; }

protected:
    // ========== 优化参数 ==========
    int _offScreenInterval = 5;      // 离屏实体更新间隔（帧）
    int _idleInterval = 3;           // 空闲实体更新间隔（帧）
    float _lodDistance = 800.0f;     // LOD 距离阈值（像素）
    bool _optimizationEnabled = true; // 是否启用优化
    int _frameCounter = 0;           // 帧计数器

    /**
     * @brief 判断是否应该更新指定实体
     * 
     * 基于实体的状态标记决定是否在当前帧更新：
     * - 非激活实体：不更新
     * - 离屏实体：每 _offScreenInterval 帧更新一次
     * - 空闲实体：每 _idleInterval 帧更新一次
     * - 其他实体：每帧更新
     * 
     * @param entity 实体ID
     * @param stateFlags 实体状态标记（可为nullptr）
     * @return true 应该更新，false 跳过更新
     */
    bool shouldUpdateEntity(entt::entity entity, const EntityStateFlags* stateFlags) const {
        // 如果优化被禁用，始终更新
        if (!_optimizationEnabled) {
            return true;
        }

        // 如果没有状态标记组件，始终更新
        if (!stateFlags) {
            return true;
        }

        // 非激活实体不更新
        if (!stateFlags->isActive) {
            return false;
        }

        // 离屏实体降频更新
        if (!stateFlags->isOnScreen) {
            return (_frameCounter % _offScreenInterval) == 0;
        }

        // 空闲实体降频更新
        if (stateFlags->isIdle) {
            return (_frameCounter % _idleInterval) == 0;
        }

        // 其他情况每帧更新
        return true;
    }

    /**
     * @brief 判断是否应该使用简化 AI
     * 
     * 基于实体到玩家的距离决定是否使用简化 AI：
     * - 距离超过 _lodDistance：使用简化 AI
     * - 距离在阈值内：使用完整 AI
     * 
     * @param stateFlags 实体状态标记（可为nullptr）
     * @return true 使用简化 AI，false 使用完整 AI
     */
    bool shouldUseSimplifiedAI(const EntityStateFlags* stateFlags) const {
        // 如果优化被禁用，不使用简化 AI
        if (!_optimizationEnabled) {
            return false;
        }

        // 如果没有状态标记组件，不使用简化 AI
        if (!stateFlags) {
            return false;
        }

        // 距离超过阈值时使用简化 AI
        return stateFlags->distanceToPlayer > _lodDistance;
    }

    /**
     * @brief 更新帧计数器（在 update 开始时调用）
     */
    void incrementFrameCounter() {
        _frameCounter++;
    }

    /**
     * @brief 更新实体的 lastUpdateFrame（在实际更新实体后调用）
     * @param stateFlags 实体状态标记
     */
    void markEntityUpdated(EntityStateFlags* stateFlags) {
        if (stateFlags) {
            stateFlags->lastUpdateFrame = _frameCounter;
        }
    }

    // ========== 子类需要实现的方法 ==========

    /**
     * @brief 完整的 AI 更新逻辑（子类必须实现）
     * @param entity 实体ID
     * @param delta 帧时间
     */
    // virtual void updateEntity(entt::entity entity, float delta) = 0;

    /**
     * @brief 简化的 AI 更新逻辑（子类可选实现）
     * 
     * 默认实现为空操作。子类可以重写此方法提供简化的 AI 行为，
     * 例如：只更新位置，不进行复杂的状态机转换。
     * 
     * @param entity 实体ID
     * @param delta 帧时间
     */
    virtual void updateEntitySimplified(entt::entity entity, float delta) {
        // 默认实现：空操作
        // 子类可以重写此方法提供简化的 AI 行为
    }
};

} // namespace ecs

#endif // __ECS_SYSTEM_OPTIMIZEDAISYSTEMBASE_H__
