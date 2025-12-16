#ifndef __PLAYER_COMPONENTS_H__
#define __PLAYER_COMPONENTS_H__

#include "cocos2d.h"
#include <vector>

namespace ecs {

// ==================== Player Tag ====================
struct PlayerTag {};

// ==================== Transform Component ====================
// 注意：如果你的项目已经有 TransformComponent，可以复用
struct TransformComponent {
    float x = 0.0f;
    float y = 0.0f;

    TransformComponent() = default;
    TransformComponent(float _x, float _y) : x(_x), y(_y) {}
};

// ==================== Player Stats Component ====================
struct PlayerStatsComponent {
    // 生命值
    float maxHealth = 100.0f;
    float currentHealth = 100.0f;
    float healthRegen = 0.5f;          // 每秒回血

    // 魔法值
    float maxMana = 20.0f;
    float currentMana = 20.0f;
    float manaRegen = 0.2f;            // 每秒回魔

    // 防御与伤害
    int defense = 0;                   // 防御力
    float meleeDamageBonus = 1.0f;     // 近战伤害加成
    float rangedDamageBonus = 1.0f;    // 远程伤害加成
    float magicDamageBonus = 1.0f;     // 魔法伤害加成
    float summonDamageBonus = 1.0f;    // 召唤物伤害加成

    // 速度与跳跃
    float moveSpeed = 200.0f;          // 移动速度（像素/秒）
    float jumpHeight = 400.0f;         // 跳跃初速度
    int extraJumps = 0;                // 额外跳跃次数（云瓶等）
    int currentJumpCount = 0;          // 当前已使用的跳跃次数

    // 暴击
    float critChance = 0.04f;          // 基础暴击率 4%
    float critMultiplier = 2.0f;       // 暴击倍率

    // 状态标志
    bool isOnGround = false;
    bool isInWater = false;
    bool isInLava = false;
    bool isDead = false;

    // 无敌帧
    bool isInvincible = false;
    float invincibleTimer = 0.0f;
    float invincibleDuration = 0.6f;   // 受伤后无敌时间

    // 生命再生计时器
    float regenTimer = 0.0f;
};

// ==================== Player Movement Component ====================
struct PlayerMovementComponent {
    cocos2d::Vec2 velocity = {0, 0};
    cocos2d::Vec2 acceleration = {0, 0};

    float friction = 0.85f;            // 地面摩擦力
    float airResistance = 0.95f;       // 空中阻力

    bool isMovingLeft = false;
    bool isMovingRight = false;
    bool wantsToJump = false;
    bool isFacingRight = true;

    // 泰拉瑞亚特色：加速移动
    float accelerationRate = 1500.0f;  // 加速度
    float maxHorizontalSpeed = 250.0f; // 最大水平速度

    // 冲刺（饰品支持）
    bool canDash = false;
    bool isDashing = false;
    float dashCooldown = 0.0f;
    float dashDuration = 0.2f;
    float dashSpeed = 600.0f;
};

// ==================== Player Equipment Component ====================
struct PlayerEquipmentComponent {
    // 装备槽位结构
    struct EquipmentSlot {
        int itemId = 0;
        int prefixId = 0;              // 词缀ID（易怒的、神级的等）

        bool isEmpty() const { return itemId == 0; }
        void clear() { itemId = 0; prefixId = 0; }
    };

    // 护甲槽位
    EquipmentSlot helmet;              // 头盔
    EquipmentSlot chestplate;          // 胸甲
    EquipmentSlot leggings;            // 护腿

    // 饰品槽位（基础3个，可扩展到6个）
    static const int MAX_ACCESSORIES = 6;
    EquipmentSlot accessories[MAX_ACCESSORIES];

    // 时装槽（纯视觉，不影响属性）
    EquipmentSlot vanityHelmet;
    EquipmentSlot vanityChest;
    EquipmentSlot vanityLegs;

    // 染料槽
    EquipmentSlot dyeHelmet;
    EquipmentSlot dyeChest;
    EquipmentSlot dyeLegs;
    EquipmentSlot dyeAccessory[MAX_ACCESSORIES];

    // 特殊槽位
    EquipmentSlot pet;                 // 宠物
    EquipmentSlot lightPet;            // 光源宠物
    EquipmentSlot mount;               // 坐骑
    EquipmentSlot hook;                // 钩爪
    EquipmentSlot minecart;            // 矿车
};

// ==================== Player Hotbar Component ====================
struct PlayerHotbarComponent {
    static const int HOTBAR_SIZE = 10; // 快捷栏10格
    int slots[HOTBAR_SIZE] = {0};      // 指向背包索引（0-49）
    int selectedIndex = 0;             // 当前选中索引（0-9）

    // 获取当前选中槽位的背包索引
    int getCurrentInventoryIndex() const {
        return slots[selectedIndex];
    }

    // 选择下一个/上一个槽位
    void selectNext() {
        selectedIndex = (selectedIndex + 1) % HOTBAR_SIZE;
    }

    void selectPrevious() {
        selectedIndex = (selectedIndex - 1 + HOTBAR_SIZE) % HOTBAR_SIZE;
    }

    void selectSlot(int index) {
        if (index >= 0 && index < HOTBAR_SIZE) {
            selectedIndex = index;
        }
    }
};

// ==================== Player Animation Component ====================
struct PlayerAnimationComponent {
    enum class AnimState {
        IDLE,
        WALK,
        RUN,
        JUMP,
        FALL,
        USE_ITEM,
        HURT,
        DEATH,
        SWIM
    };

    AnimState currentState = AnimState::IDLE;
    AnimState previousState = AnimState::IDLE;

    float animationTime = 0.0f;
    int currentFrame = 0;
    int totalFrames = 2;
    float frameTime = 0.2f;

    // 使用物品动画
    bool isUsingItem = false;
    float itemUseProgress = 0.0f;      // 0.0 - 1.0
    float itemUseSpeed = 1.0f;         // 使用速度倍率
    float itemUseTime = 0.3f;          // 物品使用时间

    // 翻转精灵
    bool needsFlip = false;
};

// ==================== Player Buff Component ====================
struct PlayerBuffComponent {
    struct Buff {
        int buffId = 0;                    // Buff ID
        float duration = 0.0f;             // 剩余时间
        float totalDuration = 0.0f;        // 总时间

        // Buff效果修正值
        float moveSpeedMultiplier = 1.0f;
        float damageMultiplier = 1.0f;
        float defenseBonus = 0.0f;
        float jumpHeightMultiplier = 1.0f;
        float healthRegenBonus = 0.0f;
        float manaRegenBonus = 0.0f;

        // 状态标志
        bool canFly = false;
        bool noFallDamage = false;
        bool waterBreathing = false;
        bool nightVision = false;

        Buff() = default;
        Buff(int id, float dur) : buffId(id), duration(dur), totalDuration(dur) {}
    };

    std::vector<Buff> activeBuffs;

    // 添加Buff（如果已存在同ID则刷新时间）
    void addBuff(const Buff& buff) {
        for (auto& b : activeBuffs) {
            if (b.buffId == buff.buffId) {
                b.duration = buff.duration;
                b.totalDuration = buff.totalDuration;
                return;
            }
        }
        activeBuffs.push_back(buff);
    }

    // 移除Buff
    void removeBuff(int buffId) {
        activeBuffs.erase(
            std::remove_if(activeBuffs.begin(), activeBuffs.end(),
                [buffId](const Buff& b) { return b.buffId == buffId; }),
            activeBuffs.end()
        );
    }

    // 检查是否有某个Buff
    bool hasBuff(int buffId) const {
        for (const auto& b : activeBuffs) {
            if (b.buffId == buffId) return true;
        }
        return false;
    }

    // 计算总修正值
    float getTotalMoveSpeedMultiplier() const {
        float mult = 1.0f;
        for (const auto& b : activeBuffs) {
            mult *= b.moveSpeedMultiplier;
        }
        return mult;
    }

    float getTotalDamageMultiplier() const {
        float mult = 1.0f;
        for (const auto& b : activeBuffs) {
            mult *= b.damageMultiplier;
        }
        return mult;
    }

    int getTotalDefenseBonus() const {
        int bonus = 0;
        for (const auto& b : activeBuffs) {
            bonus += static_cast<int>(b.defenseBonus);
        }
        return bonus;
    }
};

// ==================== Player Ability Component ====================
struct PlayerAbilityComponent {
    // 钩爪
    bool hasHook = false;
    bool hookActive = false;
    cocos2d::Vec2 hookPosition = {0, 0};
    cocos2d::Vec2 hookVelocity = {0, 0};
    float hookSpeed = 800.0f;
    float hookRange = 400.0f;
    float hookPullForce = 1000.0f;

    // 翅膀/飞行
    bool hasWings = false;
    float wingFlightTime = 0.0f;
    float maxWingFlightTime = 2.0f;    // 最大飞行时间
    bool isFlying = false;
    float wingHoverSpeed = 100.0f;

    // 双跳道具（云瓶、沙暴瓶等）
    std::vector<int> doubleJumpItems;  // 拥有的双跳道具ID

    // 坐骑
    bool isMounted = false;
    int mountId = 0;
    float mountSpeed = 300.0f;

    // 潜水/呼吸
    float maxBreathTime = 7.0f;        // 最大水下呼吸时间（秒）
    float breathTimer = 7.0f;          // 当前呼吸剩余时间
    bool hasWaterBreathing = false;    // 是否有水下呼吸效果

    // 掉落伤害
    bool noFallDamage = false;
    float fallDamageThreshold = 400.0f; // 超过此速度受伤
    float lastFallSpeed = 0.0f;

    // 墙体跳跃（忍者装备）
    bool canWallJump = false;
    bool isWallSliding = false;

    // 爬绳能力
    bool canClimbRopes = true;
    bool isClimbingRope = false;
};

// ==================== Player Combat Component ====================
struct PlayerCombatComponent {
    // 当前装备的武器
    int equippedWeaponId = 0;

    // 攻击状态
    bool isAttacking = false;
    float attackCooldown = 0.0f;
    float attackTimer = 0.0f;

    // 受伤状态
    bool justTookDamage = false;
    float damageCooldown = 0.0f;
    cocos2d::Vec2 knockbackVelocity = {0, 0};

    // 弹幕（投射物）
    int lastProjectileId = 0;          // 最后生成的弹幕ID
};

// ==================== Player Sprite Component ====================
// 用于关联Cocos2d精灵节点
struct PlayerSpriteComponent {
    cocos2d::Sprite* sprite = nullptr;
    cocos2d::Label* nameLabel = nullptr;

    // 血条UI
    cocos2d::Sprite* healthBarBg = nullptr;
    cocos2d::Sprite* healthBarFill = nullptr;

    // 魔法条UI
    cocos2d::Sprite* manaBarBg = nullptr;
    cocos2d::Sprite* manaBarFill = nullptr;

    ~PlayerSpriteComponent() {
        // 注意：不在这里释放，由Cocos2d管理
        // 只是存储引用
    }
};

} // namespace ecs

#endif // __PLAYER_COMPONENTS_H__
