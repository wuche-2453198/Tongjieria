#ifndef __ECS_ANIMATION_CACHE_MANAGER_H__
#define __ECS_ANIMATION_CACHE_MANAGER_H__

#include "cocos2d.h"
#include <unordered_map>
#include <string>
#include <vector>

namespace ecs {

/**
 * @brief 动画缓存管理器 - 管理动画对象的缓存和共享
 * 
 * 职责：
 * - 缓存 Animation 对象，避免重复创建
 * - 支持多个实体共享同一个 Animation 实例
 * - 提供预加载接口，在场景初始化时加载动画
 * - 管理动画资源的生命周期
 * 
 * 设计原则：
 * - 使用 animationId 作为缓存键
 * - Animation 对象使用 retain/release 管理引用计数
 * - 支持懒加载和预加载两种模式
 * 
 * Requirements: 6.1, 6.2
 */
class AnimationCacheManager {
public:
    /**
     * @brief 获取单例实例
     */
    static AnimationCacheManager& getInstance();
    
    /**
     * @brief 获取动画（从缓存或创建新的）
     * @param animationId 动画ID
     * @return Animation 指针，如果不存在返回 nullptr
     * 
     * 注意：返回的 Animation 已被 retain，调用者不需要再 retain
     * 多个实体可以共享同一个 Animation 实例
     */
    cocos2d::Animation* getAnimation(const std::string& animationId);
    
    /**
     * @brief 预加载动画
     * @param animationId 动画ID（用于缓存查找）
     * @param frameNames 精灵帧名称列表（从 SpriteFrameCache 获取）
     * @param frameTime 每帧时间（秒）
     * @param loops 循环次数（-1 表示无限循环）
     * @return 是否成功
     * 
     * 在场景初始化时调用，预先创建并缓存动画对象
     */
    bool preloadAnimation(const std::string& animationId,
                          const std::vector<std::string>& frameNames,
                          float frameTime,
                          int loops = -1);
    
    /**
     * @brief 预加载动画（使用帧索引范围）
     * @param animationId 动画ID
     * @param framePrefix 帧名称前缀（如 "GreenSlime_Green_Slime"）
     * @param startIndex 起始索引
     * @param endIndex 结束索引
     * @param frameTime 每帧时间
     * @param loops 循环次数
     * @return 是否成功
     */
    bool preloadAnimationWithRange(const std::string& animationId,
                                   const std::string& framePrefix,
                                   int startIndex,
                                   int endIndex,
                                   float frameTime,
                                   int loops = -1);
    
    /**
     * @brief 检查动画是否已缓存
     * @param animationId 动画ID
     * @return 是否已缓存
     */
    bool hasAnimation(const std::string& animationId) const;
    
    /**
     * @brief 移除指定动画缓存
     * @param animationId 动画ID
     */
    void removeAnimation(const std::string& animationId);
    
    /**
     * @brief 清理所有未使用的动画（引用计数为1的）
     * 
     * 在场景切换时调用，释放不再使用的动画资源
     */
    void cleanup();
    
    /**
     * @brief 清空所有动画缓存
     * 
     * 在游戏退出或需要完全重置时调用
     */
    void clearAll();
    
    /**
     * @brief 获取缓存统计信息
     * @return 缓存的动画数量
     */
    size_t getCacheSize() const { return _animationCache.size(); }
    
    /**
     * @brief 打印缓存状态（调试用）
     */
    void printCacheStatus() const;
    
private:
    AnimationCacheManager() = default;
    ~AnimationCacheManager();
    AnimationCacheManager(const AnimationCacheManager&) = delete;
    AnimationCacheManager& operator=(const AnimationCacheManager&) = delete;
    
    // 动画缓存（animationId -> Animation*）
    std::unordered_map<std::string, cocos2d::Animation*> _animationCache;
};

} // namespace ecs

#endif // __ECS_ANIMATION_CACHE_MANAGER_H__
