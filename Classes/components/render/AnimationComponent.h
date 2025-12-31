#ifndef __ECS_COMPONENT_ANIMATIONCOMPONENT_H__
#define __ECS_COMPONENT_ANIMATIONCOMPONENT_H__

#include <string>
#include <vector>

namespace ecs {

/**
 * @brief 动画组件 - 纯数据帧动画（新解耦架构）
 * 
 * 设计原则：
 * - 不持有SpriteFrame对象
 * - 只记录动画播放状态
 * - 由AnimationSystem负责更新帧
 */
struct AnimationComponent {
    std::string animationSetId;        // 动画集ID（对应一组精灵帧）
    std::vector<int> frameSequence;    // 帧序列（1-indexed，如{1,2,3,2}）
    float frameTime = 0.15f;           // 每帧时间
    int currentFrameIndex = 0;         // 当前帧序列索引
    float frameTimer = 0.0f;           // 帧计时器
    bool isPlaying = true;             // 是否播放
    bool loop = true;                  // 是否循环
    
    AnimationComponent() = default;
    AnimationComponent(const std::string& animId, float time = 0.15f)
        : animationSetId(animId), frameTime(time) {}
    
    void reset() {
        currentFrameIndex = 0;
        frameTimer = 0.0f;
    }
    
    int getCurrentFrameNumber() const {
        if (frameSequence.empty()) return 0;
        return frameSequence[currentFrameIndex];
    }
};

} // namespace ecs

#endif // __ECS_COMPONENT_ANIMATIONCOMPONENT_H__
