#ifndef __ECS_COMPONENT_SHAREDANIMATIONCOMPONENT_H__
#define __ECS_COMPONENT_SHAREDANIMATIONCOMPONENT_H__

#include <string>

namespace ecs {

/**
 * @brief 共享动画组件 - 支持多实体共享同一动画资源
 * 
 * 设计原则：
 * - 使用 animationId 引用 AnimationCacheManager 中的缓存动画
 * - 每个实体维护独立的播放状态（帧索引、计时器）
 * - 动画资源本身由 AnimationCacheManager 管理和共享
 * 
 * 与 AnimationComponent 的区别：
 * - AnimationComponent: 存储完整的帧序列数据，适合自定义动画
 * - SharedAnimationComponent: 引用缓存的动画，适合大量相同动画的实体
 * 
 * 使用场景：
 * - 大量相同类型的敌人（如史莱姆群）
 * - 粒子效果
 * - 环境装饰物
 * 
 * Requirements: 6.1, 6.2
 */
struct SharedAnimationComponent {
    std::string animationId;       // 动画ID（用于从 AnimationCacheManager 获取）
    int currentFrameIndex = 0;     // 当前帧索引
    float frameTimer = 0.0f;       // 帧计时器
    bool isPlaying = true;         // 是否正在播放
    bool loop = true;              // 是否循环播放
    float playbackSpeed = 1.0f;    // 播放速度倍率（1.0 = 正常速度）
    
    SharedAnimationComponent() = default;
    
    /**
     * @brief 构造函数
     * @param id 动画ID（对应 AnimationCacheManager 中的缓存键）
     * @param shouldLoop 是否循环播放
     * @param speed 播放速度倍率
     */
    SharedAnimationComponent(const std::string& id, bool shouldLoop = true, float speed = 1.0f)
        : animationId(id)
        , loop(shouldLoop)
        , playbackSpeed(speed) {}
    
    /**
     * @brief 重置播放状态
     */
    void reset() {
        currentFrameIndex = 0;
        frameTimer = 0.0f;
        isPlaying = true;
    }
    
    /**
     * @brief 暂停播放
     */
    void pause() {
        isPlaying = false;
    }
    
    /**
     * @brief 恢复播放
     */
    void resume() {
        isPlaying = true;
    }
    
    /**
     * @brief 切换动画
     * @param newAnimationId 新的动画ID
     * @param resetState 是否重置播放状态
     */
    void switchAnimation(const std::string& newAnimationId, bool resetState = true) {
        if (animationId != newAnimationId) {
            animationId = newAnimationId;
            if (resetState) {
                reset();
            }
        }
    }
};

} // namespace ecs

#endif // __ECS_COMPONENT_SHAREDANIMATIONCOMPONENT_H__
