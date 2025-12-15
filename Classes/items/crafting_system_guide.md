# 泰拉瑞亚风格 - 合成系统技术纲要

## 📚 目录
1. [系统概述](#系统概述)
2. [架构设计](#架构设计)
3. [数据结构定义](#数据结构定义)
4. [核心模块实现](#核心模块实现)
5. [实现步骤](#实现步骤)
6. [使用示例](#使用示例)
7. [后续扩展](#后续扩展)

---

## 系统概述

### 设计理念
- **数据驱动**: 所有配方由 JSON 定义，代码只负责逻辑
- **环境感知**: 基于玩家周围环境（工作台/熔炉等）动态显示可合成配方
- **事件响应**: 背包变化、环境变化时自动刷新合成列表
- **模块化**: 各模块职责单一，便于测试和扩展

### 核心特性
✅ 支持徒手合成和环境合成（工作台、熔炉、铁砧等）
✅ 自动检测材料是否足够
✅ 背包满时自动生成掉落物
✅ 二次校验防止异步问题
✅ 完整的事件通知机制

---

## 架构设计

### 系统组件关系图

```
┌─────────────────────────────────────────────────────────┐
│                   Crafting System                       │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  ┌──────────────┐         ┌──────────────┐            │
│  │   recipes    │────────▶│   Recipe     │            │
│  │   .json      │  加载   │   Manager    │            │
│  └──────────────┘         └──────────────┘            │
│                                  │                     │
│                                  │ 提供配方            │
│  ┌──────────────┐         ┌──────────────┐            │
│  │   Player     │────────▶│   Station    │            │
│  │   Position   │  位置   │   Detector   │            │
│  └──────────────┘         └──────────────┘            │
│                                  │                     │
│                                  │ 环境列表            │
│                                  ▼                     │
│  ┌──────────────┐         ┌──────────────┐            │
│  │  Inventory   │────────▶│   Crafting   │────────┐   │
│  │   (背包)     │  材料   │   Matcher    │  可合  │   │
│  └──────────────┘         └──────────────┘  成列表│   │
│         ▲                        │              │   │
│         │                        ▼              ▼   │
│         │                 ┌──────────────┐  ┌──────────────┐
│         │                 │   Crafting   │  │   Crafting   │
│         │扣材料/加产物     │   Executor   │  │    Layer     │
│         └─────────────────│   (执行器)   │  │   (UI界面)   │
│                           └──────────────┘  └──────────────┘
│                                  │                     │
│                                  └─────────────────────┘
│                                      用户点击合成        │
└─────────────────────────────────────────────────────────┘
```

### 数据流向

```
玩家移动 ──▶ StationDetector.updateStations()
              │
              ├──▶ 派发 Event_StationChanged
              │
背包变化 ──▶ Inventory.dispatchInventoryChanged()
              │
              ├──▶ 派发 Event_InventoryChanged
              │
              ▼
       CraftingLayer.refresh()
              │
              ├──▶ CraftingMatcher.getAvailableRecipes()
              │         │
              │         ├─▶ RecipeManager (获取所有配方)
              │         ├─▶ StationDetector (环境筛选)
              │         └─▶ Inventory (材料检查)
              │
              └──▶ 更新UI显示

用户点击合成 ──▶ CraftingExecutor.craft()
                    │
                    ├─▶ 二次校验材料
                    ├─▶ Inventory.removeItem() (扣材料)
                    ├─▶ Inventory.addItemWithOverflow() (加产物)
                    └─▶ 处理溢出掉落
```

---

## 数据结构定义

### CraftingDef.h
```cpp
// 合成环境类型
enum class StationType {
    Hand = 0,      // 徒手
    Workbench,     // 工作台 (tag=1)
    Furnace,       // 熔炉 (tag=2)
    Anvil,         // 铁砧 (tag=3)
};

// 配方材料
struct RecipeIngredient {
    int itemId;    // 材料ID
    int count;     // 数量
};

// 配方定义
struct RecipeDefinition {
    int recipeId;
    int resultItemId;
    int resultCount;
    std::vector<StationType> allowedStations;  // 允许的工作站列表（支持多个）
    std::vector<RecipeIngredient> ingredients;
};
```

---

## 核心模块实现

### 模块列表
1. **RecipeManager** - 加载recipes.json，提供配方查询
2. **StationDetector** - 检测玩家周围环境（基于tag）
3. **CraftingMatcher** - 筛选可合成配方（环境+材料）
4. **CraftingExecutor** - 执行合成（扣材料、加产物）
5. **CraftingLayer** - UI显示和交互

### 关键实现
```cpp
// 1. 加载配方
RecipeManager::getInstance()->loadRecipes("items/items_json/recipes.json");

// 2. 检测环境
StationDetector::getInstance()->addStation(StationType::Workbench);

// 3. 获取可合成列表
auto recipes = CraftingMatcher::getInstance()->getAvailableRecipes();

// 4. 执行合成
CraftingExecutor::getInstance()->craft(*recipe);
```

---

## 实现步骤

1. 创建 `CraftingDef.h`（数据结构）
2. 实现 `RecipeManager`（配方管理）
3. 实现 `StationDetector`（环境检测）
4. 实现 `CraftingMatcher` + `CraftingExecutor`
5. 实现 `CraftingLayer`（UI）
6. 测试完整流程

---

## 使用示例

```cpp
// 场景初始化
bool TestScene::init() {
    // 添加测试物品
    Inventory::getInstance()->addItem(1, 50);  // 木头

    // 模拟工作台环境
    StationDetector::getInstance()->addStation(StationType::Workbench);

    // 添加合成UI
    auto craftingUI = CraftingLayer::create();
    this->addChild(craftingUI);

    return true;
}
```

---

## CMake集成

```cmake
list(APPEND GAME_SOURCE
    Classes/items/CraftingDef.h
    Classes/items/RecipeManager.h
    Classes/items/RecipeManager.cpp
    Classes/items/StationDetector.h
    Classes/items/StationDetector.cpp
    Classes/items/CraftingMatcher.h
    Classes/items/CraftingMatcher.cpp
    Classes/items/CraftingExecutor.h
    Classes/items/CraftingExecutor.cpp
    Classes/items/CraftingLayer.h
    Classes/items/CraftingLayer.cpp
)
```

---

## 参考

### recipes.json 格式说明

**单个工作站**:
```json
{
  "Result": { "ItemID": 4, "Count": 1 },
  "Station": "Workbench",
  "Ingredients": [{"Type": "Item", "ID": 1, "Count": 10}]
}
```

**多个工作站**（同一配方可在多个环境合成）:
```json
{
  "Result": { "ItemID": 10, "Count": 1 },
  "Stations": ["Workbench", "Anvil"],
  "Ingredients": [{"Type": "Item", "ID": 1, "Count": 5}]
}
```

### 其他参考
- recipes.json 已存在于 `Resources/items/items_json/`
- 环境tag: 工作台=1, 熔炉=2, 铁砧=3
- 事件: `Event_InventoryChanged`, `Event_StationChanged`
- 支持单个或多个工作站的配方定义

**状态**: ✅ 设计完成，待实现
