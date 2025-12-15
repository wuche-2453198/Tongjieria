# EnTT 库集成说明

## ✅ 已完成的配置

### 1. CMakeLists.txt 配置
已在 CMakeLists.txt 中添加 EnTT 的包含路径：

```cmake
target_include_directories(${APP_NAME}
    PRIVATE Classes
    PRIVATE ${COCOS2DX_ROOT_PATH}/cocos/audio/include/
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/entt-main/entt-main/single_include
)
```

**位置**: 第 156-160 行

### 2. VSCode IntelliSense 配置
已在 `.vscode/c_cpp_properties.json` 中添加 EnTT 路径：

```json
"includePath": [
    ...
    "${workspaceFolder}/third_party/entt-main/entt-main/single_include"
]
```

这样 VSCode 的智能提示和错误检查就能正常工作了。

### 3. 测试文件
创建了 `Classes/items/EnttTest.h` 用于验证 EnTT 是否正常工作。

### 4. 集成测试
已在 `ItemsTestScene.cpp` 中添加了 EnTT 测试代码。

## 🚀 如何验证 EnTT 是否正常工作

### 方法 1: 运行游戏并查看日志
1. 编译并运行游戏
2. 进入 Items Test Scene
3. 查看控制台输出，应该看到：

```
========================================
Running EnTT Integration Tests...
========================================

=== EnTT Basic Test Start ===
✓ Test 1 Passed: Created 3 entities
✓ Test 2 Passed: Added components to entities
✓ Test 3 Passed: Component query works (x=100.0, y=200.0)
✓ Test 4 Passed: View query found 3 entities with Position
✓ Test 5 Passed: Multi-component view found 2 entities
  - Entity with Position(100.0, 200.0) and Velocity(5.0, -3.0)
  - Entity with Position(500.0, 600.0) and Velocity(-2.0, 4.0)
Test 6: Using each() to iterate:
  - Player at position (100.0, 200.0)
  - Enemy at position (300.0, 400.0)
✓ Test 6 Passed: each() iteration works
✓ Test 7 Passed: Component removal works
✓ Test 8 Passed: Entity destruction works
✓ Test 9 Passed: 2 entities alive
=== EnTT Basic Test Complete: ALL TESTS PASSED ===

✓ EnTT is properly configured and working!

========================================
```

### 方法 2: 在代码中使用 EnTT
在任何 cpp 文件中添加：

```cpp
#include "entt/entt.hpp"

void testEnTT() {
    entt::registry registry;

    // 创建实体
    auto entity = registry.create();

    // 添加组件
    struct Position { float x, y; };
    registry.emplace<Position>(entity, 10.0f, 20.0f);

    // 获取组件
    auto& pos = registry.get<Position>(entity);
    CCLOG("Position: (%.1f, %.1f)", pos.x, pos.y);
}
```

## 📝 EnTT 基础使用

### 1. 创建 Registry
```cpp
entt::registry registry;
```

### 2. 创建实体
```cpp
auto entity = registry.create();
```

### 3. 添加组件
```cpp
struct Position { float x, y; };
struct Velocity { float dx, dy; };

registry.emplace<Position>(entity, 100.0f, 200.0f);
registry.emplace<Velocity>(entity, 1.0f, -1.0f);
```

### 4. 获取组件
```cpp
// 安全获取（可能为 nullptr）
auto* pos = registry.try_get<Position>(entity);
if (pos) {
    // 使用 pos
}

// 直接获取（不存在会崩溃）
auto& pos = registry.get<Position>(entity);
```

### 5. 检查组件
```cpp
if (registry.all_of<Position>(entity)) {
    // 实体有 Position 组件
}

if (registry.any_of<Position, Velocity>(entity)) {
    // 实体有 Position 或 Velocity 组件
}
```

### 6. 移除组件
```cpp
registry.remove<Position>(entity);
```

### 7. 销毁实体
```cpp
registry.destroy(entity);
```

### 8. 查询实体（View）
```cpp
// 单组件查询
auto view = registry.view<Position>();
for (auto entity : view) {
    auto& pos = view.get<Position>(entity);
    CCLOG("Entity at (%.1f, %.1f)", pos.x, pos.y);
}

// 多组件查询
auto multiView = registry.view<Position, Velocity>();
for (auto entity : multiView) {
    auto [pos, vel] = multiView.get<Position, Velocity>(entity);
    pos.x += vel.dx;
    pos.y += vel.dy;
}

// 使用 each 遍历
registry.view<Position, Velocity>().each([](auto entity, Position& pos, Velocity& vel) {
    pos.x += vel.dx;
    pos.y += vel.dy;
});
```

## 🎯 下一步：将物品系统改造为 ECS

### 建议的改造步骤：

#### 1. 定义物品组件（已准备好示例）
- `ItemComponent` - 物品基础信息
- `StackableComponent` - 可堆叠属性
- `EquipmentComponent` - 装备属性
- `ConsumableComponent` - 消耗品属性
- `ItemOwnerComponent` - 物品所属信息
- `DroppedItemComponent` - 掉落物信息

#### 2. 创建物品系统
- `ItemSystem` - 管理物品的创建、添加、移除
- `InventorySystem` - 管理背包逻辑
- `DropSystem` - 管理掉落物

#### 3. 迁移现有代码
- 保留 `ItemManager` 用于加载 JSON 定义
- 将 `Inventory` 改为基于 EnTT 的实现
- `InventoryLayer` 从 EnTT registry 读取数据显示

## ⚙️ 编译说明

### 重新生成 CMake 项目（如果需要）
```bash
cd build
cmake ..
```

### 在 Visual Studio 中
1. 打开 `proj.win32/Mygame.sln`
2. 直接编译运行（CMake 配置会自动生效）

### 如果遇到编译错误
1. 检查 `third_party/entt-main/entt-main/single_include/entt/entt.hpp` 是否存在
2. 确保 CMakeLists.txt 中的路径正确
3. 清理并重新生成项目

## 📚 EnTT 参考资源

- [EnTT 官方文档](https://github.com/skypjack/entt/wiki)
- [EnTT GitHub](https://github.com/skypjack/entt)
- [ECS 设计模式介绍](https://github.com/skypjack/entt/wiki/Crash-Course:-entity-component-system)

## 🔍 故障排除

### 问题: 无法找到 entt/entt.hpp
**解决方案**:
- 检查 CMakeLists.txt 是否正确添加了 include 路径
- 重新运行 CMake 生成项目

### 问题: VSCode 报错但编译成功
**解决方案**:
- 重新加载 VSCode 窗口（Ctrl+Shift+P -> Reload Window）
- 检查 `.vscode/c_cpp_properties.json` 是否包含 EnTT 路径

### 问题: 编译很慢
**解决方案**:
- 使用单头文件版本（已配置）
- 考虑使用预编译头 (PCH)

## ✨ 总结

EnTT 已成功集成到你的项目中！

**已完成**:
- ✅ CMakeLists.txt 配置
- ✅ VSCode IntelliSense 配置
- ✅ 测试代码和验证

**下一步**:
- 🎯 将物品系统改造为基于 EnTT 的 ECS 架构
- 🎯 实现物品组件和系统
- 🎯 迁移现有的物品逻辑

**优势**:
- 🚀 高性能的组件查询
- 🧩 灵活的组件组合
- 📦 内存友好的存储方式
- 🔧 易于扩展和维护
