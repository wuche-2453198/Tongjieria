#ifndef __PLAYER_ANIMATION_LOADER_H__
#define __PLAYER_ANIMATION_LOADER_H__

#include "cocos2d.h"
#include "PlayerComponents.h"
#include <vector>
#include <string>
#include <map>

/**
 * @class PlayerAnimationLoader
 * @brief 玩家动画资源加载器
 *
 * 职责：
 * - 从 Resources/player/ 加载动画帧
 * - 管理不同动画状态的帧序列
 * - 提供动画数据给 PlayerAnimationSystem
 */
class PlayerAnimationLoader {
public:
    /**
     * @brief 动画帧数据
     */
    struct AnimationFrames {
        std::vector<cocos2d::Sprite*> frames;  // 帧精灵列表
        float frameTime = 0.07f;               // 每帧时间（秒）
        bool loop = true;                      // 是否循环播放

        int getFrameCount() const { return static_cast<int>(frames.size()); }
    };

    /**
     * @brief 初始化动画资源（预加载所有动画）
     * @param parentNode 父节点（用于添加精灵）
     * @return 是否成功加载
     */
    static bool initialize(cocos2d::Node* parentNode);

    /**
     * @brief 加载指定动画状态的帧
     * @param state 动画状态
     * @param parentNode 父节点
     * @return 动画帧数据
     */
    static AnimationFrames loadAnimation(
        ecs::PlayerAnimationComponent::AnimState state,
        cocos2d::Node* parentNode
    );

    /**
     * @brief 获取已加载的动画数据
     * @param state 动画状态
     * @return 动画帧数据指针，如果未加载返回 nullptr
     */
    static const AnimationFrames* getAnimation(
        ecs::PlayerAnimationComponent::AnimState state
    );

    /**
     * @brief 清理所有动画资源
     */
    static void cleanup();

private:
    PlayerAnimationLoader() = delete;

    // 动画资源缓存
    static std::map<ecs::PlayerAnimationComponent::AnimState, AnimationFrames> s_animationCache;

    /**
     * @brief 加载行走动画（14帧序列）
     */
    static AnimationFrames loadWalkAnimation(cocos2d::Node* parentNode);

    /**
     * @brief 加载站立动画（单帧）
     */
    static AnimationFrames loadIdleAnimation(cocos2d::Node* parentNode);

    /**
     * @brief 加载跳跃动画（单帧）
     */
    static AnimationFrames loadJumpAnimation(cocos2d::Node* parentNode);

    /**
     * @brief 加载下落动画（单帧）
     */
    static AnimationFrames loadFallAnimation(cocos2d::Node* parentNode);

    /**
     * @brief 加载坐下动画（单帧）
     */
    static AnimationFrames loadSitAnimation(cocos2d::Node* parentNode);

    /**
     * @brief 创建精灵帧（通用方法）
     */
    static cocos2d::Sprite* createFrameSprite(
        const std::string& path,
        cocos2d::Node* parentNode
    );
};

#endif // __PLAYER_ANIMATION_LOADER_H__
