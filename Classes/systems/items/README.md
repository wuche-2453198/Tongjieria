# Items System - 物品系统模块

此目录包含游戏中所有物品相关的核心系统代码，包括物品管理、物品栏、装备系统和合成系统。

## 目录结构

```
items/
├── 核心数据定义
│   ├── InventoryDef.h          # 物品栏数据结构定义
│   └── CraftingDef.h           # 合成系统数据结构定义
│
├── 物品管理
│   ├── ItemManager.h/cpp       # 物品数据管理器（单例）
│   ├── Inventory.h/cpp         # 物品栏逻辑（单例）
│   └── ItemManagerTest.h       # 物品管理器测试
│
├── UI 层
│   ├── InventoryLayer.h/cpp    # 物品栏 UI（10x5 网格 + 特殊槽位）
│   ├── EquipmentPanel.h/cpp    # 装备面板 UI（7 个装备槽）
│   └── CraftBar.h/cpp          # 合成栏 UI（垂直列表 + 快速合成栏）
│
├── 合成系统
│   ├── RecipeManager.h/cpp     # 配方管理器（单例）
│   ├── StationDetector.h/cpp   # 工作台检测器（单例）
│   ├── CraftingMatcher.h/cpp   # 合成匹配器（单例）
│   └── CraftingExecutor.h/cpp  # 合成执行器（单例）
│
└── 
```

## 模块详细说明

---

### 1. 核心数据定义

#### `InventoryDef.h`
定义物品栏相关的数据结构。

**主要结构体：**
```cpp
struct ItemDefinition {
    int itemId;              // 物品 ID
    std::string name;        // 物品名称
    int type;                // 物品类型（1=装备, 2=武器, 3=材料, 4=消耗品, 5=可放置）
    int maxStack;            // 最大堆叠数量
    std::string iconPath;    // 图标路径
    // ... 更多属性
};

struct ItemSlot {
    int itemId;              // 物品 ID（0 表示空槽位）
    int count;               // 数量
};
```

#### `CraftingDef.h`
定义合成系统相关的数据结构。

**主要结构体：**
```cpp
enum class StationType {
    Hand,                    // 徒手合成
    Workbench,              // 工作台
    Furnace,                // 熔炉
    Anvil                   // 铁砧
};

struct IngredientRequirement {
    int itemId;             // 材料物品 ID
    int count;              // 需要数量
};

struct RecipeDefinition {
    int resultItemId;       // 产出物品 ID
    int resultCount;        // 产出数量
    std::vector<IngredientRequirement> ingredients;  // 材料需求
    StationType requiredStation;  // 需要的工作台类型
};

enum class CraftingStatus {
    CanCraft,               // 可以合成
    MissingMaterial,        // 缺少材料
    MissingStation,         // 缺少工作台
    InventoryFull           // 物品栏已满
};
```

---

### 2. 物品管理

#### `ItemManager` - 物品数据管理器
**职责：** 加载和管理所有物品的定义数据（从 JSON 文件加载）。

**单例模式：** `ItemManager::getInstance()`

**主要方法：**
```cpp
bool loadItemData(const std::string& jsonPath);  // 从 JSON 加载物品数据
const ItemDefinition* getItemData(int itemId);   // 获取物品定义
cocos2d::SpriteFrame* getItemSprite(int itemId); // 获取物品图标精灵
```

**数据文件：** `Resources/data/items.json`

**使用示例：**
```cpp
auto itemMgr = ItemManager::getInstance();
auto woodDef = itemMgr->getItemData(1001);  // 获取木头的定义
if (woodDef) {
    CCLOG("Item: %s, MaxStack: %d", woodDef->name.c_str(), woodDef->maxStack);
}
```

#### `Inventory` - 物品栏逻辑
**职责：** 管理玩家的物品栏数据（不含 UI）。

**单例模式：** `Inventory::getInstance()`

**物品栏布局：**
- 50 个普通物品槽（10x5 网格）
- 4 个货币槽
- 1 个垃圾桶槽
- 4 个弹药槽
- **总计：59 个槽位**

**主要方法：**
```cpp
void init(int capacity = 59);                    // 初始化物品栏
bool addItem(int itemId, int count);             // 添加物品
bool removeItem(int itemId, int count);          // 移除物品
bool swapSlots(int fromIndex, int toIndex);      // 交换槽位
int countItem(int itemId);                       // 统计物品数量
const std::vector<ItemSlot>& getSlots();         // 获取所有槽位
```

**事件通知：**
```cpp
// 物品栏改变时发送事件
"Event_InventoryChanged"
```

**使用示例：**
```cpp
auto inventory = Inventory::getInstance();
inventory->init();
inventory->addItem(1001, 100);  // 添加 100 个木头
bool hasEnough = (inventory->countItem(1001) >= 50);  // 检查是否有 50 个木头
```

---

### 3. UI 层

#### `InventoryLayer` - 物品栏 UI
**职责：** 显示物品栏的图形界面，处理鼠标交互（拖拽、点击）。

**继承：** `cocos2d::Layer`

**布局：**
- 主网格：10x5 普通物品槽
- 右侧：4 个货币槽 + 1 个垃圾桶槽
- 底部：4 个弹药槽
- 顶部：整理按钮

**位置：** 屏幕左上角（占屏幕 25% 宽度和 25% 高度）

**交互功能：**
- 鼠标悬停显示物品信息
- 左键拖拽移动物品
- 右键拆分堆叠物品
- 整理按钮自动排序物品

**槽位类型识别：**
```cpp
enum class SlotKind {
    Normal,   // 普通槽位
    Coin,     // 货币槽
    Ammo,     // 弹药槽
    Trash,    // 垃圾桶
    Weapon    // 武器槽
};
```

**主要方法：**
```cpp
void refresh();                                  // 刷新显示
void sortInventory();                            // 整理物品栏
```

#### `EquipmentPanel` - 装备面板 UI
**职责：** 显示角色装备槽，支持装备穿戴（待实现）。

**继承：** `cocos2d::Layer`

**装备槽位：**
- 头盔槽（Helmet）
- 胸甲槽（Chestplate）
- 护腿槽（Leggings）
- 4 个饰品槽（Accessory0-3）
- **总计：7 个装备槽**

**位置：** 屏幕右侧底部（与 CraftBar 对齐，y=70）

**槽位纹理：**
```cpp
"items/bottom/Helmet.png"       // 头盔槽背景
"items/bottom/Chestplate.png"   // 胸甲槽背景
"items/bottom/Leggings.png"     // 护腿槽背景
"items/bottom/Accessory.png"    // 饰品槽背景
```

**待实现功能：**
- 与玩家装备系统集成（PlayerInventoryIntegration）
- 装备拖拽穿戴/卸下
- 显示装备属性加成

#### `CraftBar` - 合成栏 UI
**职责：** 显示可合成的配方，提供两种合成界面。

**继承：** `cocos2d::Layer`

**两种模式：**
1. **垂直配方列表**（左侧固定）
   - 显示所有可用配方
   - 可滚动浏览
   - 点击展开显示材料需求

2. **快速合成栏**（屏幕中心横向）
   - 点击合成按钮展开
   - 显示可合成物品的图标
   - 最多显示 20 个配方

**位置：** 屏幕左下角（y=70，与装备面板对齐）

**交互逻辑（两次点击机制）：**
```cpp
// 不可合成：dark_bottom.png 背景，点击无效
// 可合成：
//   第一次点击：light_bottom.png 高亮选中
//   第二次点击：执行合成
```

**配方状态显示：**
```cpp
if (!canCraft) {
    // 使用 dark_bottom.png（暗色背景）
} else if (isSelected) {
    // 使用 light_bottom.png（高亮背景）
} else {
    // 使用 Inventory.png（普通背景）
}
```

**主要方法：**
```cpp
void refreshRecipes();                           // 刷新配方列表
void expandPanel();                              // 展开快速合成栏
void collapsePanel();                            // 收起快速合成栏
void onCategoryButtonClicked(int recipeIndex);   // 配方点击处理
void onQuickCraftButtonClicked(int recipeIndex); // 快速合成点击处理
```

---

### 4. 合成系统

合成系统采用**职责分离**设计，分为四个独立模块。

#### `RecipeManager` - 配方管理器
**职责：** 加载和管理所有合成配方数据。

**单例模式：** `RecipeManager::getInstance()`

**主要方法：**
```cpp
bool loadRecipes(const std::string& jsonPath);           // 从 JSON 加载配方
const std::vector<RecipeDefinition>& getAllRecipes();    // 获取所有配方
std::vector<const RecipeDefinition*> getRecipesByResultItem(int itemId);  // 根据产出物查询
std::vector<const RecipeDefinition*> getRecipesByStation(StationType station);  // 根据工作台查询
```

**数据文件：** `Resources/data/recipes.json`

#### `StationDetector` - 工作台检测器
**职责：** 检测玩家附近可用的工作台类型。

**单例模式：** `StationDetector::getInstance()`

**主要方法：**
```cpp
void addStation(StationType station);            // 添加可用工作台
void removeStation(StationType station);         // 移除工作台
bool hasStation(StationType station);            // 检查是否有指定工作台
const std::set<StationType>& getCurrentStations();  // 获取所有可用工作台
```

**事件通知：**
```cpp
"Event_StationChanged"  // 工作台状态改变时发送
```

**使用示例：**
```cpp
auto detector = StationDetector::getInstance();
detector->addStation(StationType::Workbench);    // 玩家靠近工作台
detector->addStation(StationType::Furnace);      // 玩家靠近熔炉
```

#### `CraftingMatcher` - 合成匹配器
**职责：** 检查配方是否可以合成（材料、工作台判断）。

**单例模式：** `CraftingMatcher::getInstance()`

**主要方法：**
```cpp
std::vector<const RecipeDefinition*> getAvailableRecipes();  // 获取可用配方（根据工作台筛选）
bool canCraft(const RecipeDefinition& recipe);               // 检查是否可合成
CraftingStatus getRecipeStatus(const RecipeDefinition& recipe);  // 获取配方状态
std::map<int, int> getMissingMaterials(const RecipeDefinition& recipe);  // 获取缺少的材料
```

**使用示例：**
```cpp
auto matcher = CraftingMatcher::getInstance();
auto recipes = matcher->getAvailableRecipes();  // 获取当前可用配方

for (auto recipe : recipes) {
    bool canCraft = matcher->canCraft(*recipe);
    if (!canCraft) {
        auto missing = matcher->getMissingMaterials(*recipe);
        // 显示缺少的材料
    }
}
```

#### `CraftingExecutor` - 合成执行器
**职责：** 执行合成操作（扣除材料、添加产出）。

**单例模式：** `CraftingExecutor::getInstance()`

**主要方法：**
```cpp
bool craft(const RecipeDefinition& recipe);      // 执行合成
```

**执行流程：**
1. 检查配方是否可合成（调用 CraftingMatcher）
2. 从物品栏扣除材料（调用 Inventory::removeItem）
3. 添加产出物品到物品栏（调用 Inventory::addItem）
4. 触发物品栏改变事件

**使用示例：**
```cpp
auto executor = CraftingExecutor::getInstance();
bool success = executor->craft(*recipe);
if (success) {
    CCLOG("Crafting successful!");
} else {
    CCLOG("Crafting failed!");
}
```

---

### 5. 测试工具

#### `EnttTest.h`
**职责：** 测试 EnTT ECS 框架的集成。

**主要函数：**
```cpp
bool runBasicTest();         // 基本功能测试
void runPerformanceTest();   // 性能测试
```

**使用位置：** `ItemsTestScene::init()`

---

## 系统交互流程

### 物品添加流程
```
用户操作 → Inventory::addItem()
         ↓
    更新内部数据
         ↓
    发送 Event_InventoryChanged
         ↓
    InventoryLayer::refresh() 刷新 UI
```

### 合成流程
```
用户点击配方 → CraftBar::onCategoryButtonClicked()
              ↓
         第一次点击：选中并高亮
              ↓
         第二次点击：调用 CraftingExecutor::craft()
              ↓
    CraftingExecutor 检查 CraftingMatcher::canCraft()
              ↓
         扣除材料（Inventory::removeItem）
              ↓
         添加产出（Inventory::addItem）
              ↓
    发送 Event_InventoryChanged
              ↓
    CraftBar::refreshRecipes() 刷新合成栏
```

### 工作台变化流程
```
玩家靠近工作台 → StationDetector::addStation()
               ↓
          发送 Event_StationChanged
               ↓
          CraftBar::refreshRecipes()
               ↓
          更新可用配方列表
```

---

## 数据文件

### `Resources/data/items.json`
定义所有物品的属性数据。

**格式示例：**
```json
{
  "items": [
    {
      "id": 1001,
      "name": "Wood",
      "type": 5,
      "maxStack": 999,
      "iconPath": "items/icons/wood.png",
      "description": "Basic building material"
    }
  ]
}
```

### `Resources/data/recipes.json`
定义所有合成配方。

**格式示例：**
```json
{
  "recipes": [
    {
      "resultItemId": 2101,
      "resultCount": 1,
      "requiredStation": 0,
      "ingredients": [
        { "itemId": 1001, "count": 10 }
      ]
    }
  ]
}
```

---

## 设计模式

### 单例模式
所有管理器类使用单例模式：
- `ItemManager::getInstance()`
- `Inventory::getInstance()`
- `RecipeManager::getInstance()`
- `StationDetector::getInstance()`
- `CraftingMatcher::getInstance()`
- `CraftingExecutor::getInstance()`

### 观察者模式
使用 Cocos2d-x 事件系统实现：
- `Event_InventoryChanged` - 物品栏改变
- `Event_StationChanged` - 工作台状态改变

### 职责分离
合成系统分为四个独立模块：
- **RecipeManager**：数据管理
- **StationDetector**：工作台检测
- **CraftingMatcher**：合成判断
- **CraftingExecutor**：合成执行

---

## 关键常量

### 物品类型
```cpp
1 = 装备（Equipment）
2 = 武器（Weapon）
3 = 材料（Material）
4 = 消耗品（Consumable）
5 = 可放置（Placeable）
```

### 工作台类型
```cpp
0 = Hand (徒手)
1 = Workbench (工作台)
2 = Furnace (熔炉)
3 = Anvil (铁砧)
```

### UI 纹理路径
```cpp
"items/bottom/Inventory.png"      // 普通槽位背景
"items/bottom/dark_bottom.png"    // 暗色背景（不可用）
"items/bottom/light_bottom.png"   // 高亮背景（选中）
"items/bottom/Helmet.png"         // 头盔槽
"items/bottom/Chestplate.png"     // 胸甲槽
"items/bottom/Leggings.png"       // 护腿槽
"items/bottom/Accessory.png"      // 饰品槽
"items/bottom/crafting.png"       // 合成按钮
```

---

## 待实现功能

### 装备系统集成
- [ ] 装备面板与 PlayerInventoryIntegration 集成
- [ ] 装备拖拽穿戴/卸下功能
- [ ] 装备属性加成显示

### 合成系统增强
- [ ] 批量合成（按住 Shift 点击）
- [ ] 配方收藏/固定功能
- [ ] 配方解锁条件（首次获得材料后解锁）

### 物品栏增强
- [ ] 物品搜索/过滤功能
- [ ] 物品快速使用（右键）
- [ ] 物品快捷栏（数字键1-9）

---

## 使用示例

### 完整合成流程示例
```cpp
// 1. 初始化系统
auto itemMgr = ItemManager::getInstance();
itemMgr->loadItemData("data/items.json");

auto recipeMgr = RecipeManager::getInstance();
recipeMgr->loadRecipes("data/recipes.json");

auto inventory = Inventory::getInstance();
inventory->init();

// 2. 添加材料
inventory->addItem(1001, 100);  // 添加 100 个木头

// 3. 添加工作台
auto detector = StationDetector::getInstance();
detector->addStation(StationType::Workbench);

// 4. 查询可用配方
auto matcher = CraftingMatcher::getInstance();
auto recipes = matcher->getAvailableRecipes();

// 5. 执行合成
if (!recipes.empty()) {
    auto recipe = recipes[0];
    if (matcher->canCraft(*recipe)) {
        auto executor = CraftingExecutor::getInstance();
        bool success = executor->craft(*recipe);
        CCLOG("Craft result: %s", success ? "SUCCESS" : "FAILED");
    }
}
```

---

## 相关文件

### 测试场景
- `Classes/ItemsTestScene.h/cpp` - 物品系统测试场景

### 玩家系统集成
- `Classes/player/PlayerInventoryIntegration.h` - 玩家装备槽定义

---

## 注意事项

1. **线程安全**：所有单例均非线程安全，仅适用于单线程环境
2. **内存管理**：所有 UI 组件继承自 Cocos2d-x Node，由引擎自动管理内存
3. **事件监听**：记得在节点销毁时移除事件监听器
4. **数据文件**：确保 JSON 文件格式正确，否则会导致加载失败
5. **两次点击机制**：合成系统使用两次点击防止误操作，第一次选中，第二次执行

---

## 更新日志

- **2025-12-18**：创建说明文件，记录物品系统完整架构
- **2025-12-18**：实现装备面板 UI
- **2025-12-18**：实现合成系统两次点击机制
- **2025-12-18**：实现快速合成栏（横向图标列表）
