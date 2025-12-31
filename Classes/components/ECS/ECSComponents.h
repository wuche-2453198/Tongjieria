//ECSComponents.h
#ifndef __ECS_COMPONENTS_H__
#define __ECS_COMPONENTS_H__

#include <string>
#include "cocos2d.h"

USING_NS_CC;

namespace ECS {  // 添加ECS命名空间
    // ==================== 基础组件 ====================
    struct Transform {
        float x = 0.0f;
        float y = 0.0f;
        float rotation = 0.0f;
        float scaleX = 1.0f;
        float scaleY = 1.0f;

        Transform() = default;
        Transform(float px, float py) : x(px), y(py) {}

        // 获取位置
        Vec2 getPosition() const { return Vec2(x, y); }
        void setPosition(const Vec2& pos) { x = pos.x; y = pos.y; }
    };

    // ==================== 精灵组件 ====================
    struct SpriteComponent {
        std::string texturePath;  // 纹理路径
        Sprite* cocosSprite = nullptr;  // Cocos2d-x Sprite 指针
        Color3B color = Color3B::WHITE;
        int zOrder = 0;

        SpriteComponent() = default;
        SpriteComponent(const std::string& path) : texturePath(path) {}

        // 创建 Cocos2d-x Sprite
        Sprite* createCocosSprite() {
            if (texturePath.empty()) {
                cocosSprite = Sprite::create();
                cocosSprite->setColor(color);
                cocosSprite->setContentSize(Size(50, 50));
            }
            else {
                cocosSprite = Sprite::create(texturePath);
                if (!cocosSprite) {
                    cocosSprite = Sprite::create();
                    cocosSprite->setColor(color);
                    cocosSprite->setContentSize(Size(50, 50));
                }
            }
            return cocosSprite;
        }

        // 释放 Cocos2d-x Sprite
        void releaseCocosSprite() {
            if (cocosSprite) {
                cocosSprite->removeFromParent();
                cocosSprite = nullptr;
            }
        }

        // 设置位置
        void setPosition(float x, float y) {
            if (cocosSprite) {
                cocosSprite->setPosition(x, y);
            }
        }
    };

    // ==================== 玩家组件 ====================
    struct PlayerComponent {
        std::string name = "Player";
        int level = 1;
        int exp = 0;
        int health = 100;
        int maxHealth = 100;
        float moveSpeed = 200.0f;
        int coins = 0;
        int mana = 100;
        int maxMana = 100;

        PlayerComponent() = default;
        PlayerComponent(const std::string& n, int lvl, int e)
            : name(n), level(lvl), exp(e) {
        }
    };

    // ==================== 物理组件 ====================
    struct PhysicsComponent {
        float velocityX = 0.0f;
        float velocityY = 0.0f;
        bool isGrounded = false;
        float gravity = 9.8f;
        float friction = 0.9f;
        float mass = 1.0f;
        bool hasCollision = true;
    };

    // ==================== 存档组件 ====================
    struct SaveableComponent {
        bool shouldSave = true;
        std::string saveGroup = "default";
        int priority = 0;
        std::string entityType;  // 实体类型：player, npc, block, item等

        SaveableComponent() = default;
        SaveableComponent(bool save, const std::string& group, const std::string& type = "entity")
            : shouldSave(save), saveGroup(group), entityType(type) {
        }
    };

    // ==================== NPC组件 ====================
    struct NPCComponent {
        std::string npcName;
        std::string dialogue;
        bool isMerchant = false;
        int npcType = 0;  // 0=普通NPC, 1=商人, 2=任务NPC
        int shopId = 0;
        std::vector<int> questIds;  // 相关任务ID
    };

    // ==================== 方块组件 ====================
    struct BlockComponent {
        int blockId = 0;
        std::string blockName;
        int hardness = 1;
        bool isSolid = true;
        bool isBreakable = true;
        int dropItemId = 0;
        bool isPlaced = false;
        int durability = 100;

        BlockComponent() = default;
        BlockComponent(int id, const std::string& name, bool solid = true)
            : blockId(id), blockName(name), isSolid(solid) {
        }
    };

    // ==================== 物品组件 ====================
    struct ItemComponent {
        int itemId = 0;
        std::string itemName;
        int stackSize = 1;
        int maxStack = 99;
        std::string description;
        bool isCollectible = true;
        int value = 1;  // 物品价值
        std::string itemType;  // 物品类型：weapon, armor, consumable等

        ItemComponent() = default;
        ItemComponent(int id, const std::string& name, int max = 99)
            : itemId(id), itemName(name), maxStack(max) {
        }
    };

    // ==================== 标签组件（用于标记实体）====================
    struct TagComponent {
        std::string tag;

        TagComponent() = default;
        TagComponent(const std::string& t) : tag(t) {}
    };

    // ==================== 动画组件 ====================
    struct AnimationComponent {
        std::string currentAnimation;
        float animationSpeed = 1.0f;
        bool isLooping = true;
        float frameTime = 0.0f;
        int currentFrame = 0;

        AnimationComponent() = default;
        AnimationComponent(const std::string& anim) : currentAnimation(anim) {}
    };
}
#endif // __ECS_COMPONENTS_H__
