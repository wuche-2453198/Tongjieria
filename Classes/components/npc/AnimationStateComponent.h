#ifndef __ECS_COMPONENT_ANIMATIONSTATECOMPONENT_H__
#define __ECS_COMPONENT_ANIMATIONSTATECOMPONENT_H__

#include "cocos2d.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace ecs {

/**
 * @brief 动画状态数据 - 单个动画状态的配置
 */
struct AnimationStateData {
    std::string animationSetId;        // 动画集ID
    std::vector<int> frameSequence;    // 帧序列
    float frameTime = 0.15f;           // 每帧时间
    bool loop = true;                  // 是否循环
    int priority = 0;                  // 优先级（高优先级动画可打断低优先级）
    
    AnimationStateData() = default;
    AnimationStateData(const std::string& animId, const std::vector<int>& frames, 
                      float time = 0.15f, bool looping = true, int prio = 0)
        : animationSetId(animId), frameSequence(frames), frameTime(time), 
          loop(looping), priority(prio) {}
};

/**
 * @brief 动画状态组件 - 支持AI状态驱动的动画切换
 * 
 * 设计原则：
 * - 将AI状态映射到动画状态
 * - 支持动画优先级和打断机制
 * - 支持从JSON配置加载动画数据
 * - 与AnimationComponent配合使用
 * 
 * 使用方式：
 * 1. 通过JSON或代码配置状态->动画映射
 * 2. AI系统更新currentState
 * 3. AnimationSystem检测状态变化并切换动画
 */
struct AnimationStateComponent {
    // 状态名称 -> 动画数据的映射
    std::unordered_map<std::string, AnimationStateData> stateAnimations;
    
    // 当前状态名称
    std::string currentState = "idle";
    
    // 上一次的状态（用于检测状态变化）
    std::string previousState = "";
    
    // 默认状态（当找不到对应动画时使用）
    std::string defaultState = "idle";
    
    // 是否启用状态驱动动画
    bool enableStateDriven = true;
    
    // 当前播放动画的优先级
    int currentPriority = 0;
    
    AnimationStateComponent() = default;
    
    /**
     * @brief 添加状态动画映射
     */
    void addStateAnimation(const std::string& stateName, const AnimationStateData& animData) {
        stateAnimations[stateName] = animData;
    }
    
    /**
     * @brief 设置当前状态
     * @param stateName 状态名称
     * @param force 是否强制切换（忽略优先级）
     * @return 是否成功切换状态
     */
    bool setState(const std::string& stateName, bool force = false) {
        // 使用局部变量来处理状态名称
        std::string targetState = stateName;
        
        // 检查状态是否存在
        if (stateAnimations.find(targetState) == stateAnimations.end()) {
            CCLOG("AnimationStateComponent: State '%s' not found, using default", targetState.c_str());
            if (stateAnimations.find(defaultState) != stateAnimations.end()) {
                targetState = defaultState;
            } else {
                return false;
            }
        }
        
        // 检查优先级（除非强制切换）
        if (!force) {
            const auto& newAnimData = stateAnimations[targetState];
            if (newAnimData.priority < currentPriority) {
                return false; // 优先级不足，不切换
            }
        }
        
        previousState = currentState;
        currentState = targetState;
        return true;
    }
    
    /**
     * @brief 获取当前状态的动画数据
     */
    const AnimationStateData* getCurrentAnimationData() const {
        auto it = stateAnimations.find(currentState);
        if (it != stateAnimations.end()) {
            return &it->second;
        }
        
        // 尝试返回默认状态
        it = stateAnimations.find(defaultState);
        if (it != stateAnimations.end()) {
            return &it->second;
        }
        
        return nullptr;
    }
    
    /**
     * @brief 检查状态是否发生变化
     */
    bool hasStateChanged() const {
        return currentState != previousState;
    }
};

} // namespace ecs

#endif // __ECS_COMPONENT_ANIMATIONSTATECOMPONENT_H__
