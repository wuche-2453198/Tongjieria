# 🎯 Player 系统 - 下一步实现清单

## ✅ 已完成

1. **组件定义** (`PlayerComponents.h`)
   - ✅ PlayerTag
   - ✅ PlayerStatsComponent
   - ✅ PlayerMovementComponent
   - ✅ PlayerEquipmentComponent
   - ✅ PlayerHotbarComponent
   - ✅ PlayerAnimationComponent
   - ✅ PlayerBuffComponent
   - ✅ PlayerAbilityComponent
   - ✅ PlayerCombatComponent
   - ✅ PlayerSpriteComponent

2. **输入系统** (`PlayerInput.h/cpp`)
   - ✅ 键盘输入监听
   - ✅ 鼠标输入监听
   - ✅ 按键状态查询（持续/刚按下/刚释放）
   - ✅ 动作映射系统
   - ✅ 单帧状态重置

3. **工厂类** (`PlayerFactory.h/cpp`)
   - ✅ 创建玩家实体骨架
   - ✅ 创建玩家精灵
   - ✅ 添加物理体
   - ✅ 加载配置方法（待实现具体逻辑）

4. **文档**
   - ✅ 完整系统设计文档 (`PlayerSystemDesign.md`)
   - ✅ 使用指南 (`README.md`)
   - ✅ 测试场景示例 (`PlayerTestScene_Example.h/cpp`)

---

## 🔧 待实现 - 核心系统

### 1. PlayerSystems.h/cpp（最重要！）

需要实现以下系统：

#### 1.1 PlayerInputSystem（优先级：★★★★★）
```cpp
class PlayerInputSystem : public ecs::System {
    // 读取 PlayerInput，更新 PlayerMovementComponent
    // 处理快捷栏切换、物品使用请求
};
```

**实现要点**：
- 读取 `PlayerInput::getInstance()` 的状态
- 更新 `PlayerMovementComponent` 的 `isMovingLeft/Right/wantsToJump`
- 处理鼠标滚轮切换快捷栏
- 处理数字键1-9切换快捷栏
- 优先级：0（最先执行）

#### 1.2 PlayerMovementSystem（优先级：★★★★★）
```cpp
class PlayerMovementSystem : public ecs::System {
    // 根据 PlayerMovementComponent，更新速度和位置
    // 处理加速、摩擦力、跳跃
    // 与 Cocos2d 物理引擎同步
};
```

**实现要点**：
- 泰拉瑞亚风格的加速移动（不是匀速）
- 地面摩擦力 vs 空中阻力
- 跳跃逻辑（地面跳+空中二段跳）
- 限制最大水平速度
- 同步物理体速度
- 优先级：10

#### 1.3 PlayerGroundDetectionSystem（优先级：★★★★☆）
```cpp
class PlayerGroundDetectionSystem : public ecs::System {
    // 检测玩家是否在地面上
    // 更新 PlayerStatsComponent::isOnGround
};
```

**实现要点**：
- 使用射线检测或物理碰撞检测
- 更新 `isOnGround` 标志
- 重置 `currentJumpCount`（着地后）
- 优先级：5

#### 1.4 PlayerAnimationSystem（优先级：★★★☆☆）
```cpp
class PlayerAnimationSystem : public ecs::System {
    // 根据状态更新动画
    // 更新精灵方向（左右翻转）
};
```

**实现要点**：
- 根据速度、是否在地面等判断动画状态
- 播放对应动画（IDLE/WALK/JUMP/FALL）
- 翻转精灵（setFlippedX）
- 优先级：100

#### 1.5 PlayerEquipmentSystem（优先级：★★★☆☆）
```cpp
class PlayerEquipmentSystem : public ecs::System {
    // 计算装备属性加成
    // 应用到 PlayerStatsComponent
};
```

**实现要点**：
- 遍历所有装备槽位
- 从 `ItemManager` 查询物品属性
- 累加防御、速度、伤害加成等
- 检查套装效果
- 优先级：8

#### 1.6 PlayerBuffSystem（优先级：★★☆☆☆）
```cpp
class PlayerBuffSystem : public ecs::System {
    // 更新Buff持续时间
    // 应用Buff效果到属性
};
```

**实现要点**：
- 更新所有Buff的 `duration -= dt`
- 移除过期Buff
- 计算总修正值（速度、伤害等）
- 优先级：7

#### 1.7 PlayerCombatSystem（优先级：★★★☆☆）
```cpp
class PlayerCombatSystem : public ecs::System {
    // 处理玩家攻击
    // 处理受伤和无敌帧
};
```

**实现要点**：
- 检测攻击输入（鼠标左键/Ctrl键）
- 根据当前武器生成弹幕或挥舞动画
- 更新无敌帧计时器
- 处理击退效果
- 优先级：20

#### 1.8 PlayerHealthSystem（优先级：★★★☆☆）
```cpp
class PlayerHealthSystem : public ecs::System {
    // 处理生命回复
    // 检查死亡
    // 更新UI
};
```

**实现要点**：
- 每秒回复 `healthRegen` 点生命
- 检查 `currentHealth <= 0` 触发死亡
- 更新血条UI（`PlayerSpriteComponent::healthBarFill`）
- 优先级：30

#### 1.9 PlayerAbilitySystem（优先级：★★☆☆☆）
```cpp
class PlayerAbilitySystem : public ecs::System {
    // 处理钩爪、翅膀、坐骑等特殊能力
};
```

**实现要点**：
- 钩爪发射与拉力
- 翅膀飞行时间管理
- 坐骑速度加成
- 水下呼吸计时
- 优先级：15

---

## 📝 实现步骤建议

### 阶段1：让玩家动起来（1-2小时）

1. **适配 ECS 框架**
   - 确认你的 `ecs::World` 有 `createEntity()`, `addComponent<T>()`, `getComponent<T>()` 等接口
   - 如果使用 EnTT，参考你的 `MonsterFactory` 实现
   - 在 `PlayerFactory.cpp` 中取消注释并适配

2. **实现 PlayerInputSystem**
   ```cpp
   // PlayerSystems.h
   class PlayerInputSystem : public ecs::System {
   public:
       void update(float dt) override;
       int getPriority() const override { return 0; }
   };
   ```

3. **实现 PlayerMovementSystem**
   - 先实现简单的水平移动
   - 再添加跳跃逻辑
   - 最后添加加速度和摩擦力

4. **测试**
   - 在 `PlayerTestScene` 中创建玩家实体
   - 注册输入和移动系统
   - 运行游戏，测试 WASD 和 Space 移动

### 阶段2：完善基础功能（2-3小时）

5. **实现 PlayerGroundDetectionSystem**
   - 使用射线检测或碰撞回调
   - 更新 `isOnGround` 标志

6. **实现 PlayerAnimationSystem**
   - 准备玩家精灵图集（idle/walk/jump）
   - 根据状态播放动画

7. **实现 PlayerHealthSystem**
   - 添加血条UI
   - 测试受伤和回血

### 阶段3：装备与战斗（2-3小时）

8. **实现 PlayerEquipmentSystem**
   - 与 `ItemManager` 集成
   - 测试装备铜套装，查看防御变化

9. **实现 PlayerCombatSystem**
   - 实现简单的近战攻击（生成碰撞体）
   - 实现受伤和无敌帧

### 阶段4：高级功能（3-4小时）

10. **实现 PlayerBuffSystem**
    - 测试速度Buff、防御Buff

11. **实现 PlayerAbilitySystem**
    - 钩爪（可选）
    - 翅膀飞行（可选）

12. **与现有系统集成**
    - ✅ 连接背包系统 (PlayerInventoryIntegration)
    - ✅ 连接合成系统 (PlayerCraftingSystem - 2025-12-29)
    - 测试完整游戏流程

---

## 🔌 ECS 框架适配

根据你的 `MonsterFactory` 代码，你的 ECS 使用方式大概是：

```cpp
// 创建实体
ecs::EntityId entity = world.createEntity();

// 添加组件
world.addComponent<ecs::TransformComponent>(entity, x, y);
world.addComponent<ecs::PlayerTag>(entity);

// 获取组件
auto& stats = world.getComponent<ecs::PlayerStatsComponent>(entity);

// 注册节点映射
ecs::NodeEntityMap::getInstance().registerNode(sprite, entity);
```

你需要在 `PlayerFactory.cpp` 中**取消注释**相关代码，并根据上述模式调整。

---

## 🎮 测试场景集成

### 方法1：使用现有场景

在 `ItemsTestScene` 或 `EcsTestScene` 中添加玩家：

```cpp
// ItemsTestScene.cpp
#include "player/PlayerFactory.h"

bool ItemsTestScene::init() {
    // ... 现有代码

    // 创建玩家
    Vec2 spawnPos(400, 300);
    _playerEntity = PlayerFactory::createPlayer(_world, spawnPos, this);

    // 注册玩家系统
    _world.addSystem<PlayerInputSystem>();
    _world.addSystem<PlayerMovementSystem>();
    // ...
}
```

### 方法2：创建新测试场景

将 `PlayerTestScene_Example.h/cpp` 重命名为 `PlayerTestScene.h/cpp`，并添加到 `CMakeLists.txt`：

```cmake
list(APPEND GAME_SOURCE
    # ... 现有代码
    Classes/player/PlayerInput.cpp
    Classes/player/PlayerFactory.cpp
    Classes/player/PlayerSystems.cpp  # 待创建
    Classes/player/PlayerTestScene.cpp
)
```

---

## 📊 完成度追踪

- [ ] 适配 ECS 框架到 `PlayerFactory`
- [ ] 实现 `PlayerInputSystem`
- [ ] 实现 `PlayerMovementSystem`
- [ ] 实现 `PlayerGroundDetectionSystem`
- [ ] 实现 `PlayerAnimationSystem`
- [ ] 实现 `PlayerEquipmentSystem`
- [ ] 实现 `PlayerCombatSystem`
- [ ] 实现 `PlayerHealthSystem`
- [ ] 实现 `PlayerBuffSystem`
- [ ] 实现 `PlayerAbilitySystem`
- [x] 与背包系统集成
- [x] 与合成系统集成 (2025-12-29)
- [ ] 完整测试

---

## 🐛 常见问题

### Q: 玩家不受物理引擎影响？
A: 检查场景是否使用 `Scene::createWithPhysics()`，并设置了重力。

### Q: 玩家穿墙？
A: 检查物理体的 `CategoryBitmask` 和 `CollisionBitmask` 是否正确设置。

### Q: 跳跃高度不对？
A: 调整 `PlayerStatsComponent::jumpHeight`（推荐值：300-500）。

### Q: 移动太快/太慢？
A: 调整 `PlayerStatsComponent::moveSpeed`（推荐值：150-250）。

### Q: 组件找不到？
A: 确保已调用 `world.addComponent<T>(entity)` 添加组件。

---

## 📞 需要帮助？

如果遇到问题，可以：
1. 查看 `PlayerSystemDesign.md` 了解设计思路
2. 参考 `MonsterFactory.cpp` 中的 ECS 使用方式
3. 启用物理调试绘制：`scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL)`
4. 添加日志输出：`CCLOG("Player velocity: %.1f", movement.velocity.x)`

---

**祝你开发顺利！🚀**
