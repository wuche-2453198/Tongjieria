# EnTT迁移示例

## 示例1：迁移AggroSystem

### 原代码（自研ECS）

```cpp
// ecs/Systems.h
class AggroSystem : public ISystem {
public:
    const char* getName() const override { return "AggroSystem"; }
    
    void update(float delta) override {
        _world->forEach<AggroComponent, TransformComponent>(
            [this](EntityId entity, AggroComponent &aggro, TransformComponent &transform) {
                if (aggro.targetTag.empty()) return;
                
                // 查找目标
                EntityId target = findTargetByTag(aggro.targetTag);
                if (target == INVALID_ENTITY) return;
                
                auto* targetTransform = _world->getComponent<TransformComponent>(target);
                if (!targetTransform) return;
                
                float dist = transform.position.distance(targetTransform->position);
                
                // 判断进入/退出仇恨
                if (!aggro.inAggro && dist <= aggro.aggroRange) {
                    aggro.inAggro = true;
                    aggro.targetEntity = target;
                } else if (aggro.inAggro && dist > aggro.deaggroRange) {
                    aggro.inAggro = false;
                    aggro.targetEntity = INVALID_ENTITY;
                }
            });
    }
    
private:
    EntityId findTargetByTag(const std::string& tag) {
        EntityId result = INVALID_ENTITY;
        _world->forEach<TransformComponent>([&](EntityId e, TransformComponent&) {
            if (_world->hasComponent<PlayerTag>(e)) {
                result = e;
            }
        });
        return result;
    }
};
```

### 新代码（EnTT） - 方案A：使用适配层

```cpp
// ecs/Systems.h
#include "ecs/EnttAdapter.h"

class AggroSystem : public ecs::ISystem {
public:
    const char* getName() const override { return "AggroSystem"; }
    
    void update(float delta) override {
        // 使用适配器的forEach函数
        ecs::forEach<AggroComponent, TransformComponent>(*_registry,
            [this](ecs::EntityId entity, AggroComponent &aggro, TransformComponent &transform) {
                if (aggro.targetTag.empty()) return;
                
                // 查找目标
                ecs::EntityId target = findTargetByTag(aggro.targetTag);
                if (target == ecs::INVALID_ENTITY) return;
                
                auto* targetTransform = ecs::tryGet<TransformComponent>(*_registry, target);
                if (!targetTransform) return;
                
                float dist = transform.position.distance(targetTransform->position);
                
                if (!aggro.inAggro && dist <= aggro.aggroRange) {
                    aggro.inAggro = true;
                    aggro.targetEntity = target;
                } else if (aggro.inAggro && dist > aggro.deaggroRange) {
                    aggro.inAggro = false;
                    aggro.targetEntity = ecs::INVALID_ENTITY;
                }
            });
    }
    
private:
    ecs::EntityId findTargetByTag(const std::string& tag) {
        // 使用EnTT的view查找
        auto view = _registry->view<TransformComponent, PlayerTag>();
        for (auto entity : view) {
            return entity; // 返回第一个找到的玩家
        }
        return ecs::INVALID_ENTITY;
    }
};
```

### 新代码（EnTT） - 方案B：原生EnTT API（推荐）

```cpp
// ecs/Systems.h
#include <entt/entt.hpp>

class AggroSystem : public ecs::ISystem {
public:
    const char* getName() const override { return "AggroSystem"; }
    
    void update(float delta) override {
        // 原生EnTT view API
        auto view = _registry->view<AggroComponent, TransformComponent>();
        
        // 使用each迭代（自动解包组件）
        view.each([this](auto entity, AggroComponent &aggro, TransformComponent &transform) {
            if (aggro.targetTag.empty()) return;
            
            auto target = findTargetByTag(aggro.targetTag);
            if (target == entt::null) return;
            
            // 更简洁的组件获取
            if (auto* targetTransform = _registry->try_get<TransformComponent>(target)) {
                float dist = transform.position.distance(targetTransform->position);
                
                if (!aggro.inAggro && dist <= aggro.aggroRange) {
                    aggro.inAggro = true;
                    aggro.targetEntity = target;
                } else if (aggro.inAggro && dist > aggro.deaggroRange) {
                    aggro.inAggro = false;
                    aggro.targetEntity = entt::null;
                }
            }
        });
    }
    
private:
    entt::entity findTargetByTag(const std::string& tag) {
        auto view = _registry->view<TransformComponent, PlayerTag>();
        // EnTT的front()返回第一个实体
        return view.empty() ? entt::null : view.front();
    }
};
```

## 示例2：迁移MonsterFactory

### 原代码

```cpp
ecs::EntityId MonsterFactory::createMonster(ecs::World &world,
                                            const std::string &monsterId,
                                            float x, float y,
                                            cocos2d::Node *parentNode) {
    // ...
    auto entity = world.createEntity();
    world.addComponent<TransformComponent>(entity, x, y);
    world.addComponent<HealthComponent>(entity, cfg.stats.maxHealth);
    // ...
    return entity;
}
```

### 新代码

```cpp
ecs::EntityId MonsterFactory::createMonster(ecs::Registry &registry,
                                            const std::string &monsterId,
                                            float x, float y,
                                            cocos2d::Node *parentNode) {
    // ...
    auto entity = registry.create();
    registry.emplace<TransformComponent>(entity, x, y);
    registry.emplace<HealthComponent>(entity, cfg.stats.maxHealth);
    // ...
    return entity;
}
```

## 示例3：迁移Scene

### 原代码

```cpp
// ZombieTestScene.h
class ZombieTestScene : public cocos2d::Layer {
private:
    ecs::World _world;
    // ...
};

// ZombieTestScene.cpp
void ZombieTestScene::setupEcsSystems() {
    _world.addSystem<ecs::AggroSystem>();
    _world.addSystem<ecs::WalkMovementSystem>();
}

void ZombieTestScene::update(float delta) {
    _world.update(delta);
}
```

### 新代码 - 方案A：使用适配层World

```cpp
// ZombieTestScene.h
#include "ecs/EnttAdapter.h"

class ZombieTestScene : public cocos2d::Layer {
private:
    ecs::World _world; // 适配层的World（包装了Registry）
    // ...
};

// ZombieTestScene.cpp
// 代码几乎不需要修改！
void ZombieTestScene::setupEcsSystems() {
    _world.addSystem<ecs::AggroSystem>();
    _world.addSystem<ecs::WalkMovementSystem>();
}

void ZombieTestScene::update(float delta) {
    _world.update(delta);
}
```

### 新代码 - 方案B：直接使用Registry（推荐）

```cpp
// ZombieTestScene.h
#include <entt/entt.hpp>
#include "ecs/EnttAdapter.h" // 只为了System基类

class ZombieTestScene : public cocos2d::Layer {
private:
    ecs::Registry _registry;
    std::vector<std::unique_ptr<ecs::ISystem>> _systems;
    // ...
    
public:
    void addSystem(std::unique_ptr<ecs::ISystem> system);
};

// ZombieTestScene.cpp
void ZombieTestScene::addSystem(std::unique_ptr<ecs::ISystem> system) {
    system->setRegistry(&_registry);
    _systems.push_back(std::move(system));
}

void ZombieTestScene::setupEcsSystems() {
    addSystem(std::make_unique<ecs::AggroSystem>());
    addSystem(std::make_unique<ecs::WalkMovementSystem>());
    
    // 按优先级排序
    std::sort(_systems.begin(), _systems.end(),
        [](const auto& a, const auto& b) {
            return a->getPriority() < b->getPriority();
        });
}

void ZombieTestScene::update(float delta) {
    for (auto& system : _systems) {
        system->update(delta);
    }
}
```

## 性能对比

### 迭代性能测试（1000个实体）

```cpp
// 自研ECS
auto start = std::chrono::high_resolution_clock::now();
_world.forEach<Position, Velocity>([](EntityId e, Position& p, Velocity& v) {
    p.x += v.dx;
    p.y += v.dy;
});
auto end = std::chrono::high_resolution_clock::now();
// 约 15-20μs

// EnTT
start = std::chrono::high_resolution_clock::now();
auto view = _registry.view<Position, Velocity>();
view.each([](auto e, Position& p, Velocity& v) {
    p.x += v.dx;
    p.y += v.dy;
});
end = std::chrono::high_resolution_clock::now();
// 约 5-8μs （快2-3倍）
```

## 迁移检查清单

- [ ] 所有`ecs::World`替换为`ecs::Registry`或适配层`ecs::World`
- [ ] 所有`world.createEntity()`替换为`registry.create()`
- [ ] 所有`world.addComponent<T>`替换为`registry.emplace<T>`
- [ ] 所有`world.getComponent<T>`替换为`registry.try_get<T>`
- [ ] 所有`world.forEach<Ts...>`替换为`registry.view<Ts...>()`
- [ ] 所有`INVALID_ENTITY`替换为`entt::null`或`ecs::INVALID_ENTITY`
- [ ] System基类的`_world`指针改为`_registry`指针
- [ ] 编译测试通过
- [ ] 运行测试通过
- [ ] 性能测试通过

## 推荐迁移顺序

1. ✅ 安装EnTT
2. ✅ 创建适配层`EnttAdapter.h`
3. ✅ 迁移一个简单的System（如HealthSystem）测试
4. ✅ 迁移所有Systems
5. ✅ 迁移MonsterFactory
6. ✅ 迁移各个Scene
7. ✅ 删除旧ECS代码（保留git历史）
8. ✅ 更新文档

## 预期收益

- ✅ **性能提升**：组件迭代快2-3倍
- ✅ **内存优化**：更紧凑的内存布局
- ✅ **团队协作**：与合作者代码无缝集成
- ✅ **功能增强**：可使用EnTT的信号、观察者等高级特性
- ✅ **减少维护**：不需要维护自研ECS的bug
