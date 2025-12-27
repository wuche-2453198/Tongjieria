#ifndef __PLAYER_ANIMATION_LOADER_H__
#define __PLAYER_ANIMATION_LOADER_H__

#include "cocos2d.h"
#include "components/player/PlayerComponents.h"
#include <vector>
#include <string>
#include <map>

/**
 * @class PlayerAnimationLoader
 * @brief Player animation resource loader
 *
 * Responsibilities:
 * - Load animation frames from Resources/player/
 * - Manage frame sequences for different animation states
 * - Provide animation data to PlayerAnimationSystem
 */
class PlayerAnimationLoader {
public:
    /**
     * @brief Animation frame data
     */
    struct AnimationFrames {
        std::vector<cocos2d::Sprite*> frames;  // Frame sprite list
        float frameTime = 0.07f;               // Time per frame (seconds)
        bool loop = true;                      // Whether to loop playback

        int getFrameCount() const { return static_cast<int>(frames.size()); }
    };

    /**
     * @brief Initialize animation resources (preload all animations)
     * @param parentNode Parent node (for adding sprites)
     * @return Whether loading succeeded
     */
    static bool initialize(cocos2d::Node* parentNode);

    /**
     * @brief Load frames for specified animation state
     * @param state Animation state
     * @param parentNode Parent node
     * @return Animation frame data
     */
    static AnimationFrames loadAnimation(
        ecs::PlayerAnimationComponent::AnimState state,
        cocos2d::Node* parentNode
    );

    /**
     * @brief Get loaded animation data
     * @param state Animation state
     * @return Animation frame data pointer, returns nullptr if not loaded
     */
    static const AnimationFrames* getAnimation(
        ecs::PlayerAnimationComponent::AnimState state
    );

    /**
     * @brief Clean up all animation resources
     */
    static void cleanup();

private:
    PlayerAnimationLoader() = delete;

    // Animation resource cache
    static std::map<ecs::PlayerAnimationComponent::AnimState, AnimationFrames> s_animationCache;

    /**
     * @brief Load walk animation (14 frame sequence)
     */
    static AnimationFrames loadWalkAnimation(cocos2d::Node* parentNode);

    /**
     * @brief Load idle animation (single frame)
     */
    static AnimationFrames loadIdleAnimation(cocos2d::Node* parentNode);

    /**
     * @brief Load jump animation (single frame)
     */
    static AnimationFrames loadJumpAnimation(cocos2d::Node* parentNode);

    /**
     * @brief Load fall animation (single frame)
     */
    static AnimationFrames loadFallAnimation(cocos2d::Node* parentNode);

    /**
     * @brief Load sit animation (single frame)
     */
    static AnimationFrames loadSitAnimation(cocos2d::Node* parentNode);

    /**
     * @brief Load break animation (left click - break block, 2 frames)
     */
    static AnimationFrames loadBreakAnimation(cocos2d::Node* parentNode);

    /**
     * @brief Load place animation (right click - place block, 2 frames)
     */
    static AnimationFrames loadPlaceAnimation(cocos2d::Node* parentNode);

    /**
     * @brief Load weapon swing animation (16 frames)
     * Used when player holds a weapon and clicks
     */
    static AnimationFrames loadWeaponSwingAnimation(cocos2d::Node* parentNode);

    /**
     * @brief Create sprite frame (common method)
     */
    static cocos2d::Sprite* createFrameSprite(
        const std::string& path,
        cocos2d::Node* parentNode
    );
};

#endif // __PLAYER_ANIMATION_LOADER_H__
