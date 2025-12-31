# 泰拉瑞亚风格 Player 系统设计文档

基于 Cocos2d-x + EnTT 的玩家系统设计方案

## 一、系统架构概览

### 1.1 核心理念
采用 **数据驱动** + **ECS架构**，将玩家功能解耦为独立组件，通过系统（System）处理逻辑。

### 1.2 目录结构
```
Classes/player/
├── PlayerComponents.h         # 玩家相关组件定义
├── PlayerSystems.h/cpp        # 玩家系统声明与实现
├── PlayerFactory.h/cpp        # 玩家实体工厂
├── PlayerConfig.json          # 玩家配置文件
├── PlayerAnimation.h/cpp      # 玩家动画管理
├── PlayerInput.h/cpp          # 输入处理（键盘/手柄）
└── PlayerAbilities.h/cpp      # 特殊能力（钩爪、翅膀等）
```

---

## 二、组件设计（Components）

### 2.1 核心组件

#### **PlayerTag**
```cpp
namespace ecs {
    // 玩家标识组件（用于系统识别）
    struct PlayerTag {};
}
```

#### **PlayerStatsComponent**
```cpp
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

    // 速度与跳跃
    float moveSpeed = 200.0f;          // 移动速度
    float jumpHeight = 400.0f;         // 跳跃高度
    int extraJumps = 0;                // 额外跳跃次数（云瓶等）
    int currentJumpCount = 0;          // 当前跳跃次数

    // 暴击
    float critChance = 0.04f;          // 基础暴击率 4%
    float critMultiplier = 2.0f;       // 暴击倍率

    // 状态
    bool isOnGround = false;
    bool isInWater = false;
    bool isInLava = false;

    // 无敌帧
    bool isInvincible = false;
    float invincibleTimer = 0.0f;
    float invincibleDuration = 0.6f;   // 受伤后无敌时间
};
```

#### **PlayerMovementComponent**
```cpp
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
    float accelerationRate = 1000.0f;  // 加速度
    float maxHorizontalSpeed = 250.0f; // 最大水平速度
};
```

#### **PlayerEquipmentComponent**
```cpp
struct PlayerEquipmentComponent {
    // 装备槽位
    struct EquipmentSlot {
        int itemId = 0;
        int prefixId = 0;              // 词缀ID（易怒的、神级的等）
    };

    // 装备栏（对应泰拉瑞亚的装备槽）
    EquipmentSlot helmet;              // 头盔
    EquipmentSlot chestplate;          // 胸甲
    EquipmentSlot leggings;            // 护腿

    EquipmentSlot accessory1;          // 饰品1
    EquipmentSlot accessory2;          // 饰品2
    EquipmentSlot accessory3;          // 饰品3
    EquipmentSlot accessory4;          // 饰品4
    EquipmentSlot accessory5;          // 饰品5
    EquipmentSlot accessory6;          // 饰品6（困难模式解锁）

    // 时装槽（纯视觉，不影响属性）
    EquipmentSlot vanityHelmet;
    EquipmentSlot vanityChest;
    EquipmentSlot vanityLegs;

    // 染料槽
    EquipmentSlot dyeHelmet;
    EquipmentSlot dyeChest;
    EquipmentSlot dyeLegs;
    EquipmentSlot dyeAccessory[6];

    // 宠物与坐骑
    EquipmentSlot pet;
    EquipmentSlot lightPet;
    EquipmentSlot mount;
    EquipmentSlot hook;                // 钩爪
    EquipmentSlot minecart;            // 矿车
};
```

#### **PlayerHotbarComponent**
```cpp
struct PlayerHotbarComponent {
    static const int HOTBAR_SIZE = 10; // 快捷栏10格
    int slots[HOTBAR_SIZE] = {0};      // 指向背包索引（0-49）
    int selectedIndex = 0;             // 当前选中索引（0-9）

    // 当前使用的物品
    int GetCurrentItemId() const;
};
```

#### **PlayerAnimationComponent**
```cpp
struct PlayerAnimationComponent {
    enum class AnimState {
        IDLE,
        WALK,
        RUN,
        JUMP,
        FALL,
        USE_ITEM,
        HURT,
        DEATH
    };

    AnimState currentState = AnimState::IDLE;
    AnimState previousState = AnimState::IDLE;

    float animationTime = 0.0f;
    int currentFrame = 0;

    // 使用物品动画
    bool isUsingItem = false;
    float itemUseProgress = 0.0f;      // 0.0 - 1.0
    float itemUseSpeed = 1.0f;         // 使用速度倍率
};
```

#### **PlayerBuffComponent**
```cpp
struct PlayerBuffComponent {
    struct Buff {
        int buffId;                    // Buff ID
        float duration;                // 剩余时间
        float totalDuration;           // 总时间

        // Buff效果（示例）
        float moveSpeedMultiplier = 1.0f;
        float damageMultiplier = 1.0f;
        float defenseBonus = 0.0f;
        bool canFly = false;
        // ... 更多效果
    };

    std::vector<Buff> activeBuffs;

    // 应用Buff效果到玩家属性
    void ApplyBuffsToStats(PlayerStatsComponent& stats);
};
```

#### **PlayerAbilityComponent**
```cpp
struct PlayerAbilityComponent {
    // 钩爪
    bool hasHook = false;
    bool hookActive = false;
    cocos2d::Vec2 hookPosition;
    cocos2d::Vec2 hookVelocity;

    // 翅膀/飞行
    bool hasWings = false;
    float wingFlightTime = 0.0f;
    float maxWingFlightTime = 2.0f;    // 最大飞行时间
    bool isFlying = false;

    // 双跳道具（云瓶、沙暴瓶等）
    std::vector<int> doubleJumpItems;  // 拥有的双跳道具ID
    int usedDoubleJumps = 0;

    // 坐骑
    bool isMounted = false;
    int mountId = 0;

    // 潜水时间
    float breathTimer = 7.0f;          // 水下呼吸时间（秒）

    // 掉落伤害免疫
    bool noFallDamage = false;
};
```

---

## 三、系统设计（Systems）

### 3.1 输入系统 - PlayerInputSystem

**职责**：处理键盘/手柄输入，更新 `PlayerMovementComponent`

```cpp
class PlayerInputSystem : public ecs::System {
public:
    void update(float dt) override {
        auto view = world->getEntitiesWith<PlayerTag, PlayerMovementComponent>();

        for (auto entity : view) {
            auto& movement = world->getComponent<PlayerMovementComponent>(entity);

            // 读取输入状态（通过 PlayerInput 单例）
            auto& input = PlayerInput::getInstance();

            movement.isMovingLeft = input.isKeyPressed(KeyCode::KEY_A);
            movement.isMovingRight = input.isKeyPressed(KeyCode::KEY_D);
            movement.wantsToJump = input.isKeyJustPressed(KeyCode::KEY_SPACE);

            // 更新朝向
            if (movement.isMovingRight) movement.isFacingRight = true;
            if (movement.isMovingLeft) movement.isFacingRight = false;
        }
    }

    int getPriority() const override { return 0; } // 最高优先级
};
```

### 3.2 移动系统 - PlayerMovementSystem

**职责**：根据输入更新玩家速度和位置

```cpp
class PlayerMovementSystem : public ecs::System {
public:
    void update(float dt) override {
        auto view = world->getEntitiesWith<
            PlayerTag,
            PlayerMovementComponent,
            PlayerStatsComponent,
            TransformComponent
        >();

        for (auto entity : view) {
            auto& movement = world->getComponent<PlayerMovementComponent>(entity);
            auto& stats = world->getComponent<PlayerStatsComponent>(entity);
            auto& transform = world->getComponent<TransformComponent>(entity);

            // 水平移动（带加速度）
            float targetVelocityX = 0.0f;
            if (movement.isMovingRight) {
                targetVelocityX = stats.moveSpeed;
            }
            if (movement.isMovingLeft) {
                targetVelocityX = -stats.moveSpeed;
            }

            // 平滑加速
            float accel = movement.accelerationRate * dt;
            if (targetVelocityX != 0) {
                movement.velocity.x += (targetVelocityX - movement.velocity.x) * accel;
            } else {
                // 摩擦力减速
                float friction = stats.isOnGround ? movement.friction : movement.airResistance;
                movement.velocity.x *= friction;
            }

            // 限制最大速度
            float maxSpeed = movement.maxHorizontalSpeed;
            if (std::abs(movement.velocity.x) > maxSpeed) {
                movement.velocity.x = (movement.velocity.x > 0 ? maxSpeed : -maxSpeed);
            }

            // 跳跃处理
            if (movement.wantsToJump) {
                if (stats.isOnGround) {
                    // 地面跳跃
                    movement.velocity.y = stats.jumpHeight;
                    stats.currentJumpCount = 0;
                } else if (stats.currentJumpCount < stats.extraJumps) {
                    // 空中二段跳
                    movement.velocity.y = stats.jumpHeight;
                    stats.currentJumpCount++;
                    // TODO: 触发二段跳特效
                }
                movement.wantsToJump = false;
            }

            // 更新物理体（如果使用Cocos物理引擎）
            auto sprite = getNodeFromEntity(entity);
            if (sprite && sprite->getPhysicsBody()) {
                auto body = sprite->getPhysicsBody();
                body->setVelocity(cocos2d::Vec2(movement.velocity.x, body->getVelocity().y));
            }
        }
    }

    int getPriority() const override { return 10; }
};
```

### 3.3 装备系统 - PlayerEquipmentSystem

**职责**：根据装备更新玩家属性

```cpp
class PlayerEquipmentSystem : public ecs::System {
public:
    void update(float dt) override {
        auto view = world->getEntitiesWith<
            PlayerTag,
            PlayerEquipmentComponent,
            PlayerStatsComponent
        >();

        for (auto entity : view) {
            auto& equipment = world->getComponent<PlayerEquipmentComponent>(entity);
            auto& stats = world->getComponent<PlayerStatsComponent>(entity);

            // 重置基础属性
            stats.defense = 0;
            stats.meleeDamageBonus = 1.0f;
            // ... 重置其他属性

            // 应用装备加成
            applyEquipmentBonus(equipment.helmet, stats);
            applyEquipmentBonus(equipment.chestplate, stats);
            applyEquipmentBonus(equipment.leggings, stats);

            // 应用饰品加成
            applyEquipmentBonus(equipment.accessory1, stats);
            applyEquipmentBonus(equipment.accessory2, stats);
            // ... 其他饰品

            // 套装效果
            checkSetBonus(equipment, stats);
        }
    }

private:
    void applyEquipmentBonus(const PlayerEquipmentComponent::EquipmentSlot& slot,
                            PlayerStatsComponent& stats) {
        if (slot.itemId == 0) return;

        auto itemData = ItemManager::getInstance()->getItemData(slot.itemId);
        if (!itemData) return;

        // 从物品数据中读取属性加成
        // 例如：铜头盔 +1 防御
        stats.defense += itemData->defense;
        // ... 其他属性
    }

    void checkSetBonus(const PlayerEquipmentComponent& equipment,
                      PlayerStatsComponent& stats) {
        // 检查套装效果（例如：铜套装 +2 防御）
        // 实现逻辑：检查头盔、胸甲、护腿是否属于同一套装
    }

    int getPriority() const override { return 5; }
};
```

### 3.4 战斗系统 - PlayerCombatSystem

**职责**：处理玩家攻击、受伤、无敌帧

```cpp
class PlayerCombatSystem : public ecs::System {
public:
    void update(float dt) override {
        auto view = world->getEntitiesWith<
            PlayerTag,
            PlayerStatsComponent,
            PlayerHotbarComponent
        >();

        for (auto entity : view) {
            auto& stats = world->getComponent<PlayerStatsComponent>(entity);
            auto& hotbar = world->getComponent<PlayerHotbarComponent>(entity);

            // 更新无敌帧
            if (stats.isInvincible) {
                stats.invincibleTimer -= dt;
                if (stats.invincibleTimer <= 0) {
                    stats.isInvincible = false;
                }
            }

            // 使用物品（攻击）
            if (PlayerInput::getInstance().isMousePressed()) {
                int currentItemId = hotbar.GetCurrentItemId();
                if (currentItemId > 0) {
                    useItem(entity, currentItemId);
                }
            }
        }
    }

private:
    void useItem(EntityId player, int itemId) {
        auto itemData = ItemManager::getInstance()->getItemData(itemId);
        if (!itemData) return;

        // 根据物品类型执行不同逻辑
        if (itemData->type == ItemType::Equipment) {
            // 武器攻击逻辑
            // TODO: 生成弹幕/挥舞动画
        }
        // ... 其他类型
    }

    int getPriority() const override { return 20; }
};
```

### 3.5 动画系统 - PlayerAnimationSystem

**职责**：根据玩家状态更新动画

```cpp
class PlayerAnimationSystem : public ecs::System {
public:
    void update(float dt) override {
        auto view = world->getEntitiesWith<
            PlayerTag,
            PlayerAnimationComponent,
            PlayerMovementComponent,
            PlayerStatsComponent
        >();

        for (auto entity : view) {
            auto& anim = world->getComponent<PlayerAnimationComponent>(entity);
            auto& movement = world->getComponent<PlayerMovementComponent>(entity);
            auto& stats = world->getComponent<PlayerStatsComponent>(entity);

            // 确定当前动画状态
            auto newState = determineAnimationState(movement, stats);

            // 状态切换
            if (newState != anim.currentState) {
                anim.previousState = anim.currentState;
                anim.currentState = newState;
                anim.animationTime = 0.0f;
                anim.currentFrame = 0;

                // TODO: 播放对应动画
                playAnimation(entity, newState);
            }

            // 更新动画时间
            anim.animationTime += dt;
        }
    }

private:
    PlayerAnimationComponent::AnimState determineAnimationState(
        const PlayerMovementComponent& movement,
        const PlayerStatsComponent& stats) {

        if (stats.currentHealth <= 0) {
            return PlayerAnimationComponent::AnimState::DEATH;
        }

        if (!stats.isOnGround) {
            if (movement.velocity.y > 50) {
                return PlayerAnimationComponent::AnimState::JUMP;
            } else {
                return PlayerAnimationComponent::AnimState::FALL;
            }
        }

        if (std::abs(movement.velocity.x) > 10.0f) {
            return PlayerAnimationComponent::AnimState::WALK;
        }

        return PlayerAnimationComponent::AnimState::IDLE;
    }

    void playAnimation(EntityId entity, PlayerAnimationComponent::AnimState state) {
        // 根据状态播放对应动画
        // TODO: 使用 Cocos2d 的 Animate 动作
    }

    int getPriority() const override { return 100; } // 低优先级（渲染前）
};
```

### 3.6 Buff系统 - PlayerBuffSystem

**职责**：更新Buff持续时间，应用Buff效果

```cpp
class PlayerBuffSystem : public ecs::System {
public:
    void update(float dt) override {
        auto view = world->getEntitiesWith<
            PlayerTag,
            PlayerBuffComponent,
            PlayerStatsComponent
        >();

        for (auto entity : view) {
            auto& buffComp = world->getComponent<PlayerBuffComponent>(entity);
            auto& stats = world->getComponent<PlayerStatsComponent>(entity);

            // 更新Buff时间
            for (auto it = buffComp.activeBuffs.begin();
                 it != buffComp.activeBuffs.end();) {
                it->duration -= dt;

                if (it->duration <= 0) {
                    // Buff结束
                    it = buffComp.activeBuffs.erase(it);
                } else {
                    ++it;
                }
            }

            // 重新应用所有Buff效果
            buffComp.ApplyBuffsToStats(stats);
        }
    }

    int getPriority() const override { return 8; }
};
```

### 3.7 能力系统 - PlayerAbilitySystem

**职责**：处理钩爪、翅膀、坐骑等特殊能力

```cpp
class PlayerAbilitySystem : public ecs::System {
public:
    void update(float dt) override {
        auto view = world->getEntitiesWith<
            PlayerTag,
            PlayerAbilityComponent,
            PlayerMovementComponent,
            PlayerStatsComponent
        >();

        for (auto entity : view) {
            auto& ability = world->getComponent<PlayerAbilityComponent>(entity);
            auto& movement = world->getComponent<PlayerMovementComponent>(entity);
            auto& stats = world->getComponent<PlayerStatsComponent>(entity);

            // 钩爪逻辑
            if (ability.hasHook) {
                updateHook(entity, ability, movement);
            }

            // 翅膀飞行
            if (ability.hasWings) {
                updateWings(entity, ability, movement, stats, dt);
            }

            // 水下呼吸
            if (stats.isInWater) {
                updateBreathing(entity, ability, stats, dt);
            }
        }
    }

private:
    void updateHook(EntityId entity, PlayerAbilityComponent& ability,
                   PlayerMovementComponent& movement) {
        auto& input = PlayerInput::getInstance();

        if (input.isKeyJustPressed(KeyCode::KEY_E) && !ability.hookActive) {
            // 发射钩爪
            launchHook(entity, ability);
        }

        if (ability.hookActive) {
            // 拉向钩爪点
            // TODO: 实现钩爪拉力
        }
    }

    void updateWings(EntityId entity, PlayerAbilityComponent& ability,
                    PlayerMovementComponent& movement,
                    PlayerStatsComponent& stats, float dt) {
        auto& input = PlayerInput::getInstance();

        if (input.isKeyPressed(KeyCode::KEY_SPACE) && !stats.isOnGround) {
            // 飞行中
            if (ability.wingFlightTime < ability.maxWingFlightTime) {
                ability.isFlying = true;
                ability.wingFlightTime += dt;

                // 抵消重力
                movement.velocity.y = 100.0f; // 缓慢上升或悬停
            } else {
                ability.isFlying = false;
            }
        } else {
            ability.isFlying = false;
        }

        // 着地后恢复飞行时间
        if (stats.isOnGround) {
            ability.wingFlightTime = 0.0f;
        }
    }

    void updateBreathing(EntityId entity, PlayerAbilityComponent& ability,
                        PlayerStatsComponent& stats, float dt) {
        ability.breathTimer -= dt;

        if (ability.breathTimer <= 0) {
            // 溺水伤害
            stats.currentHealth -= 10.0f * dt; // 每秒10点伤害
        }
    }

    int getPriority() const override { return 15; }
};
```

---

## 四、工厂类设计

### 4.1 PlayerFactory

**职责**：创建玩家实体，添加所有必要组件

```cpp
class PlayerFactory {
public:
    static EntityId createPlayer(ecs::World& world,
                                 const cocos2d::Vec2& spawnPos,
                                 cocos2d::Node* parentNode) {
        // 创建实体
        EntityId player = world.createEntity("Player");

        // 添加标识
        world.addComponent<PlayerTag>(player);

        // 添加Transform
        world.addComponent<TransformComponent>(player, spawnPos.x, spawnPos.y);

        // 添加属性
        auto& stats = world.addComponent<PlayerStatsComponent>(player);
        loadPlayerStats(stats); // 从存档/配置加载

        // 添加移动
        world.addComponent<PlayerMovementComponent>(player);

        // 添加装备
        world.addComponent<PlayerEquipmentComponent>(player);

        // 添加快捷栏
        world.addComponent<PlayerHotbarComponent>(player);

        // 添加动画
        world.addComponent<PlayerAnimationComponent>(player);

        // 添加Buff
        world.addComponent<PlayerBuffComponent>(player);

        // 添加能力
        world.addComponent<PlayerAbilityComponent>(player);

        // 创建Cocos精灵
        auto sprite = createPlayerSprite();
        sprite->setPosition(spawnPos);
        parentNode->addChild(sprite);

        // 添加物理体
        auto physicsBody = PhysicsBody::createBox(
            Size(20, 42),
            PhysicsMaterial(1.0f, 0.0f, 0.5f)
        );
        physicsBody->setDynamic(true);
        physicsBody->setRotationEnable(false); // 玩家不旋转
        physicsBody->setContactTestBitmask(0xFFFFFFFF);
        sprite->setPhysicsBody(physicsBody);

        // 注册节点映射
        NodeEntityMap::getInstance().registerNode(sprite, player);

        CCLOG("Player created at (%.1f, %.1f)", spawnPos.x, spawnPos.y);
        return player;
    }

private:
    static cocos2d::Sprite* createPlayerSprite() {
        // 尝试加载玩家精灵
        auto sprite = Sprite::create("player/player_idle_0.png");
        if (!sprite) {
            // 备用方案：创建矩形
            sprite = Sprite::create();
            sprite->setTextureRect(Rect(0, 0, 20, 42));
            sprite->setColor(Color3B(255, 200, 100));
        }
        return sprite;
    }

    static void loadPlayerStats(PlayerStatsComponent& stats) {
        // 从配置文件或存档加载玩家数据
        // TODO: 读取 PlayerConfig.json
    }
};
```

---

## 五、使用示例

### 5.1 在GameScene中创建玩家

```cpp
// GameScene.cpp
#include "player/PlayerFactory.h"
#include "player/PlayerSystems.h"

bool GameScene::init() {
    if (!Layer::init()) return false;

    // 初始化ECS世界
    _world = new ecs::World();

    // 注册玩家系统
    _world->addSystem<PlayerInputSystem>();
    _world->addSystem<PlayerMovementSystem>();
    _world->addSystem<PlayerBuffSystem>();
    _world->addSystem<PlayerEquipmentSystem>();
    _world->addSystem<PlayerAbilitySystem>();
    _world->addSystem<PlayerCombatSystem>();
    _world->addSystem<PlayerAnimationSystem>();

    // 创建玩家
    Vec2 spawnPos(400, 300);
    _playerEntity = PlayerFactory::createPlayer(*_world, spawnPos, this);

    // 启动更新
    this->scheduleUpdate();

    return true;
}

void GameScene::update(float dt) {
    _world->update(dt);
}
```

### 5.2 装备物品

```cpp
// 从背包穿戴装备
void equipItemFromInventory(int inventorySlotIndex) {
    auto& inventory = Inventory::getInstance();
    auto slot = inventory.getSlot(inventorySlotIndex);

    if (slot.itemId == 0) return;

    auto itemData = ItemManager::getInstance()->getItemData(slot.itemId);
    if (itemData->type != ItemType::Equipment) return;

    // 获取玩家装备组件
    auto& equipment = _world->getComponent<PlayerEquipmentComponent>(_playerEntity);

    // 根据物品子类型装备到对应槽位
    if (isHelmet(itemData)) {
        // 交换：将旧装备放回背包
        if (equipment.helmet.itemId != 0) {
            inventory.addItem(equipment.helmet.itemId, 1);
        }

        // 装备新物品
        equipment.helmet.itemId = slot.itemId;
        equipment.helmet.prefixId = slot.prefixId;

        // 从背包移除
        inventory.removeItem(slot.itemId, 1);
    }
    // ... 其他槽位同理
}
```

### 5.3 添加Buff

```cpp
// 使用药水，添加Buff
void useHealthPotion() {
    auto& buffComp = _world->getComponent<PlayerBuffComponent>(_playerEntity);

    PlayerBuffComponent::Buff regenBuff;
    regenBuff.buffId = 1001; // 生命再生Buff
    regenBuff.duration = 30.0f; // 持续30秒
    regenBuff.totalDuration = 30.0f;
    regenBuff.defenseBonus = 2.0f;

    buffComp.activeBuffs.push_back(regenBuff);

    CCLOG("Added regen buff for 30 seconds");
}
```

---

## 六、配置文件示例

### PlayerConfig.json
```json
{
  "defaultStats": {
    "maxHealth": 100,
    "maxMana": 20,
    "defense": 0,
    "moveSpeed": 200,
    "jumpHeight": 400
  },

  "animations": {
    "idle": {
      "frames": ["player/idle_0.png", "player/idle_1.png"],
      "frameTime": 0.2
    },
    "walk": {
      "frames": ["player/walk_0.png", "player/walk_1.png", "player/walk_2.png"],
      "frameTime": 0.15
    }
  }
}
```

---

## 七、后续扩展方向

1. **钓鱼系统**：添加 `PlayerFishingComponent` + `FishingSystem`
2. **建造系统**：添加 `PlayerBuildComponent` + `PlacementSystem`
3. **魔法系统**：添加 `PlayerMagicComponent` + `MagicCastSystem`
4. **宠物/召唤物**：添加 `PlayerMinionComponent` + `MinionAISystem`
5. **成就系统**：添加 `PlayerAchievementComponent` + `AchievementSystem`
6. **NPC交互**：添加 `PlayerInteractionComponent` + `NPCDialogueSystem`

---

## 八、注意事项

### 8.1 性能优化
- 使用EnTT的视图（View）高效遍历实体
- 避免频繁的组件添加/删除，改用标志位
- 动画系统使用纹理图集（Sprite Sheet）减少批次

### 8.2 存档系统
- 定期序列化玩家组件数据到JSON/二进制文件
- 保存内容：属性、装备、背包、Buff状态

### 8.3 网络同步（多人模式）
- 使用状态同步而非指令同步
- 关键组件：`PlayerStatsComponent`, `TransformComponent`, `PlayerEquipmentComponent`
- 客户端预测 + 服务器校正

---

## 九、总结

本设计方案完全基于你现有的架构：
- ✅ 使用 **EnTT** 管理实体和组件
- ✅ 与 **ItemManager/Inventory** 系统无缝集成
- ✅ 遵循 **ECS模式**，高度解耦
- ✅ 参考 **MonsterFactory** 的设计模式
- ✅ 完整实现泰拉瑞亚的核心玩家功能

开始实现时，建议按以下顺序：
1. 实现基础组件（PlayerTag, PlayerStatsComponent, PlayerMovementComponent）
2. 实现输入系统和移动系统（让玩家能动起来）
3. 实现装备系统（与物品栏集成）
4. 实现战斗系统和动画系统
5. 最后添加能力系统和Buff系统

祝你开发顺利！🎮
