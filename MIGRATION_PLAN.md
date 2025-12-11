# EnTT迁移计划

## 背景
团队决定统一使用EnTT库，需要将当前自研ECS迁移到EnTT。

## 迁移策略：渐进式 + 兼容层

### 优势
- ✅ 边迁移边测试，降低风险
- ✅ 保持项目可运行状态
- ✅ 可随时回滚
- ✅ 学习EnTT的同时完成迁移

## 阶段划分

### 阶段0：准备（30分钟）
- [ ] 安装EnTT库
- [ ] 创建Git分支：`feature/migrate-to-entt`
- [ ] 备份当前代码

### 阶段1：引入EnTT + 适配层（2小时）
- [ ] 添加EnTT头文件
- [ ] 创建`ecs/EnttAdapter.h`适配层
- [ ] 测试编译通过

### 阶段2：迁移核心组件（3小时）
- [ ] 迁移Component定义（无需修改，EnTT兼容普通struct）
- [ ] 迁移World → Registry
- [ ] 迁移Entity创建/销毁
- [ ] 测试基础功能

### 阶段3：迁移System（4小时）
- [ ] 迁移AggroSystem
- [ ] 迁移JumpMovementSystem
- [ ] 迁移WalkMovementSystem
- [ ] 迁移其他Systems
- [ ] 测试游戏逻辑

### 阶段4：迁移场景（3小时）
- [ ] 迁移SlimeTestScene
- [ ] 迁移ZombieTestScene
- [ ] 迁移MonsterFactory
- [ ] 全面测试

### 阶段5：清理（1小时）
- [ ] 删除自研ECS文件（保留备份）
- [ ] 更新文档
- [ ] 代码审查
- [ ] 合并到主分支

## 总预计时间：1-2个工作日

## API对照表

### 创建实体
```cpp
// 旧代码（自研）
auto entity = _world.createEntity();

// 新代码（EnTT）
auto entity = _registry.create();
```

### 添加组件
```cpp
// 旧代码
_world.addComponent<HealthComponent>(entity, 100);

// 新代码
_registry.emplace<HealthComponent>(entity, 100);
```

### 获取组件
```cpp
// 旧代码
auto* health = _world.getComponent<HealthComponent>(entity);

// 新代码
auto* health = _registry.try_get<HealthComponent>(entity);
```

### 迭代组件
```cpp
// 旧代码
_world.forEach<HealthComponent, TransformComponent>(
    [](EntityId e, HealthComponent& h, TransformComponent& t) {
        // ...
    });

// 新代码
auto view = _registry.view<HealthComponent, TransformComponent>();
for(auto entity : view) {
    auto& [health, trans] = view.get<HealthComponent, TransformComponent>(entity);
    // ...
}
```

### 删除组件
```cpp
// 旧代码
_world.removeComponent<HealthComponent>(entity);

// 新代码
_registry.remove<HealthComponent>(entity);
```

### 销毁实体
```cpp
// 旧代码
_world.destroyEntity(entity);

// 新代码
_registry.destroy(entity);
```

## 兼容性注意事项

1. **EntityId类型**
   - 旧：`ecs::EntityId` (uint32_t)
   - 新：`entt::entity` (uint32_t) ✅ 兼容

2. **组件定义**
   - EnTT不需要继承`IComponent` ✅ 直接可用

3. **System基类**
   - 需要适配`ISystem`接口 ⚠️ 需要修改

4. **NodeEntityMap**
   - 保留这个工具类 ✅ 继续使用

## 风险控制

1. **分支开发**：在独立分支进行，不影响主分支
2. **增量测试**：每个阶段都编译测试
3. **保留备份**：迁移完成前不删除旧代码
4. **性能对比**：迁移前后对比帧率

## 回滚方案

如果遇到无法解决的问题：
1. Git revert到迁移前的commit
2. 保留EnTT分支供后续研究
3. 与团队沟通，寻求帮助
