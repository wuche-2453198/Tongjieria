#ifndef __ECS_SYSTEM_ANIMATIONSTATEHELPER_H__
#define __ECS_SYSTEM_ANIMATIONSTATEHELPER_H__

#include "../components/AnimationStateComponent.h"
#include "../components/DemonEyeMovementComponent.h"
#include "../components/AntlionMovementComponent.h"
#include "../components/KingSlimeComponent.h"
#include <string>

namespace ecs {

/**
 * @brief 动画状态辅助工具 - 简化AI系统与动画状态的集成
 * 
 * 提供便捷的状态映射函数，将AI状态枚举转换为动画状态字符串
 */
class AnimationStateHelper {
public:
    /**
     * @brief 将DemonEye AI状态转换为动画状态名称
     */
    static std::string demonEyeStateToAnimationState(DemonEyeMovementComponent::State aiState) {
        switch (aiState) {
            case DemonEyeMovementComponent::HOVERING:
                return "hovering";
            case DemonEyeMovementComponent::DASHING:
                return "dashing";
            default:
                return "idle";
        }
    }
    
    /**
     * @brief 将Antlion AI状态转换为动画状态名称
     */
    static std::string antlionStateToAnimationState(AntlionMovementComponent::AIState aiState) {
        switch (aiState) {
            case AntlionMovementComponent::IDLE:
                return "idle";
            case AntlionMovementComponent::TRACKING:
                return "tracking";
            case AntlionMovementComponent::SHOOTING:
                return "shooting";
            default:
                return "idle";
        }
    }
    
    /**
     * @brief 将KingSlime AI状态转换为动画状态名称
     */
    static std::string kingSlimeStateToAnimationState(KingSlimeComponent::AIState aiState) {
        switch (aiState) {
            case KingSlimeComponent::IDLE:
                return "idle";
            case KingSlimeComponent::JUMPING:
                return "jumping";
            case KingSlimeComponent::TELEPORTING_OUT:
                return "teleport_out";
            case KingSlimeComponent::TELEPORTING_IN:
                return "teleport_in";
            default:
                return "idle";
        }
    }
    
    /**
     * @brief 通用状态更新函数 - 更新AnimationStateComponent的当前状态
     * @param animState 动画状态组件
     * @param newStateName 新的状态名称
     * @param force 是否强制切换（忽略优先级）
     * @return 是否成功切换状态
     */
    static bool updateAnimationState(AnimationStateComponent& animState, 
                                    const std::string& newStateName, 
                                    bool force = false) {
        if (animState.currentState == newStateName) {
            return false; // 状态未改变
        }
        
        return animState.setState(newStateName, force);
    }
    
    /**
     * @brief 为实体设置动画状态（带AI状态自动映射）
     * 
     * 使用示例：
     * @code
     * // 在DemonEyeAISystem中
     * auto* animState = registry->try_get<AnimationStateComponent>(entity);
     * if (animState) {
     *     AnimationStateHelper::updateAnimationState(*animState, 
     *         AnimationStateHelper::demonEyeStateToAnimationState(demon.aiState));
     * }
     * @endcode
     */
};

} // namespace ecs

#endif // __ECS_SYSTEM_ANIMATIONSTATEHELPER_H__
