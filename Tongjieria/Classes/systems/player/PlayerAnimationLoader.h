#ifndef __PLAYER_ANIMATION_LOADER_H__
#define __PLAYER_ANIMATION_LOADER_H__

#include "cocos2d.h"
#include "components/player/PlayerComponents.h"
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
 * - 为 PlayerAnimationSystem 提供动画数据
 */
class PlayerAnimationLoader {
public:
    /**
     * @brief 动画帧数据
     */
    struct AnimationFrames {
        std::vector<cocos2d::Sprite*> frames;  
        float frameTime = 0.07f;               
        bool loop = true;                      

        int getFrameCount() const { return static_cast<int>(frames.size()); }
    };

    /**
     * @brief 初始化动画资源（预加载所有动画）
     * @param parentNode 父节点（用于添加精灵）
     * @return 是否加载成功
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
     * @return 动画帧数据指针，未加载则返回 nullptr
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

    
    static std::map<ecs::PlayerAnimationComponent::AnimState, AnimationFrames> s_animationCache;

    /**
     * @brief 加载行走动画（14帧序列）
     */
    static AnimationFrames loadWalkAnimation(cocos2d::Node* parentNode);

    /**
     * @brief 加载待机动画（单帧）
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
     * @brief 加载破坏动画（左键 - 破坏方块，2帧）
     */
    static AnimationFrames loadBreakAnimation(cocos2d::Node* parentNode);

    /**
     * @brief 加载放置动画（右键 - 放置方块，2帧）
     */
    static AnimationFrames loadPlaceAnimation(cocos2d::Node* parentNode);

    /**
     * @brief 加载武器挥砍动画（16帧）
     * 玩家持有武器并点击时使用
     */
    static AnimationFrames loadWeaponSwingAnimation(cocos2d::Node* parentNode);

    /**
     * @brief 加载进食动画（吃食物）
     */
    static AnimationFrames loadEatAnimation(cocos2d::Node* parentNode);

    /**
     * @brief 加载饮用动画（喝药水）
     */
    static AnimationFrames loadDrinkAnimation(cocos2d::Node* parentNode);

    /**
     * @brief 加载挖掘动画（使用镐子挖掘）
     */
    static AnimationFrames loadMineAnimation(cocos2d::Node* parentNode);

    /**
     * @brief 创建精灵帧（通用方法）
     */
    static cocos2d::Sprite* createFrameSprite(
        const std::string& path,
        cocos2d::Node* parentNode
    );
};

#endif // __PLAYER_ANIMATION_LOADER_H__
