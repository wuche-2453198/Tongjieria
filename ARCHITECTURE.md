# Tongjieria 项目架构文档

## 目录

1. [项目概述](#1-项目概述)
2. [目录结构](#2-目录结构)
3. [技术栈](#3-技术栈)
4. [ECS架构详解](#4-ecs架构详解)
5. [组件系统](#5-组件系统)
6. [系统系统](#6-系统系统)
7. [工厂模式与配置驱动](#7-工厂模式与配置驱动)
8. [物理与碰撞系统](#8-物理与碰撞系统)
9. [场景管理](#9-场景管理)
10. [游戏流程](#10-游戏流程)
11. [数据流图](#11-数据流图)
12. [扩展指南](#12-扩展指南)

---

## 1. 项目概述

**Tongjieria** 是一个基于 **Cocos2d-x** 游戏引擎和 **EnTT** ECS库开发的2D怪物AI演示项目。项目展示了多种怪物类型的AI行为实现，包括：

- **史莱姆 (Slime)** - 跳跃式移动，追踪玩家
- **僵尸 (Zombie)** - 行走式移动，智能跳跃障碍
- **恶魔眼 (DemonEye)** - 飞行追踪，弧形回弹

核心设计理念：
- **数据驱动**：怪物属性通过JSON配置文件定义
- **ECS架构**：组件存储数据，系统处理逻辑，实体作为ID
- **工厂模式**：统一的怪物创建接口

---

## 2. 目录结构

```
Tongjieria/
├── Classes/                    # 源代码目录
│   ├── AppDelegate.cpp/h       # 应用入口
│   ├── ecs/                    # ECS框架核心
│   │   ├── Entity.h            # 实体ID定义
│   │   ├── Components.h        # 所有组件定义
│   │   ├── SystemsEntt.h       # 所有System实现
│   │   ├── ECS.h               # ECS统一头文件
│   │   ├── SpriteComponent.h   # NodeEntityMap工具类
│   │   ├── PhysicsContactHandler.h  # 物理碰撞处理器
│   │   └── README.md           # ECS使用指南
│   ├── MonsterFactory.cpp/h    # 怪物工厂（核心）
│   ├── SlimeTestScene.cpp/h    # 史莱姆测试场景
│   ├── ZombieTestScene.cpp/h   # 僵尸测试场景
│   ├── DemonEyeTestScene.cpp/h # 恶魔眼测试场景
│   ├── MainMenuScene.cpp/h     # 主菜单场景
│   └── SplashScene.cpp/h       # 启动画面
├── Resources/                  # 资源目录
│   ├── config/                 # JSON配置文件
│   │   ├── slimes/             # 史莱姆配置（12种）
│   │   └── zombies/            # 僵尸配置（9种）
│   ├── Minor monster/          # 怪物贴图
│   ├── fonts/                  # 字体文件
│   └── music/                  # 音效文件
├── third_party/
│   └── entt/                   # EnTT ECS库
└── cocos2d/                    # Cocos2d-x引擎
```

---

## 3. 技术栈

| 技术 | 版本/说明 | 用途 |
|------|----------|------|
| **Cocos2d-x** | 3.x | 2D游戏引擎，提供渲染、物理、音频等 |
| **EnTT** | 3.x | 高性能ECS库，实体-组件-系统架构 |
| **RapidJSON** | (内置) | JSON解析，用于配置文件加载 |
| **C++17** | | 编程语言标准 |
| **CMake** | | 构建系统 |

---

## 4. ECS架构详解

### 4.1 什么是ECS

**ECS (Entity-Component-System)** 是一种面向数据的架构模式：

```
┌─────────────────────────────────────────────────────────────┐
│                         ECS 架构                             │
├─────────────────────────────────────────────────────────────┤
│  Entity (实体)     │  只是一个ID (uint32_t)                  │
│  Component (组件)  │  纯数据结构，不包含逻辑                  │
│  System (系统)     │  处理特定组件组合的逻辑                  │
└─────────────────────────────────────────────────────────────┘
```

### 4.2 本项目的ECS实现

```cpp
// Entity - 简单的ID类型
using EntityId = uint32_t;
constexpr EntityId INVALID_ENTITY = std::numeric_limits<EntityId>::max();

// 使用EnTT的registry管理实体和组件
entt::registry _registry;

// 创建实体
auto entity = _registry.create();

// 添加组件
_registry.emplace<TransformComponent>(entity, 100.0f, 200.0f);
_registry.emplace<HealthComponent>(entity, 50.0f);

// System查询组件
auto view = _registry.view<TransformComponent, HealthComponent>();
view.each([](auto entity, TransformComponent& t, HealthComponent& h) {
    // 处理逻辑
});
```

### 4.3 为什么选择EnTT

1. **高性能**：基于稀疏集(Sparse Set)实现，O(1)组件访问
2. **类型安全**：编译期类型检查
3. **灵活性**：支持任意C++类型作为组件
4. **零开销抽象**：不使用虚函数，内联优化

---

## 5. 组件系统

### 5.1 组件分类

项目中的组件按功能分为以下几类：

#### 5.1.1 基础组件

| 组件 | 文件位置 | 用途 |
|------|----------|------|
| `TransformComponent` | Components.h:17 | 位置、旋转、缩放、速度 |
| `HealthComponent` | Components.h:285 | 生命值、无敌时间、死亡状态 |
| `LifetimeComponent` | Components.h:350 | 自动销毁计时 |

```cpp
struct TransformComponent {
  cocos2d::Vec2 position = cocos2d::Vec2::ZERO;
  float rotation = 0.0f;
  cocos2d::Vec2 scale = cocos2d::Vec2(1.0f, 1.0f);
  cocos2d::Vec2 velocity = cocos2d::Vec2::ZERO;
  EntityId parent = INVALID_ENTITY;
};
```

#### 5.1.2 渲染组件

| 组件 | 用途 |
|------|------|
| `SlimeSpriteComponent` | 史莱姆专用精灵，内置帧动画 |
| `MonsterSpriteComponent` | 通用怪物精灵，支持自定义帧序列 |
| `ProjectileSpriteComponent` | 投射物精灵 |

```cpp
struct SlimeSpriteComponent {
  cocos2d::Sprite *sprite = nullptr;
  std::string slimeType = "Green";
  cocos2d::Vector<cocos2d::SpriteFrame *> animFrames;
  float frameTime = 0.15f;
  bool facingRight = true;
  // ... 更多属性
};
```

#### 5.1.3 AI组件

| 组件 | 用途 |
|------|------|
| `AggroComponent` | 仇恨系统，目标检测与追踪 |
| `GroundDetectorComponent` | 地面检测，静止判断 |
| `JumpMovementComponent` | 跳跃式移动（史莱姆） |
| `WarriorMovementComponent` | 行走式移动（僵尸） |
| `DemonEyeMovementComponent` | 飞行追踪（恶魔眼） |

```cpp
struct AggroComponent {
  EntityId targetEntity = INVALID_ENTITY;
  float aggroRange = 500.0f;    // 进入仇恨距离
  float deaggroRange = 600.0f;  // 脱离仇恨距离
  bool hasAggro = false;
  float distanceToTarget = 99999.0f;
  cocos2d::Vec2 directionToTarget;
  std::string targetTag = "Player";
};
```

#### 5.1.4 战斗组件

| 组件 | 用途 |
|------|------|
| `CombatComponent` | 攻击参数（范围、伤害、冷却） |
| `ProjectileAttackComponent` | 投射物攻击配置 |
| `ProjectileComponent` | 投射物实体属性 |
| `DebuffComponent` | 减益效果（冷冻、冰冻、中毒） |

#### 5.1.5 标记组件 (Tag)

| 组件 | 用途 |
|------|------|
| `PlayerTag` | 标记玩家实体 |
| `EnemyTag` | 标记敌人实体 |

```cpp
struct PlayerTag {
  char _dummy = 0;  // EnTT需要非空结构体
};
```

### 5.2 组件设计原则

1. **纯数据**：组件只存储数据，不包含业务逻辑
2. **移动语义**：禁止拷贝，允许移动（管理cocos2d资源）
3. **默认值**：所有字段都有合理的默认值
4. **自包含**：组件可以包含辅助方法（如计算函数）

---

## 6. 系统系统

### 6.1 System基类

```cpp
class ISystemEntt {
protected:
    entt::registry* _registry = nullptr;

public:
    virtual const char* getName() const = 0;
    virtual int getPriority() const { return 0; }
    virtual void update(float delta) = 0;
    void setRegistry(entt::registry* registry);
};
```

### 6.2 系统优先级

系统按优先级顺序执行，数值越小越先执行：

```cpp
namespace SystemPriority {
    constexpr int INPUT = 0;      // 输入处理
    constexpr int PHYSICS = 100;  // 物理更新
    constexpr int COLLISION = 200;// 碰撞检测
    constexpr int AI = 300;       // AI决策
    constexpr int MOVEMENT = 400; // 移动执行
    constexpr int ANIMATION = 500;// 动画更新
    constexpr int RENDER = 600;   // 渲染同步
}
```

### 6.3 系统列表

#### 6.3.1 AI系统

| 系统 | 优先级 | 功能 |
|------|--------|------|
| `AggroSystemEntt` | AI-10 | 仇恨检测，查找目标，计算距离和方向 |
| `DebuffSystemEntt` | AI-5 | 更新减益效果计时，应用毒素伤害 |

**AggroSystem工作流程：**
```
1. 遍历所有有AggroComponent的实体
2. 根据targetTag查找目标实体（如"Player"）
3. 计算与目标的距离和方向
4. 根据aggroRange/deaggroRange切换仇恨状态
```

#### 6.3.2 移动系统

| 系统 | 优先级 | 功能 |
|------|--------|------|
| `JumpMovementSystemEntt` | MOVEMENT-10 | 史莱姆跳跃移动 |
| `WarriorAISystemEntt` | MOVEMENT | 僵尸行走AI |
| `DemonEyeAISystemEntt` | MOVEMENT | 恶魔眼飞行AI |

**JumpMovementSystem工作流程：**
```
1. 检查地面状态和跳跃冷却
2. 如果有仇恨目标：计算智能追踪冲量
3. 如果无目标：计算随机巡逻冲量
4. 应用物理冲量，播放跳跃动画
5. 重置冷却计时器
```

**WarriorAISystem特性：**
- 行走追踪玩家
- 智能跳跃障碍物
- 跟随玩家跳跃反应
- 连续失败后退重试

#### 6.3.3 物理系统

| 系统 | 优先级 | 功能 |
|------|--------|------|
| `GroundDetectorSystemEntt` | PHYSICS+10 | 史莱姆地面检测，动态阻尼 |
| `MonsterGroundDetectorSystemEntt` | PHYSICS+10 | 通用怪物地面检测 |
| `SlowFallSystemEntt` | PHYSICS+1 | 缓降效果（伞史莱姆） |
| `ProjectileSystemEntt` | PHYSICS+5 | 投射物更新 |

#### 6.3.4 渲染系统

| 系统 | 优先级 | 功能 |
|------|--------|------|
| `SlimeSyncSystemEntt` | RENDER-10 | 同步物理体位置到Transform |
| `MonsterSyncSystemEntt` | RENDER-10 | 通用怪物位置同步 |
| `SlimeRenderSystemEntt` | RENDER | 史莱姆渲染，帧动画播放 |
| `MonsterAnimationSystemEntt` | ANIMATION | 通用怪物帧动画 |

#### 6.3.5 战斗系统

| 系统 | 优先级 | 功能 |
|------|--------|------|
| `HealthSystemEntt` | COLLISION+50 | 无敌时间更新 |
| `CombatSystemEntt` | COLLISION+100 | 攻击冷却更新 |
| `LifetimeSystemEntt` | COLLISION+150 | 限时实体销毁 |
| `ProjectileAttackSystemEntt` | AI+10 | 投射物发射逻辑 |

### 6.4 SystemManager

```cpp
class SystemManagerEntt {
private:
    entt::registry* _registry = nullptr;
    std::vector<std::unique_ptr<ISystemEntt>> _systems;

public:
    void setRegistry(entt::registry* registry);
    
    template<typename T, typename... Args>
    T* addSystem(Args&&... args);
    
    void update(float delta);  // 按优先级顺序调用所有系统
    size_t getSystemCount() const;
    void clear();
};
```

---

## 7. 工厂模式与配置驱动

### 7.1 MonsterFactory

`MonsterFactory` 是项目的核心类，负责：
1. 加载JSON配置文件
2. 解析配置到 `MonsterConfig` 结构
3. 根据配置创建EnTT实体和组件

```cpp
class MonsterFactory {
public:
    static MonsterFactory& getInstance();
    
    // 加载配置
    bool loadConfig(const std::string& configPath);
    int loadConfigsFromDir(const std::string& dirPath);
    bool loadSingleConfig(const std::string& filePath);
    
    // 创建怪物
    ecs::EntityId createMonster(entt::registry& registry,
                                const std::string& monsterId,
                                float x, float y,
                                cocos2d::Node* parentNode);
    
    // 查询
    const MonsterConfig* getConfig(const std::string& monsterId) const;
    std::vector<std::string> getAllMonsterIds() const;
    std::vector<std::string> getMonsterIdsByType(const std::string& type) const;

private:
    std::unordered_map<std::string, MonsterConfig> _configs;
};
```

### 7.2 MonsterConfig结构

```cpp
struct MonsterConfig {
  std::string id;    // 怪物ID (如 "GreenSlime")
  std::string type;  // 怪物类型 (如 "Slime", "Zombie", "DemonEye")
  
  struct Display {
    std::string spriteFolder;    // 贴图目录
    std::string spritePrefix;    // 贴图前缀
    int frameCount = 2;          // 帧数
    std::vector<int> frameSequence;  // 帧序列（如{1,2,3,2}）
    float frameTime = 0.15f;     // 帧间隔
    float scale = 1.0f;          // 缩放
  } display;
  
  struct Physics {
    float bodyWidth = 40.0f;
    float bodyHeight = 25.0f;
    float mass = 1.0f;
    float friction = 1.0f;
    float restitution = 0.0f;
    int collisionGroup = -1;
    bool useGravity = true;
  } physics;
  
  struct Movement {
    std::string type = "jump";   // "jump", "walk", "fly"
    float jumpCooldown = 2.0f;
    float horizontalImpulse = 300.0f;
    float verticalImpulse = 500.0f;
    // ... 更多参数
  } movement;
  
  struct AI { ... } ai;
  struct Stats { ... } stats;
  struct Projectile { ... } projectile;
  struct Debuff { ... } debuff;
  struct SlowFall { ... } slowFall;
  std::vector<LootItem> loot;
};
```

### 7.3 JSON配置示例

```json
// Resources/config/slimes/GreenSlime.json
{
  "id": "GreenSlime",
  "type": "Slime",
  "description": "基础史莱姆，无特殊能力",
  
  "display": {
    "spriteFolder": "Minor monster/GreenSlime",
    "spritePrefix": "Green_Slime",
    "frameCount": 2,
    "frameTime": 0.15,
    "scale": 1.0
  },
  
  "physics": {
    "bodyWidth": 30.0,
    "bodyHeight": 21.0,
    "mass": 1.0,
    "friction": 1.0,
    "restitution": 0.0,
    "collisionGroup": -1
  },
  
  "movement": {
    "type": "jump",
    "jumpCooldown": 2.5,
    "horizontalImpulse": 300.0,
    "verticalImpulse": 500.0,
    "patrolImpulseRatio": 0.5,
    "directionChangeChance": 0.3
  },
  
  "ai": {
    "behavior": "aggressive",
    "aggroRange": 300.0,
    "deaggroRange": 400.0,
    "targetTag": "Player"
  },
  
  "stats": {
    "maxHealth": 50.0,
    "attackDamage": 10.0,
    "attackRange": 50.0,
    "attackCooldown": 1.0
  },
  
  "loot": [
    {"itemId": "gel", "minCount": 1, "maxCount": 3, "dropChance": 0.8}
  ]
}
```

### 7.4 字符串到组件的映射

由于EnTT不支持运行时字符串到组件类型的映射，项目采用**工厂分支**方式：

```cpp
ecs::EntityId MonsterFactory::createMonster(...) {
    const MonsterConfig& cfg = _configs[monsterId];
    auto entity = registry.create();
    
    // 根据怪物类型选择精灵组件
    if (cfg.type == "Slime") {
        auto& spriteComp = registry.emplace<ecs::SlimeSpriteComponent>(entity);
        // 初始化...
    } else if (cfg.type == "Zombie") {
        auto& spriteComp = registry.emplace<ecs::MonsterSpriteComponent>(entity);
        // 初始化...
    } else if (cfg.type == "DemonEye") {
        auto& spriteComp = registry.emplace<ecs::MonsterSpriteComponent>(entity);
        // 初始化...
    }
    
    // 根据移动类型选择移动组件
    if (cfg.movement.type == "jump") {
        auto& jump = registry.emplace<ecs::JumpMovementComponent>(entity);
        // 从配置初始化参数...
    } else if (cfg.movement.type == "walk") {
        auto& walk = registry.emplace<ecs::WalkMovementComponent>(entity);
        // 从配置初始化参数...
    } else if (cfg.movement.type == "fly") {
        auto& fly = registry.emplace<ecs::DemonEyeMovementComponent>(entity);
        // 从配置初始化参数...
    }
    
    // 添加通用组件...
    return entityId;
}
```

**优点**：
- 编译期类型安全
- 无运行时反射开销
- 代码清晰易读

**缺点**：
- 添加新怪物类型需要修改工厂代码
- 组件类型在编译期确定

---

## 8. 物理与碰撞系统

### 8.1 Cocos2d-x物理引擎

项目使用Cocos2d-x内置的Chipmunk物理引擎：

```cpp
// 创建物理场景
auto scene = Scene::createWithPhysics();
auto physicsWorld = scene->getPhysicsWorld();
physicsWorld->setGravity(Vec2(0, -980));
physicsWorld->setSubsteps(8);  // 提高碰撞精度
```

### 8.2 碰撞层掩码

```cpp
namespace CollisionLayer {
    constexpr int NONE = 0;
    constexpr int PLAYER = 1 << 0;     // 1
    constexpr int ENEMY = 1 << 1;      // 2
    constexpr int GROUND = 1 << 2;     // 4
    constexpr int WALL = 1 << 3;       // 8
    constexpr int PROJECTILE = 1 << 4; // 16
    constexpr int ITEM = 1 << 5;       // 32
    constexpr int TRIGGER = 1 << 6;    // 64
    constexpr int ALL = 0xFFFFFFFF;
}
```

### 8.3 NodeEntityMap

用于O(1)查找Node对应的Entity：

```cpp
class NodeEntityMap {
public:
    static NodeEntityMap& getInstance();
    
    void registerNode(cocos2d::Node* node, EntityId entity);
    void unregisterNode(cocos2d::Node* node);
    EntityId findEntity(cocos2d::Node* node);

private:
    std::unordered_map<cocos2d::Node*, EntityId> _nodeToEntity;
};
```

**使用场景**：碰撞回调中根据Node查找Entity

```cpp
listener->onContactBegin = [&](PhysicsContact& contact) {
    Node* node = contact.getShapeA()->getBody()->getNode();
    EntityId entity = NodeEntityMap::getInstance().findEntity(node);
    if (entity != INVALID_ENTITY) {
        auto* health = registry.try_get<HealthComponent>(entity);
        if (health) health->takeDamage(10);
    }
    return true;
};
```

### 8.4 PhysicsContactHandler

封装通用的碰撞处理逻辑：

```cpp
class PhysicsContactHandler {
public:
    struct ContactInfo {
        PhysicsBody* dynamicBody;
        PhysicsBody* staticBody;
        Node* dynamicNode;
        Vec2 normal;
        bool isGroundContact;
        bool isWallContact;
    };

    static ContactInfo parseContact(PhysicsContact& contact);
    static void handleGroundContactBegin(registry&, const ContactInfo&);
    static void handleGroundContactSeparate(registry&, const ContactInfo&);
    static void handleWallPreSolve(const ContactInfo&, PhysicsContactPreSolve&);
    
    static EventListenerPhysicsContact* createContactListener(
        entt::registry& registry,
        std::function<bool(PhysicsContact&, const ContactInfo&)> customBeginHandler,
        std::function<void(PhysicsContact&, const ContactInfo&)> customSeparateHandler
    );
};
```

**地面检测原理**：
- 根据碰撞法线判断：`normal.y < -0.3` 表示地面接触
- 使用接触计数器处理同时接触多个地面的情况

---

## 9. 场景管理

### 9.1 场景层次

```
AppDelegate
    └── SplashScene (启动画面)
            └── MainMenuScene (主菜单)
                    ├── SlimeTestScene (史莱姆测试)
                    ├── ZombieTestScene (僵尸测试)
                    └── DemonEyeTestScene (恶魔眼测试)
```

### 9.2 测试场景结构

以 `SlimeTestScene` 为例：

```cpp
class SlimeTestScene : public cocos2d::Layer {
private:
    entt::registry _registry;              // EnTT实体注册表
    ecs::SystemManagerEntt _systemManager; // System管理器
    
    cocos2d::Sprite* _fakePlayer;          // 虚拟玩家
    ecs::EntityId _fakePlayerEntity;       // 玩家实体ID
    
public:
    static Scene* createScene();
    virtual bool init();
    virtual void update(float delta) override;
    
private:
    void setupEcsSystems();           // 初始化ECS系统
    void createPhysicsEnvironment();  // 创建物理环境（地面、平台）
    void createFakePlayerEntity();    // 创建玩家实体
    void createEcsSlime();            // 创建史莱姆实体
    void setupSharedContactListener();// 设置碰撞监听
    void setupKeyboardListener();     // 设置键盘控制
};
```

### 9.3 场景初始化流程

```
1. createScene()
   ├── 创建物理场景 Scene::createWithPhysics()
   ├── 设置重力 setGravity(0, -980)
   └── 添加Layer

2. init()
   ├── 创建背景和UI
   ├── createPhysicsEnvironment() - 创建地面、平台、墙壁
   ├── setupEcsSystems() - 加载配置，初始化System
   ├── setupSharedContactListener() - 设置碰撞监听
   ├── createFakePlayerEntity() - 创建玩家
   ├── createEcsSlime() - 创建怪物
   ├── setupKeyboardListener() - 键盘输入
   └── scheduleUpdate() - 开始更新循环

3. update(delta)
   ├── updateFakePlayerPosition() - 更新玩家位置
   └── _systemManager.update(delta) - 更新所有ECS系统
```

---

## 10. 游戏流程

### 10.1 主循环

```
┌─────────────────────────────────────────────────────────────┐
│                      每帧更新流程                            │
├─────────────────────────────────────────────────────────────┤
│  1. 输入处理 (键盘/触摸)                                     │
│  2. 物理引擎更新 (Cocos2d-x内部)                             │
│  3. ECS系统更新:                                            │
│     ├── PHYSICS系统: 地面检测、缓降                         │
│     ├── COLLISION系统: 生命值、战斗冷却                     │
│     ├── AI系统: 仇恨检测、减益效果                          │
│     ├── MOVEMENT系统: 跳跃/行走/飞行                        │
│     ├── ANIMATION系统: 帧动画更新                           │
│     └── RENDER系统: 位置同步、渲染属性                      │
│  4. Cocos2d渲染                                             │
└─────────────────────────────────────────────────────────────┘
```

### 10.2 怪物AI状态机

**史莱姆 (JumpMovement)：**
```
┌───────────────────────────────────────────────────────────┐
│                    史莱姆AI状态                            │
├───────────────────────────────────────────────────────────┤
│                                                           │
│   ┌─────────────┐                 ┌─────────────┐        │
│   │   巡逻      │  玩家进入范围   │   追踪      │        │
│   │  (随机跳跃)  │ ─────────────> │  (智能跳跃)  │        │
│   └─────────────┘                 └─────────────┘        │
│         ↑                               │                 │
│         │         玩家离开范围          │                 │
│         └───────────────────────────────┘                 │
│                                                           │
│   每次跳跃后进入冷却期，地面检测后可再次跳跃              │
└───────────────────────────────────────────────────────────┘
```

**僵尸 (WarriorMovement)：**
```
┌───────────────────────────────────────────────────────────┐
│                    僵尸AI状态                              │
├───────────────────────────────────────────────────────────┤
│                                                           │
│   ┌─────────────┐                 ┌─────────────┐        │
│   │   巡逻      │  玩家进入范围   │   追击      │        │
│   │  (行走)     │ ─────────────> │  (行走+跳跃) │        │
│   └─────────────┘                 └─────────────┘        │
│         ↑                               │                 │
│         │                               ↓                 │
│         │                         ┌─────────────┐        │
│         │    冷却结束/成功        │   后退重试   │        │
│         └──────────────────────── │  (多次失败)  │        │
│                                   └─────────────┘        │
│                                                           │
│   特殊行为：                                              │
│   - 障碍物检测 → 自动跳跃                                 │
│   - 玩家跳跃 → 反应跳跃（延迟0.1秒）                      │
│   - 连续跳跃失败 → 后退重试                               │
└───────────────────────────────────────────────────────────┘
```

**恶魔眼 (DemonEyeMovement)：**
```
┌───────────────────────────────────────────────────────────┐
│                  恶魔眼AI状态                              │
├───────────────────────────────────────────────────────────┤
│                                                           │
│   ┌─────────────┐                 ┌─────────────┐        │
│   │   巡逻      │  玩家进入范围   │   追踪      │        │
│   │  (飘荡)     │ ─────────────> │  (缓慢转向)  │        │
│   └─────────────┘                 └─────────────┘        │
│                                         │                 │
│                                   撞墙/被击退              │
│                                         ↓                 │
│                                   ┌─────────────┐        │
│                                   │  弧形回弹   │        │
│                                   │  (180°弧线) │        │
│                                   └─────────────┘        │
│                                                           │
│   特殊行为：                                              │
│   - 无重力飞行                                            │
│   - 缓慢转向（turnRate控制）                              │
│   - 直线加速，转弯减速                                    │
│   - 轻微摆动效果（wobble）                                │
└───────────────────────────────────────────────────────────┘
```

---

## 11. 数据流图

### 11.1 实体创建流程

```
┌──────────────────────────────────────────────────────────────────┐
│                       实体创建流程                                │
├──────────────────────────────────────────────────────────────────┤
│                                                                  │
│  JSON配置文件                                                    │
│      │                                                           │
│      ▼                                                           │
│  MonsterFactory::loadSingleConfig()                              │
│      │                                                           │
│      ▼                                                           │
│  parseMonsterConfig() ──> MonsterConfig结构                      │
│      │                                                           │
│      ▼                                                           │
│  _configs[monsterId] = config  (存入哈希表)                      │
│                                                                  │
│  ─────────────────────────────────────────────────────────────   │
│                                                                  │
│  MonsterFactory::createMonster(registry, "GreenSlime", x, y)     │
│      │                                                           │
│      ▼                                                           │
│  registry.create() ──> entt::entity                              │
│      │                                                           │
│      ├──> 根据cfg.type选择精灵组件                               │
│      │        SlimeSpriteComponent / MonsterSpriteComponent      │
│      │                                                           │
│      ├──> 根据cfg.movement.type选择移动组件                      │
│      │        JumpMovementComponent / WalkMovementComponent      │
│      │        DemonEyeMovementComponent                          │
│      │                                                           │
│      ├──> 添加通用组件                                           │
│      │        TransformComponent, HealthComponent, AggroComponent│
│      │        GroundDetectorComponent                            │
│      │                                                           │
│      ├──> 创建Cocos2d精灵和物理体                                │
│      │        Sprite::create(), PhysicsBody::createBox()         │
│      │                                                           │
│      └──> NodeEntityMap::registerNode(sprite, entityId)          │
│                                                                  │
│      ▼                                                           │
│  返回EntityId                                                    │
└──────────────────────────────────────────────────────────────────┘
```

### 11.2 系统更新流程

```
┌──────────────────────────────────────────────────────────────────┐
│                      系统更新数据流                               │
├──────────────────────────────────────────────────────────────────┤
│                                                                  │
│  SystemManagerEntt::update(delta)                                │
│      │                                                           │
│      ▼ (按优先级顺序)                                            │
│                                                                  │
│  ┌────────────────────────────────────────────────────────────┐ │
│  │ AggroSystem (AI-10)                                        │ │
│  │   读取: TransformComponent, PlayerTag                      │ │
│  │   写入: AggroComponent (hasAggro, distance, direction)     │ │
│  └────────────────────────────────────────────────────────────┘ │
│      │                                                           │
│      ▼                                                           │
│  ┌────────────────────────────────────────────────────────────┐ │
│  │ GroundDetectorSystem (PHYSICS+10)                          │ │
│  │   读取: SlimeSpriteComponent.velocity                      │ │
│  │   写入: GroundDetectorComponent (isStill)                  │ │
│  │         PhysicsBody (linearDamping)                        │ │
│  └────────────────────────────────────────────────────────────┘ │
│      │                                                           │
│      ▼                                                           │
│  ┌────────────────────────────────────────────────────────────┐ │
│  │ JumpMovementSystem (MOVEMENT-10)                           │ │
│  │   读取: JumpMovementComponent, GroundDetectorComponent,    │ │
│  │         AggroComponent, TransformComponent                 │ │
│  │   写入: PhysicsBody (applyImpulse)                         │ │
│  │         SlimeSpriteComponent (facing, jumpAnimation)       │ │
│  │         GroundDetectorComponent (isOnGround=false)         │ │
│  └────────────────────────────────────────────────────────────┘ │
│      │                                                           │
│      ▼                                                           │
│  ┌────────────────────────────────────────────────────────────┐ │
│  │ SlimeRenderSystem (RENDER)                                 │ │
│  │   读取: PhysicsBody.position                               │ │
│  │   写入: TransformComponent.position                        │ │
│  │         Sprite (rotation, visible, color, spriteFrame)     │ │
│  └────────────────────────────────────────────────────────────┘ │
│                                                                  │
└──────────────────────────────────────────────────────────────────┘
```

---

## 12. 扩展指南

### 12.1 添加新怪物类型

1. **创建JSON配置文件**
   ```json
   // Resources/config/newtype/NewMonster.json
   {
     "id": "NewMonster",
     "type": "NewType",
     "display": { ... },
     "physics": { ... },
     "movement": { "type": "newmovement", ... },
     ...
   }
   ```

2. **（可选）创建新移动组件**
   ```cpp
   // Classes/ecs/Components.h
   struct NewMovementComponent {
     float speed = 100.0f;
     // ... 新属性
   };
   ```

3. **（可选）创建新System**
   ```cpp
   // Classes/ecs/SystemsEntt.h
   class NewMovementSystemEntt : public ISystemEntt {
   public:
     const char* getName() const override { return "NewMovementSystem"; }
     int getPriority() const override { return SystemPriority::MOVEMENT; }
     
     void update(float delta) override {
       auto view = _registry->view<NewMovementComponent, ...>();
       view.each([delta](auto entity, NewMovementComponent& move, ...) {
         // 实现移动逻辑
       });
     }
   };
   ```

4. **更新MonsterFactory**
   ```cpp
   // MonsterFactory.cpp
   if (cfg.type == "NewType") {
     auto& sprite = registry.emplace<ecs::MonsterSpriteComponent>(entity);
     // 初始化...
   }
   
   if (cfg.movement.type == "newmovement") {
     auto& move = registry.emplace<ecs::NewMovementComponent>(entity);
     // 初始化...
   }
   ```

5. **在场景中注册System**
   ```cpp
   _systemManager.addSystem<ecs::NewMovementSystemEntt>();
   ```

### 12.2 添加新组件

1. 在 `Components.h` 中定义结构体
2. 遵循命名规范：`XxxComponent`
3. 提供默认值和默认构造函数
4. 如果管理Cocos2d资源，需要：
   - 禁止拷贝
   - 实现移动构造/赋值
   - 实现析构函数释放资源

### 12.3 添加新System

1. 继承 `ISystemEntt`
2. 实现 `getName()`、`getPriority()`、`update()`
3. 使用 `_registry->view<...>()` 查询组件
4. 在场景中通过 `_systemManager.addSystem<>()` 注册

---

## 附录：常用代码模式

### A.1 创建实体

```cpp
auto entity = _registry.create();
_registry.emplace<TransformComponent>(entity, x, y);
_registry.emplace<HealthComponent>(entity, 100.0f);
```

### A.2 查询组件

```cpp
// 查询单个实体的组件
auto* health = _registry.try_get<HealthComponent>(entity);
if (health) {
    health->takeDamage(10);
}

// 批量查询
auto view = _registry.view<TransformComponent, HealthComponent>();
view.each([](auto entity, TransformComponent& t, HealthComponent& h) {
    // 处理每个实体
});
```

### A.3 销毁实体

```cpp
_registry.destroy(entity);
```

### A.4 物理体创建

```cpp
PhysicsMaterial material(1.0f, 0.0f, 1.0f);  // density, restitution, friction
auto body = PhysicsBody::createBox(Size(40, 25), material);
body->setDynamic(true);
body->setRotationEnable(false);
body->setCategoryBitmask(0x0002);
body->setContactTestBitmask(0xFFFFFFFF);
body->setCollisionBitmask(0xFFFFFFFB);
body->setGroup(-1);
sprite->setPhysicsBody(body);
```

---

*文档生成日期: 2025-12-11*
*项目版本: Tongjieria ECS*
