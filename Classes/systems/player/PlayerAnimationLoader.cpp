#include "PlayerAnimationLoader.h"

USING_NS_CC;

// 静态成员初始化
std::map<ecs::PlayerAnimationComponent::AnimState, PlayerAnimationLoader::AnimationFrames>
    PlayerAnimationLoader::s_animationCache;

bool PlayerAnimationLoader::initialize(Node* parentNode) {
    if (!parentNode) {
        CCLOG("PlayerAnimationLoader::initialize - parentNode is null");
        return false;
    }

    CCLOG("PlayerAnimationLoader: Loading all player animations...");

    // Preload all animations
    s_animationCache[ecs::PlayerAnimationComponent::AnimState::IDLE] =
        loadIdleAnimation(parentNode);

    s_animationCache[ecs::PlayerAnimationComponent::AnimState::WALK] =
        loadWalkAnimation(parentNode);

    s_animationCache[ecs::PlayerAnimationComponent::AnimState::JUMP] =
        loadJumpAnimation(parentNode);

    s_animationCache[ecs::PlayerAnimationComponent::AnimState::FALL] =
        loadFallAnimation(parentNode);

    CCLOG("PlayerAnimationLoader: All animations loaded successfully!");
    CCLOG("  - IDLE: %d frames", s_animationCache[ecs::PlayerAnimationComponent::AnimState::IDLE].getFrameCount());
    CCLOG("  - WALK: %d frames", s_animationCache[ecs::PlayerAnimationComponent::AnimState::WALK].getFrameCount());
    CCLOG("  - JUMP: %d frames", s_animationCache[ecs::PlayerAnimationComponent::AnimState::JUMP].getFrameCount());
    CCLOG("  - FALL: %d frames", s_animationCache[ecs::PlayerAnimationComponent::AnimState::FALL].getFrameCount());

    return true;
}

PlayerAnimationLoader::AnimationFrames PlayerAnimationLoader::loadAnimation(
    ecs::PlayerAnimationComponent::AnimState state,
    Node* parentNode)
{
    switch (state) {
        case ecs::PlayerAnimationComponent::AnimState::IDLE:
            return loadIdleAnimation(parentNode);
        case ecs::PlayerAnimationComponent::AnimState::WALK:
            return loadWalkAnimation(parentNode);
        case ecs::PlayerAnimationComponent::AnimState::JUMP:
            return loadJumpAnimation(parentNode);
        case ecs::PlayerAnimationComponent::AnimState::FALL:
            return loadFallAnimation(parentNode);
        default:
            CCLOG("PlayerAnimationLoader: Animation state %d not implemented, using IDLE",
                  static_cast<int>(state));
            return loadIdleAnimation(parentNode);
    }
}

const PlayerAnimationLoader::AnimationFrames* PlayerAnimationLoader::getAnimation(
    ecs::PlayerAnimationComponent::AnimState state)
{
    auto it = s_animationCache.find(state);
    if (it != s_animationCache.end()) {
        return &it->second;
    }
    return nullptr;
}

void PlayerAnimationLoader::cleanup() {
    CCLOG("PlayerAnimationLoader: Cleaning up animations...");

    // Clear cache (sprites are managed by Cocos2d and will be auto-released)
    s_animationCache.clear();

    CCLOG("PlayerAnimationLoader: Cleanup complete");
}

// ==================== Specific Animation Loading Implementation ====================

PlayerAnimationLoader::AnimationFrames PlayerAnimationLoader::loadWalkAnimation(Node* parentNode) {
    AnimationFrames anim;
    anim.frameTime = 0.07f;  // 70ms per frame
    anim.loop = true;

    CCLOG("PlayerAnimationLoader: Starting to load WALK animation (14 frames)...");

    // Load animation using for loop
    int successCount = 0;
    for (int i = 0; i <= 13; i++) {
        std::string path = StringUtils::format("player/walk/frame_%02d_delay-0.07s.png", i);
        auto sprite = createFrameSprite(path, parentNode);
        if (sprite) {
            anim.frames.push_back(sprite);
            successCount++;
        } else {
            CCLOG("PlayerAnimationLoader: [FAILED] walk frame %d at path: %s", i, path.c_str());
        }
    }

    CCLOG("PlayerAnimationLoader: Loaded WALK animation: %d/%d frames successful", successCount, 14);
    return anim;
}

PlayerAnimationLoader::AnimationFrames PlayerAnimationLoader::loadIdleAnimation(Node* parentNode) {
    AnimationFrames anim;
    anim.frameTime = 0.1f;
    anim.loop = true;

    // Idle animation has only one frame
    auto sprite = createFrameSprite("player/idle/Style_1_male.png", parentNode);
    if (sprite) {
        anim.frames.push_back(sprite);
    } else {
        CCLOG("PlayerAnimationLoader: Failed to load IDLE frame");
    }

    CCLOG("PlayerAnimationLoader: Loaded IDLE animation with %d frames", anim.getFrameCount());
    return anim;
}

PlayerAnimationLoader::AnimationFrames PlayerAnimationLoader::loadJumpAnimation(Node* parentNode) {
    AnimationFrames anim;
    anim.frameTime = 0.1f;
    anim.loop = false;  // Jump doesn't loop

    // Jump uses idle pose (Terraria style)
    auto sprite = createFrameSprite("player/idle/Style_1_male.png", parentNode);
    if (sprite) {
        anim.frames.push_back(sprite);
    } else {
        CCLOG("PlayerAnimationLoader: Failed to load JUMP frame");
    }

    CCLOG("PlayerAnimationLoader: Loaded JUMP animation with %d frames", anim.getFrameCount());
    return anim;
}

PlayerAnimationLoader::AnimationFrames PlayerAnimationLoader::loadFallAnimation(Node* parentNode) {
    AnimationFrames anim;
    anim.frameTime = 0.1f;
    anim.loop = false;

    // Fall animation
    auto sprite = createFrameSprite("player/fall/Style_1_male_falling.png", parentNode);
    if (sprite) {
        anim.frames.push_back(sprite);
    } else {
        CCLOG("PlayerAnimationLoader: Failed to load FALL frame");
    }

    CCLOG("PlayerAnimationLoader: Loaded FALL animation with %d frames", anim.getFrameCount());
    return anim;
}

PlayerAnimationLoader::AnimationFrames PlayerAnimationLoader::loadSitAnimation(Node* parentNode) {
    AnimationFrames anim;
    anim.frameTime = 0.1f;
    anim.loop = true;

    auto sprite = createFrameSprite("player/sit/Style_1_male_sitting.png", parentNode);
    if (sprite) {
        anim.frames.push_back(sprite);
    }

    return anim;
}

Sprite* PlayerAnimationLoader::createFrameSprite(const std::string& path, Node* parentNode) {
    CCLOG("PlayerAnimationLoader: Attempting to load sprite: %s", path.c_str());

    auto sprite = Sprite::create(path);
    if (!sprite) {
        CCLOG("PlayerAnimationLoader: [ERROR] Failed to create sprite from: %s", path.c_str());

        // Try to get full path for debugging
        auto fullPath = cocos2d::FileUtils::getInstance()->fullPathForFilename(path);
        CCLOG("PlayerAnimationLoader: Full path resolved to: %s", fullPath.c_str());
        CCLOG("PlayerAnimationLoader: File exists: %s",
              cocos2d::FileUtils::getInstance()->isFileExist(path) ? "YES" : "NO");

        return nullptr;
    }

    CCLOG("PlayerAnimationLoader: [SUCCESS] Loaded sprite: %s", path.c_str());

    // Hide by default (visibility controlled by animation system)
    sprite->setVisible(false);
    sprite->retain();  // Retain reference to prevent auto-release

    // Add to parent node (but not visible)
    if (parentNode) {
        parentNode->addChild(sprite, 10);
    }

    return sprite;
}
