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

    s_animationCache[ecs::PlayerAnimationComponent::AnimState::BREAK] =
        loadBreakAnimation(parentNode);

    s_animationCache[ecs::PlayerAnimationComponent::AnimState::PLACE] =
        loadPlaceAnimation(parentNode);

    s_animationCache[ecs::PlayerAnimationComponent::AnimState::ATTACK] =
        loadWeaponSwingAnimation(parentNode);

    s_animationCache[ecs::PlayerAnimationComponent::AnimState::EAT] =
        loadEatAnimation(parentNode);

    s_animationCache[ecs::PlayerAnimationComponent::AnimState::DRINK] =
        loadDrinkAnimation(parentNode);

    s_animationCache[ecs::PlayerAnimationComponent::AnimState::MINE] =
        loadMineAnimation(parentNode);

    CCLOG("PlayerAnimationLoader: All animations loaded successfully!");
    CCLOG("  - IDLE: %d frames", s_animationCache[ecs::PlayerAnimationComponent::AnimState::IDLE].getFrameCount());
    CCLOG("  - WALK: %d frames", s_animationCache[ecs::PlayerAnimationComponent::AnimState::WALK].getFrameCount());
    CCLOG("  - JUMP: %d frames", s_animationCache[ecs::PlayerAnimationComponent::AnimState::JUMP].getFrameCount());
    CCLOG("  - FALL: %d frames", s_animationCache[ecs::PlayerAnimationComponent::AnimState::FALL].getFrameCount());
    CCLOG("  - BREAK: %d frames", s_animationCache[ecs::PlayerAnimationComponent::AnimState::BREAK].getFrameCount());
    CCLOG("  - PLACE: %d frames", s_animationCache[ecs::PlayerAnimationComponent::AnimState::PLACE].getFrameCount());
    CCLOG("  - ATTACK: %d frames", s_animationCache[ecs::PlayerAnimationComponent::AnimState::ATTACK].getFrameCount());
    CCLOG("  - EAT: %d frames", s_animationCache[ecs::PlayerAnimationComponent::AnimState::EAT].getFrameCount());
    CCLOG("  - DRINK: %d frames", s_animationCache[ecs::PlayerAnimationComponent::AnimState::DRINK].getFrameCount());
    CCLOG("  - MINE: %d frames", s_animationCache[ecs::PlayerAnimationComponent::AnimState::MINE].getFrameCount());

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
        case ecs::PlayerAnimationComponent::AnimState::BREAK:
            return loadBreakAnimation(parentNode);
        case ecs::PlayerAnimationComponent::AnimState::PLACE:
            return loadPlaceAnimation(parentNode);
        case ecs::PlayerAnimationComponent::AnimState::ATTACK:
            return loadWeaponSwingAnimation(parentNode);
        case ecs::PlayerAnimationComponent::AnimState::EAT:
            return loadEatAnimation(parentNode);
        case ecs::PlayerAnimationComponent::AnimState::DRINK:
            return loadDrinkAnimation(parentNode);
        case ecs::PlayerAnimationComponent::AnimState::MINE:
            return loadMineAnimation(parentNode);
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

    // Release all retained sprites to prevent memory leaks
    for (auto& pair : s_animationCache) {
        for (auto* sprite : pair.second.frames) {
            if (sprite) {
                sprite->removeFromParent();  // Remove from scene
                sprite->release();           // Release the retain() call from createFrameSprite
            }
        }
    }

    // Clear cache
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

PlayerAnimationLoader::AnimationFrames PlayerAnimationLoader::loadBreakAnimation(Node* parentNode) {
    AnimationFrames anim;
    anim.frameTime = 0.1f;  // 100ms per frame
    anim.loop = false;      // Don't loop, play once

    CCLOG("PlayerAnimationLoader: Loading BREAK animation (2 frames)...");

    // Load break animation frames (01, 02)
    int successCount = 0;
    for (int i = 1; i <= 2; i++) {
        std::string path = StringUtils::format("player/break/Style_1_male_break_%02d.png", i);
        auto sprite = createFrameSprite(path, parentNode);
        if (sprite) {
            anim.frames.push_back(sprite);
            successCount++;
        } else {
            CCLOG("PlayerAnimationLoader: [FAILED] break frame %d at path: %s", i, path.c_str());
        }
    }

    CCLOG("PlayerAnimationLoader: Loaded BREAK animation: %d/2 frames successful", successCount);
    return anim;
}

PlayerAnimationLoader::AnimationFrames PlayerAnimationLoader::loadPlaceAnimation(Node* parentNode) {
    AnimationFrames anim;
    anim.frameTime = 0.1f;  // 100ms per frame
    anim.loop = false;      // Don't loop, play once

    CCLOG("PlayerAnimationLoader: Loading PLACE animation (2 frames)...");

    // Load place animation frames (01, 02)
    int successCount = 0;
    for (int i = 1; i <= 2; i++) {
        std::string path = StringUtils::format("player/place/Style_1_male_place_%02d.png", i);
        auto sprite = createFrameSprite(path, parentNode);
        if (sprite) {
            anim.frames.push_back(sprite);
            successCount++;
        } else {
            CCLOG("PlayerAnimationLoader: [FAILED] place frame %d at path: %s", i, path.c_str());
        }
    }

    CCLOG("PlayerAnimationLoader: Loaded PLACE animation: %d/2 frames successful", successCount);
    return anim;
}

PlayerAnimationLoader::AnimationFrames PlayerAnimationLoader::loadWeaponSwingAnimation(Node* parentNode) {
    AnimationFrames anim;
    anim.frameTime = 0.05f;  // 50ms per frame (fast swing)
    anim.loop = false;       // Don't loop, play once

    CCLOG("PlayerAnimationLoader: Loading ATTACK animation (16 frames)...");

    // Load frames 000-015
    int successCount = 0;
    for (int i = 0; i <= 15; i++) {
        std::string path = StringUtils::format("player/Swing_example/Swing_example_frame_%03d.png", i);
        auto sprite = createFrameSprite(path, parentNode);
        if (sprite) {
            anim.frames.push_back(sprite);
            successCount++;
        } else {
            CCLOG("PlayerAnimationLoader: [FAILED] weapon swing frame %d at path: %s", i, path.c_str());
        }
    }

    CCLOG("PlayerAnimationLoader: Loaded ATTACK animation: %d/16 frames successful", successCount);
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

PlayerAnimationLoader::AnimationFrames PlayerAnimationLoader::loadEatAnimation(Node* parentNode) {
    AnimationFrames anim;
    anim.frameTime = 0.07f;  // 70ms per frame
    anim.loop = false;       // Don't loop, play once

    CCLOG("PlayerAnimationLoader: Loading EAT animation...");

    // Load eat animation frames from player/eat/ directory
    // Try EatFood_example_frame_NNN.png format first (starts from 000)
    int successCount = 0;
    for (int i = 0; i < 20; i++) {  // Try up to 20 frames, starting from 000
        std::string path = StringUtils::format("player/eat/EatFood_example_frame_%03d.png", i);
        auto sprite = createFrameSprite(path, parentNode);
        if (sprite) {
            anim.frames.push_back(sprite);
            successCount++;
        } else {
            // Stop if frame doesn't exist
            break;
        }
    }

    // If no frames found, try alternative formats
    if (successCount == 0) {
        CCLOG("PlayerAnimationLoader: No EatFood frames found, trying alternative formats...");
        for (int i = 1; i <= 20; i++) {
            std::string path = StringUtils::format("player/eat/Style_1_male_%02d.png", i);
            auto sprite = createFrameSprite(path, parentNode);
            if (sprite) {
                anim.frames.push_back(sprite);
                successCount++;
            } else {
                break;
            }
        }
    }

    CCLOG("PlayerAnimationLoader: Loaded EAT animation: %d frames", successCount);
    return anim;
}

PlayerAnimationLoader::AnimationFrames PlayerAnimationLoader::loadDrinkAnimation(Node* parentNode) {
    AnimationFrames anim;
    anim.frameTime = 0.07f;  // 70ms per frame
    anim.loop = false;       // Don't loop, play once

    CCLOG("PlayerAnimationLoader: Loading DRINK animation...");

    // Load drink animation frames from player/drink/ directory
    // Try DrinkOld_example_frame_NNN.png format first (starts from 000)
    int successCount = 0;
    for (int i = 0; i < 20; i++) {  // Try up to 20 frames, starting from 000
        std::string path = StringUtils::format("player/drink/DrinkOld_example_frame_%03d.png", i);
        auto sprite = createFrameSprite(path, parentNode);
        if (sprite) {
            anim.frames.push_back(sprite);
            successCount++;
        } else {
            // Stop if frame doesn't exist
            break;
        }
    }

    // If no frames found, try alternative formats
    if (successCount == 0) {
        CCLOG("PlayerAnimationLoader: No DrinkOld frames found, trying alternative formats...");
        for (int i = 1; i <= 20; i++) {
            std::string path = StringUtils::format("player/drink/Style_1_male_%02d.png", i);
            auto sprite = createFrameSprite(path, parentNode);
            if (sprite) {
                anim.frames.push_back(sprite);
                successCount++;
            } else {
                break;
            }
        }
    }

    CCLOG("PlayerAnimationLoader: Loaded DRINK animation: %d frames", successCount);
    return anim;
}

PlayerAnimationLoader::AnimationFrames PlayerAnimationLoader::loadMineAnimation(Node* parentNode) {
    AnimationFrames anim;
    anim.frameTime = 0.07f;  // 70ms per frame
    anim.loop = true;        // Loop continuously while mining

    CCLOG("PlayerAnimationLoader: Loading MINE animation...");

    // Load mine animation frames from player/pickaxe/ directory
    // Try NN.png format (00.png, 01.png, etc.)
    int successCount = 0;
    for (int i = 0; i < 20; i++) {  // Try up to 20 frames
        std::string path = StringUtils::format("player/pickaxe/%02d.png", i);
        auto sprite = createFrameSprite(path, parentNode);
        if (sprite) {
            anim.frames.push_back(sprite);
            successCount++;
        } else {
            // Stop if frame doesn't exist
            break;
        }
    }

    CCLOG("PlayerAnimationLoader: Loaded MINE animation: %d frames", successCount);
    return anim;
}
