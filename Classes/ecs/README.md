# ECS 框架使用指南

## 概述

重构后的 ECS 框架提供了更灵活、模块化的实体创建方式。主要改进包括：

1. **EntityHandle** - 便捷的实体操作接口
2. **EntityBuilder** - 链式调用创建实体
3. **ComponentPresets** - 预设系统，快速创建相似实体
4. **EntityFactory** - 统一的实体工厂
5. **NodeEntityMap** - O(1) 的 Node 到 Entity 查找

## 快速开始

### 方式1: 使用 EntityHandle（推荐）

```cpp
#include "ecs/ECS.h"

ecs::World world;

// 创建实体并链式添加组件
auto player = world.entity("Player")
    .add<TransformComponent>(100, 200)
    .add<HealthComponent>(100)
    .add<MovementComponent>(150.0f);

// 获取和修改组件
if (auto* health = player.get<HealthComponent>()) {
    health->takeDamage(10);
}

// 条件操作
player.ifHas<MovementComponent>([](MovementComponent& m) {
    m.moveSpeed *= 1.5f;  // 加速
});
```

### 方式2: 使用 EntityBuilder

```cpp
auto entity = ecs::EntityBuilder(world)
    .withTag("Enemy")
    .with<TransformComponent>(x, y)
    .with<HealthComponent>(50)
    .configure<AggroComponent>([](AggroComponent& a) {
        a.aggroRange = 300.0f;
        a.deaggroRange = 400.0f;
        a.targetTag = "Player";
    })
    .build();
```

### 方式3: 使用 EntityFactory（最灵活）

```cpp
#include "EntityFactory.h"

auto& factory = EntityFactory::getInstance();

// 使用预设快速创建
auto slime = factory.createFromPreset(world, "GreenSlime", x, y, parentNode);

// 或使用构建器自定义
auto custom = factory.create(world)
    .at(x, y)
    .withSprite("Minor monster/GreenSlime", "Green_Slime", 2)
    .withPhysics(35, 1.0f)
    .withHealth(100)
    .withAggro(300, 400)
    .withJumpMovement(2.0f, 300, 500)
    .withCombat(50, 10, 1.0f)
    .withGroundDetector()
    .asEnemy("Slime")
    .attachTo(parentNode)
    .build();
```

### 方式4: 使用预设系统

```cpp
#include "ecs/ComponentPresets.h"

// 使用内置预设
auto preset = ecs::Presets::greenSlime();

// 或自定义预设
auto customPreset = ecs::EntityPreset("CustomEnemy")
    .health(100)
    .aggro(400, 500)
    .jumpMovement(1.5f, 400, 600)
    .combat(60, 15, 0.8f)
    .enemyTag("Boss");

// 注册预设
ecs::PresetRegistry::getInstance().registerPreset("CustomEnemy", customPreset);

// 应用预设
ecs::EntityId entity = world.createEntity();
world.addComponent<TransformComponent>(entity, x, y);
customPreset.applyTo(world, entity);
```

## 组件说明

### 核心组件

| 组件 | 用途 |
|------|------|
| `TransformComponent` | 位置、旋转、缩放 |
| `SpriteComponent` | 精灵渲染（新，纯数据） |
| `SlimeSpriteComponent` | 史莱姆精灵（兼容旧代码） |
| `HealthComponent` | 生命值管理 |
| `MovementComponent` | 移动参数 |

### 游戏逻辑组件

| 组件 | 用途 |
|------|------|
| `AggroComponent` | 仇恨系统 |
| `JumpMovementComponent` | 跳跃移动 |
| `GroundDetectorComponent` | 地面检测 |
| `CombatComponent` | 战斗参数 |
| `EnemyTag` / `PlayerTag` | 实体标记 |

### 物理组件

| 组件 | 用途 |
|------|------|
| `PhysicsBodyConfig` | 物理体配置 |
| `ColliderComponent` | 碰撞体 |
| `RigidbodyComponent` | 刚体 |

## 碰撞检测优化

使用 `NodeEntityMap` 实现 O(1) 的 Node 到 Entity 查找：

```cpp
// 在碰撞回调中
Node* node = contact.getShapeA()->getBody()->getNode();
ecs::EntityId entity = ecs::NodeEntityMap::getInstance().findEntity(node);

if (entity != ecs::INVALID_ENTITY) {
    auto* health = world.getComponent<HealthComponent>(entity);
    if (health) {
        health->takeDamage(10);
    }
}
```

## 自定义组件

创建新组件只需继承 `IComponent`：

```cpp
struct MyCustomComponent : public ecs::IComponent {
    float customValue = 0.0f;
    std::string customName;
    
    MyCustomComponent() = default;
    MyCustomComponent(float val, const std::string& name) 
        : customValue(val), customName(name) {}
};

// 使用
entity.add<MyCustomComponent>(42.0f, "MyEntity");
```

## 自定义系统

```cpp
class MyCustomSystem : public ecs::ISystem {
public:
    const char* getName() const override { return "MyCustomSystem"; }
    int getPriority() const override { return ecs::SystemPriority::AI + 10; }
    
    void update(float delta) override {
        _world->forEach<MyCustomComponent, TransformComponent>(
            [delta](ecs::EntityId entity, MyCustomComponent& custom,
                    TransformComponent& transform) {
                // 处理逻辑
            });
    }
};

// 注册系统
world.addSystem<MyCustomSystem>();
```

## 最佳实践

1. **组件应该是纯数据** - 逻辑放在 System 中
2. **使用预设减少重复代码** - 相似实体使用相同预设
3. **使用 EntityHandle 进行链式操作** - 代码更简洁
4. **注册 Node 到 EntityMap** - 优化碰撞检测性能
5. **使用 configure() 进行复杂配置** - 比多个 with() 更清晰
