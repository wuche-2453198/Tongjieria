# Player 系统使用指南

## 📁 文件结构

```
Classes/player/
├── PlayerComponents.h          # ✅ 所有玩家组件定义
├── PlayerInput.h/cpp          # ✅ 输入管理器
├── PlayerFactory.h/cpp        # ✅ 玩家实体工厂
├── PlayerSystems.h/cpp        # ⏳ 待实现：所有玩家系统
├── PlayerAnimation.h/cpp      # ⏳ 待实现：动画管理
├── PlayerAbilities.h/cpp      # ⏳ 待实现：特殊能力
├── PlayerSystemDesign.md      # 📖 完整设计文档
└── README.md                  # 📖 本文件
```

## 🚀 快速开始

### 1. 在你的场景中初始化输入系统

```cpp
// GameScene.h
#include "player/PlayerInput.h"
#include "player/PlayerFactory.h"

class GameScene : public cocos2d::Layer {
private:
    ecs::World _world;
    ecs::EntityId _playerEntity;
};

// GameScene.cpp
bool GameScene::init() {
    if (!Layer::init()) return false;

    // 初始化输入系统
    PlayerInput::getInstance().initialize(this->getScene());

    // 创建玩家
    Vec2 spawnPos(400, 300);
    _playerEntity = PlayerFactory::createPlayer(_world, spawnPos, this);

    // 注册玩家系统（待实现）
    // _world.addSystem<PlayerInputSystem>();
    // _world.addSystem<PlayerMovementSystem>();
    // ...

    this->scheduleUpdate();
    return true;
}

void GameScene::update(float dt) {
    // 更新输入状态
    PlayerInput::getInstance().update(dt);

    // 更新ECS系统
    _world.update(dt);
}
```

### 2. 使用输入系统

```cpp
auto& input = PlayerInput::getInstance();

// 检查按键
if (input.isKeyPressed(EventKeyboard::KeyCode::KEY_A)) {
    CCLOG("A key is held down");
}

if (input.isKeyJustPressed(EventKeyboard::KeyCode::KEY_SPACE)) {
    CCLOG("Space just pressed (single frame)");
}

// 使用动作映射
if (input.isActionPressed("MoveLeft")) {
    CCLOG("Player wants to move left");
}

// 鼠标输入
if (input.isMousePressed()) {
    Vec2 mousePos = input.getMouseWorldPosition();
    CCLOG("Mouse at: (%.1f, %.1f)", mousePos.x, mousePos.y);
}
```

## 🧩 组件说明

### 核心组件

| 组件名 | 用途 | 关键字段 |
|--------|------|----------|
| `PlayerTag` | 标识玩家实体 | - |
| `PlayerStatsComponent` | 属性（血、魔、防御等） | maxHealth, defense, moveSpeed |
| `PlayerMovementComponent` | 移动状态 | velocity, isOnGround, isFacingRight |
| `PlayerEquipmentComponent` | 装备槽位 | helmet, chestplate, accessories[] |
| `PlayerHotbarComponent` | 快捷栏 | slots[10], selectedIndex |
| `PlayerAnimationComponent` | 动画状态 | currentState, animationTime |
| `PlayerBuffComponent` | Buff效果 | activeBuffs[] |
| `PlayerAbilityComponent` | 特殊能力 | hasWings, hasHook, isMounted |
| `PlayerCombatComponent` | 战斗状态 | isAttacking, attackCooldown |
| `PlayerSpriteComponent` | 渲染相关 | sprite, healthBarFill |

### 组件使用示例

```cpp
// 获取玩家组件
auto& stats = _world.getComponent<ecs::PlayerStatsComponent>(_playerEntity);
auto& movement = _world.getComponent<ecs::PlayerMovementComponent>(_playerEntity);

// 修改属性
stats.currentHealth -= 10.0f; // 受伤
stats.defense += 5;            // 增加防御

// 检查状态
if (movement.isOnGround) {
    CCLOG("Player is on ground");
}

// 添加Buff
auto& buffComp = _world.getComponent<ecs::PlayerBuffComponent>(_playerEntity);
ecs::PlayerBuffComponent::Buff speedBuff;
speedBuff.buffId = 1;
speedBuff.duration = 10.0f;
speedBuff.moveSpeedMultiplier = 1.5f; // 移速+50%
buffComp.addBuff(speedBuff);
```

## 🎯 待实现的系统

按优先级排序：

### 1. PlayerInputSystem（优先级：最高）
- 读取输入，更新 `PlayerMovementComponent`
- 处理快捷栏切换、物品使用

### 2. PlayerMovementSystem（优先级：高）
- 根据输入更新玩家速度
- 处理加速、摩擦力、跳跃
- 与 Cocos2d 物理引擎同步

### 3. PlayerAnimationSystem（优先级：中）
- 根据状态切换动画
- 更新精灵方向（左右翻转）
- 播放动作动画

### 4. PlayerEquipmentSystem（优先级：中）
- 计算装备加成
- 应用到 `PlayerStatsComponent`
- 检查套装效果

### 5. PlayerCombatSystem（优先级：中）
- 处理攻击逻辑
- 生成弹幕/挥舞动画
- 处理受伤和无敌帧

### 6. PlayerBuffSystem（优先级：低）
- 更新Buff时间
- 应用Buff效果到属性

### 7. PlayerAbilitySystem（优先级：低）
- 钩爪、翅膀、坐骑等特殊能力
- 水下呼吸、掉落伤害

## 🔗 与现有系统集成

### 与物品栏系统集成

```cpp
// 从背包装备物品
void equipItem(int inventorySlotIndex) {
    auto& inventory = Inventory::getInstance();
    auto slot = inventory.getSlot(inventorySlotIndex);

    if (slot.itemId == 0) return;

    auto& equipment = _world.getComponent<ecs::PlayerEquipmentComponent>(_playerEntity);

    // 装备到对应槽位
    // ...
}

// 使用快捷栏物品
void useHotbarItem() {
    auto& hotbar = _world.getComponent<ecs::PlayerHotbarComponent>(_playerEntity);
    int inventoryIndex = hotbar.getCurrentInventoryIndex();

    auto& inventory = Inventory::getInstance();
    auto slot = inventory.getSlot(inventoryIndex);

    // 使用物品
    // ...
}
```

### 与合成系统集成

```cpp
// 检查玩家附近的工作台
void checkNearbyStations() {
    auto& transform = _world.getComponent<ecs::TransformComponent>(_playerEntity);
    Vec2 playerPos(transform.x, transform.y);

    // 使用 StationDetector 检测附近工作台
    auto& detector = StationDetector::getInstance();
    auto nearbyStations = detector.detectStations(playerPos, 200.0f);

    // 更新合成界面可用配方
    // ...
}
```

## 📝 配置文件格式

### player_config.json（示例）

```json
{
  "defaultStats": {
    "maxHealth": 100,
    "maxMana": 20,
    "defense": 0,
    "moveSpeed": 200,
    "jumpHeight": 400,
    "healthRegen": 0.5,
    "manaRegen": 0.2
  },

  "movement": {
    "accelerationRate": 1500,
    "maxHorizontalSpeed": 250,
    "friction": 0.85,
    "airResistance": 0.95
  },

  "combat": {
    "invincibleDuration": 0.6,
    "critChance": 0.04,
    "critMultiplier": 2.0
  },

  "animations": {
    "idle": {
      "frames": ["player/idle_0.png", "player/idle_1.png"],
      "frameTime": 0.2
    },
    "walk": {
      "frames": ["player/walk_0.png", "player/walk_1.png", "player/walk_2.png"],
      "frameTime": 0.15
    },
    "jump": {
      "frames": ["player/jump_0.png"],
      "frameTime": 0.2
    }
  }
}
```

## 🐛 调试技巧

### 1. 可视化玩家状态

```cpp
// 在屏幕上显示调试信息
auto debugLabel = Label::createWithSystemFont("", "Arial", 16);
debugLabel->setPosition(Vec2(100, 500));
this->addChild(debugLabel, 1000);

this->schedule([this, debugLabel](float dt) {
    auto& stats = _world.getComponent<ecs::PlayerStatsComponent>(_playerEntity);
    auto& movement = _world.getComponent<ecs::PlayerMovementComponent>(_playerEntity);

    std::string info = StringUtils::format(
        "HP: %.0f/%.0f\n"
        "Defense: %d\n"
        "Velocity: (%.1f, %.1f)\n"
        "On Ground: %s\n"
        "Facing: %s",
        stats.currentHealth, stats.maxHealth,
        stats.defense,
        movement.velocity.x, movement.velocity.y,
        stats.isOnGround ? "Yes" : "No",
        movement.isFacingRight ? "Right" : "Left"
    );

    debugLabel->setString(info);
}, 0.1f, "update_debug_label");
```

### 2. 启用物理调试绘制

```cpp
// 在创建场景时
auto scene = Scene::createWithPhysics();
scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);
```

## 🎮 默认按键映射

| 动作 | 默认按键 | 说明 |
|------|---------|------|
| MoveLeft | A | 左移 |
| MoveRight | D | 右移 |
| Jump | Space | 跳跃 |
| UseItem | Left Ctrl | 使用物品 |
| Hook | E | 发射钩爪 |
| QuickHeal | H | 快速治疗 |
| QuickMana | M | 快速回魔 |
| Mount | R | 召唤/卸载坐骑 |
| Inventory | Escape | 打开背包 |

可以通过 `PlayerInput::setKeyMapping()` 自定义按键。

## 📚 参考资料

- [PlayerSystemDesign.md](PlayerSystemDesign.md) - 完整系统设计文档
- [泰拉瑞亚Wiki](https://terraria.fandom.com/wiki/Player) - 玩家机制参考
- [EnTT文档](https://github.com/skypjack/entt) - ECS框架文档

## ⚠️ 注意事项

1. **ECS适配**：当前代码需要根据你的ECS实现调整（`World::createEntity`, `World::addComponent`等）
2. **坐标系统**：确保Cocos2d坐标和物理引擎坐标一致
3. **性能优化**：避免每帧创建/销毁组件，使用标志位
4. **存档系统**：定期序列化玩家数据到文件

## 🔧 下一步工作

1. ✅ 完成组件定义
2. ✅ 完成输入系统
3. ✅ 完成工厂类
4. ⏳ 实现 PlayerSystems.h/cpp（核心系统）
5. ⏳ 实现动画管理
6. ⏳ 集成到游戏场景
7. ⏳ 测试与调试

---

**开发建议**：先实现输入系统和移动系统，让玩家能动起来，再逐步添加其他功能。
