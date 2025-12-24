# PlayerInventoryIntegration 使用示例

## 概述

`PlayerInventoryIntegration` 是连接玩家系统（ECS组件）和物品/背包系统（单例）的桥接层。

## 核心功能

### 1. 装备管理

#### 装备物品
```cpp
#include "player/PlayerInventoryIntegration.h"

// 从背包第5格装备到头盔槽
bool success = PlayerInventoryBridge::equipItem(
    registry,
    playerEntity,
    5,  // 背包槽位索引
    PlayerInventoryBridge::EquipmentSlotType::Helmet
);

if (success) {
    CCLOG("装备成功！");
}
```

#### 卸下装备
```cpp
// 卸下头盔
bool success = PlayerInventoryBridge::unequipItem(
    registry,
    playerEntity,
    PlayerInventoryBridge::EquipmentSlotType::Helmet
);
```

#### 装备槽位类型
```cpp
enum class EquipmentSlotType {
    Helmet,       // 头盔
    Chestplate,   // 胸甲
    Leggings,     // 护腿
    Accessory0,   // 饰品槽 1
    Accessory1,   // 饰品槽 2
    Accessory2,   // 饰品槽 3
    Accessory3,   // 饰品槽 4
    Accessory4,   // 饰品槽 5
    Accessory5    // 饰品槽 6
};
```

### 2. 快捷栏使用

#### 获取当前快捷栏物品
```cpp
int itemId = PlayerInventoryBridge::getCurrentHotbarItemId(
    registry,
    playerEntity
);

if (itemId != 0) {
    CCLOG("当前手持物品: %d", itemId);
}
```

#### 使用快捷栏物品
```cpp
// 使用当前选中的快捷栏物品
bool success = PlayerInventoryBridge::useCurrentHotbarItem(
    registry,
    playerEntity
);

// 或者使用指定快捷栏槽位的物品
bool success = PlayerInventoryBridge::useHotbarItem(
    registry,
    playerEntity,
    3  // 快捷栏索引 (0-9)
);
```

### 3. 物品操作

#### 消耗物品
```cpp
// 消耗3个物品ID为101的物品（例如：药水）
bool success = PlayerInventoryBridge::consumeItem(101, 3);

if (!success) {
    CCLOG("物品不足！");
}
```

#### 给予物品
```cpp
// 给玩家10个物品ID为202的物品（例如：铜矿）
bool success = PlayerInventoryBridge::giveItem(202, 10);

if (!success) {
    CCLOG("背包已满！");
}
```

#### 检查物品
```cpp
// 检查玩家是否有至少5个物品ID为303的物品
bool hasEnough = PlayerInventoryBridge::hasItem(303, 5);

if (hasEnough) {
    CCLOG("材料充足！");
}
```

### 4. 属性计算

#### 重新计算装备属性
```cpp
// 当装备变化时自动调用，也可以手动调用
PlayerInventoryBridge::calculateEquipmentStats(
    registry,
    playerEntity
);

// 这会更新 PlayerStatsComponent 中的：
// - defense（防御力）
// - meleeDamageBonus（近战伤害加成）
// - rangedDamageBonus（远程伤害加成）
// - magicDamageBonus（魔法伤害加成）
```

#### 检查套装
```cpp
int armorSetId = PlayerInventoryBridge::checkArmorSet(
    registry,
    playerEntity
);

if (armorSetId > 0) {
    CCLOG("玩家穿着套装: %d", armorSetId);
    // TODO: 应用套装加成
}
```

## 完整示例：在 PlayerTestScene 中集成

### 示例1：添加装备按键

```cpp
// 在 PlayerTestScene.cpp 的 init() 中添加键盘监听

auto keyListener = EventListenerKeyboard::create();
keyListener->onKeyPressed = [this](EventKeyboard::KeyCode keyCode, Event* event) {
    if (keyCode == EventKeyboard::KeyCode::KEY_E) {
        // 按E键装备当前选中的物品到对应槽位
        auto& hotbar = _registry.get<ecs::PlayerHotbarComponent>(_playerEntity);
        int currentSlot = hotbar.getCurrentInventoryIndex();

        // 假设自动判断槽位（需要根据物品类型）
        // 这里简单演示装备到头盔槽
        bool success = PlayerInventoryBridge::equipItem(
            _registry,
            _playerEntity,
            currentSlot,
            PlayerInventoryBridge::EquipmentSlotType::Helmet
        );

        if (success) {
            CCLOG("装备成功！");
        }
    }
    else if (keyCode == EventKeyboard::KeyCode::KEY_Q) {
        // 按Q键卸下头盔
        PlayerInventoryBridge::unequipItem(
            _registry,
            _playerEntity,
            PlayerInventoryBridge::EquipmentSlotType::Helmet
        );
    }
};

_eventDispatcher->addEventListenerWithSceneGraphPriority(keyListener, this);
```

### 示例2：添加调试显示装备信息

```cpp
// 在 PlayerTestScene 的 updateDebugInfo() 中添加

void PlayerTestScene::updateDebugInfo(float dt) {
    // ... 现有的调试信息 ...

    // 添加装备信息
    auto& equipment = _registry.get<ecs::PlayerEquipmentComponent>(_playerEntity);
    auto& stats = _registry.get<ecs::PlayerStatsComponent>(_playerEntity);

    std::string equipInfo = StringUtils::format(
        "\n--- Equipment ---\n"
        "Helmet: %d\n"
        "Chestplate: %d\n"
        "Leggings: %d\n"
        "Accessories: %d %d %d %d %d %d\n"
        "Total Defense: %d",
        equipment.helmet.itemId,
        equipment.chestplate.itemId,
        equipment.leggings.itemId,
        equipment.accessories[0].itemId,
        equipment.accessories[1].itemId,
        equipment.accessories[2].itemId,
        equipment.accessories[3].itemId,
        equipment.accessories[4].itemId,
        equipment.accessories[5].itemId,
        stats.defense
    );

    // 添加到调试标签
    _debugLabel->setString(_debugLabel->getString() + equipInfo);
}
```

### 示例3：鼠标点击使用物品

```cpp
// 在 PlayerTestScene.cpp 的 init() 中添加鼠标监听

auto mouseListener = EventListenerMouse::create();
mouseListener->onMouseDown = [this](Event* event) {
    EventMouse* e = static_cast<EventMouse*>(event);

    if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {
        // 左键：使用当前快捷栏物品
        bool success = PlayerInventoryBridge::useCurrentHotbarItem(
            _registry,
            _playerEntity
        );

        if (success) {
            CCLOG("使用物品成功！");
        }
    }
};

_eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);
```

### 示例4：自动拾取物品并装备

```cpp
// 假设玩家击败敌人后掉落装备
void PlayerTestScene::onEnemyDefeated(int droppedItemId) {
    // 1. 先给玩家物品
    bool added = PlayerInventoryBridge::giveItem(droppedItemId, 1);

    if (!added) {
        CCLOG("背包已满，物品掉落地面");
        return;
    }

    // 2. 找到刚添加的物品在背包中的位置
    auto& inventory = Inventory::getInstance();
    auto& slots = inventory->getSlots();

    int slotIndex = -1;
    for (int i = 0; i < slots.size(); i++) {
        if (slots[i].itemId == droppedItemId) {
            slotIndex = i;
            break;
        }
    }

    if (slotIndex == -1) return;

    // 3. 根据物品类型自动装备
    // TODO: 需要从 ItemManager 获取物品类型
    // 这里简单演示装备到饰品槽0
    PlayerInventoryBridge::equipItem(
        _registry,
        _playerEntity,
        slotIndex,
        PlayerInventoryBridge::EquipmentSlotType::Accessory0
    );

    CCLOG("自动装备物品: %d", droppedItemId);
}
```

## 注意事项

### 1. 装备和背包的关系
当前实现中，装备系统是**独立于背包**的：
- 装备物品时，物品**不会**从背包移除（泰拉瑞亚风格）
- 卸下装备时，物品**不会**放回背包

如果需要改变这个行为，请修改：
- `PlayerInventoryIntegration.cpp` 第92-93行（装备时移除）
- `PlayerInventoryIntegration.cpp` 第123-126行（卸下时放回）

### 2. 物品类型判断
当前 `canEquipToSlot()` 函数只是简单返回 `true`。

实际使用时，你需要：
```cpp
bool canEquipToSlot(int itemId, EquipmentSlotType slotType) {
    // 从 ItemManager 获取物品信息
    ItemData* itemData = ItemManager::getInstance()->getItemData(itemId);
    if (!itemData) return false;

    // 根据物品类型判断
    switch (slotType) {
        case EquipmentSlotType::Helmet:
            return itemData->type == ItemType::Helmet;
        case EquipmentSlotType::Chestplate:
            return itemData->type == ItemType::Chestplate;
        case EquipmentSlotType::Leggings:
            return itemData->type == ItemType::Leggings;
        case EquipmentSlotType::Accessory0:
        case EquipmentSlotType::Accessory1:
        // ... 其他饰品槽 ...
            return itemData->type == ItemType::Accessory;
        default:
            return false;
    }
}
```

### 3. 属性计算
当前 `calculateEquipmentStats()` 使用**临时固定值**：
- 头盔：+2 防御
- 胸甲：+3 防御
- 护腿：+2 防御

实际使用时，你需要从 `ItemManager` 获取真实的装备属性。

### 4. 套装检测
当前 `checkArmorSet()` 只检查是否穿满了3件护甲，但不检查是否是同一套装。

实际使用时，你需要定义套装数据并检查物品ID是否匹配。

## 下一步扩展

### 待实现功能
- [ ] 从 ItemManager 获取物品类型和属性
- [ ] 完善 `canEquipToSlot()` 物品类型检查
- [ ] 完善 `calculateEquipmentStats()` 使用真实装备属性
- [ ] 实现套装数据和套装效果
- [ ] 实现 `autoEquipItem()` 自动判断槽位
- [ ] 实现 `useHotbarItem()` 中的物品使用逻辑（消耗品、武器、工具等）
- [ ] 添加装备UI显示
- [ ] 添加拖拽装备功能

## 测试清单

- [ ] 编译成功
- [ ] 可以装备物品到各个槽位
- [ ] 可以卸下装备
- [ ] 装备后防御力正确更新
- [ ] 快捷栏物品使用功能正常
- [ ] 物品消耗和给予功能正常
- [ ] 调试信息正确显示装备状态
