# EnTT库安装指南

## 方法1：使用vcpkg（推荐）

### 步骤
```powershell
# 1. 安装vcpkg（如果还没有）
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# 2. 安装EnTT
.\vcpkg install entt:x64-windows

# 3. 集成到Visual Studio
.\vcpkg integrate install
```

### CMakeLists.txt配置
```cmake
find_package(EnTT CONFIG REQUIRED)
target_link_libraries(${APP_NAME} EnTT::EnTT)
```

## 方法2：手动下载（快速开始）

### 步骤

1. **下载EnTT**
   - 访问：https://github.com/skypjack/entt/releases
   - 下载最新版本（v3.13.0或更高）
   - 解压到项目目录

2. **放置头文件**
   ```
   Tongjieria/
   └── external/
       └── entt/
           └── entt.hpp
           └── (其他头文件)
   ```

3. **修改项目包含路径**
   
   **Visual Studio方式：**
   - 右键项目 → 属性
   - C/C++ → 常规 → 附加包含目录
   - 添加：`$(ProjectDir)..\external`
   
   **CMakeLists.txt方式：**
   ```cmake
   include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external)
   ```

4. **测试安装**
   ```cpp
   #include <entt/entt.hpp>
   
   void test() {
       entt::registry registry;
       auto entity = registry.create();
       // 如果编译通过，说明安装成功
   }
   ```

## 方法3：作为子模块（团队协作推荐）

```bash
# 在项目根目录
git submodule add https://github.com/skypjack/entt.git external/entt
git submodule update --init --recursive
```

### CMakeLists.txt配置
```cmake
add_subdirectory(external/entt)
target_link_libraries(${APP_NAME} EnTT::EnTT)
```

## 验证安装

创建测试文件 `test_entt.cpp`：

```cpp
#include <entt/entt.hpp>
#include <iostream>

struct Position {
    float x, y;
};

struct Velocity {
    float dx, dy;
};

int main() {
    entt::registry registry;
    
    // 创建实体
    auto entity = registry.create();
    
    // 添加组件
    registry.emplace<Position>(entity, 0.f, 0.f);
    registry.emplace<Velocity>(entity, 1.f, 1.f);
    
    // 迭代组件
    auto view = registry.view<Position, Velocity>();
    for(auto e : view) {
        auto& pos = view.get<Position>(e);
        auto& vel = view.get<Velocity>(e);
        pos.x += vel.dx;
        pos.y += vel.dy;
        std::cout << "Entity moved to (" << pos.x << ", " << pos.y << ")\n";
    }
    
    std::cout << "EnTT is working!\n";
    return 0;
}
```

编译运行，如果输出 `EnTT is working!` 则安装成功。

## 常见问题

### Q: 编译错误 "entt/entt.hpp: No such file"
A: 检查包含路径是否正确配置。

### Q: 链接错误
A: EnTT是header-only库，不需要链接，只需要包含头文件。

### Q: C++版本要求
A: EnTT需要C++17或更高版本。在CMakeLists.txt中设置：
```cmake
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

## 下一步

安装完成后，参考 `MIGRATION_PLAN.md` 开始迁移。
