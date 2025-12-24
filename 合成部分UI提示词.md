# 泰拉瑞亚风格 - 左下角制作栏UI设计规范

## 📋 前置说明

### 已完成的基础功能
- ✅ 物品栏UI（InventoryLayer）- 59格背包，支持拖拽、堆叠
- ✅ 物品管理系统（ItemManager）- 基于EnTT的物品数据管理
- ✅ 背包系统（Inventory）- 物品增删、溢出处理

### 技术栈
- **引擎**: Cocos2d-x（C++）
- **平台**: Windows PC（鼠标+键盘）
- **UI风格**: 像素风、深色半透明、类泰拉瑞亚

---

## 🎨 整体布局设计

### 布局方案（左下角制作栏）
```
┌─────────────────────────────────────────────────────┐
│  屏幕布局 (16:9)                                      │
├─────────────────────────────────────────────────────┤
│  ┌────────────┐                                     │
│  │  物品栏    │                                     │
│  │  (已完成)  │           游戏区域                   │
│  │  10x5格子  │                                     │
│  │  + 弹药栏  │                                     │
│  │  + 钱币栏  │                                     │
│  └────────────┘                                     │
│                                                     │
│                        ┌────────────────────┐      │
│                        │  配方列表(展开态)   │      │
│                        │  ┌──────────────┐  │      │
│                        │  │[图]工作台  ● │  │      │
│                        │  │[图]木门    ● │  │      │
│  ┌───────────────┐     │  │[图]木墙    ● │  │      │
│  │ [全部]  ○    │     │  │[图]木椅    ○ │  │      │
│  │ [工具]  □    │     │  └──────────────┘  │      │
│  │ [建筑]  ■    │     │  缺少:木头x3        │      │
│  │ [消耗]  ◇    │     │  [Craft x1] [Max]  │      │
│  │              │     └────────────────────┘      │
│  │ ┌─────────┐  │          ▲ 向上弹出              │
│  │ │ [选中]  │  │          │                       │
│  │ │ 工作台  │  │          │                       │
│  │ │  图标   │  │          │                       │
│  │ └─────────┘  │          │                       │
│  │  制作/Craft  │          │                       │
│  │  [展开▲]     │──────────┘                       │
│  └───────────────┘                                  │
│    ▲ 左下角固定(收起态)                              │
└─────────────────────────────────────────────────────┘
```

### 尺寸规范
- **制作栏(收起态)**: 左下角固定，宽度约80px，高度约350px
- **配方列表(展开态)**: 从制作栏向上弹出，宽度约280px，高度约400px
- **安全边距**: 距离屏幕边缘20px
- **当前配方槽**: 56-64px方形
- **分类按钮**: 48x48px，竖向排列，间距6px

---

## 📦 核心组件设计

### 1. 制作栏容器（收起态）- CraftBar

**视觉风格**:
- 深蓝半透明背景 (Color4B: 15, 25, 45, 230)
- 2px浅色像素描边 (Color3B: 180, 200, 220)
- 轻微浮雕效果（内阴影+外高光）

**组件结构**:
```cpp
CraftBar (Layer)  // 位置: 屏幕左下角(20, 20)
├── _background (DrawNode)                // 半透明背景面板
├── _categoryButtons (std::vector<Button>) // 分类按钮列（竖向）
│   ├── [全部]  All
│   ├── [工具]  Tools
│   ├── [建筑]  Blocks
│   ├── [武器]  Weapons
│   ├── [消耗]  Consumables
│   └── [材料]  Materials
├── _selectedRecipeSlot (Node)            // 当前选中配方槽
│   ├── _slotBorder (DrawNode)           // 槽位边框（可合成时黄色高亮）
│   ├── _resultIcon (Sprite)             // 产物图标
│   ├── _countLabel (Label)              // 数量角标
│   └── _statusIndicators (Node)         // 状态角标（材料不足红点/站点不足黄点）
├── _titleLabel (Label)                   // "制作 / Craft"文字
├── _expandButton (ui::Button)            // 展开/收起按钮
└── _recipeListPanel (RecipeListPanel)    // 配方列表（展开时显示）
```

**位置计算**:
```cpp
void CraftBar::init() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    float margin = 20.0f;

    // 左下角定位
    this->setPosition(Vec2(margin, margin));

    // 背景面板尺寸
    float barWidth = 80.0f;
    float barHeight = 350.0f;

    // 分类按钮从上往下排列
    float buttonSize = 48.0f;
    float buttonGap = 6.0f;
    float startY = barHeight - 60.0f;  // 留出顶部空间给标题

    for (int i = 0; i < _categoryButtons.size(); ++i) {
        _categoryButtons[i]->setPosition(
            Vec2(barWidth / 2, startY - i * (buttonSize + buttonGap))
        );
    }

    // 选中配方槽位于底部
    _selectedRecipeSlot->setPosition(Vec2(barWidth / 2, 100));
}
```

---

### 2. 分类按钮 (CategoryButton)

**视觉设计**:
- **Normal态**: 灰色背景，图标正常色
- **Selected态**: 亮色背景(60,90,130)，图标高亮，外发光
- **Disabled态**: 置灰50%透明度（未解锁类别）

**图标系统**:
```cpp
enum class RecipeCategory {
    All,          // 图标: ☰ 三横线
    Tools,        // 图标: ⚒ 锤子
    Blocks,       // 图标: ■ 方块
    Weapons,      // 图标: ⚔ 剑
    Consumables,  // 图标: ⚗ 药瓶
    Materials     // 图标: ◆ 钻石
};

void CategoryButton::setState(ButtonState state) {
    switch (state) {
        case Normal:
            _background->setColor(Color3B(40, 50, 70));
            _icon->setOpacity(200);
            break;
        case Selected:
            _background->setColor(Color3B(60, 90, 130));
            _icon->setOpacity(255);
            // 添加外发光
            auto glow = Sprite::create("glow_effect.png");
            glow->setBlendFunc({GL_SRC_ALPHA, GL_ONE}); // 叠加混合
            this->addChild(glow, -1);
            break;
        case Disabled:
            _background->setColor(Color3B(30, 30, 30));
            _icon->setOpacity(100);
            break;
    }
}
```

**点击事件**:
```cpp
void CraftBar::onCategoryClicked(RecipeCategory category) {
    // 1. 更新按钮选中状态
    for (auto& btn : _categoryButtons) {
        btn->setState(btn->getCategory() == category ? Selected : Normal);
    }

    // 2. 筛选配方
    _currentCategory = category;
    refreshRecipes();
}
```

---

### 3. 当前配方槽 (SelectedRecipeSlot)

**功能**: 显示当前选中的配方产物，带状态反馈

**尺寸**: 64x64px方形槽

**状态显示**:
```
┌──────────────┐
│ ┏━━━━━━━━━┓ │  ← 可合成: 黄色高亮边框
│ ┃ [工作台] ┃ │
│ ┃   图标   ┃ │
│ ┃    x1  ● ┃ │  ← 绿点: 可合成
│ ┗━━━━━━━━━┛ │
└──────────────┘

┌──────────────┐
│ ┌─────────── │  ← 不可合成: 灰色边框
│ │ [铁剑]   ● │  ← 红点: 缺材料
│ │   图标   🔧 │  ← 黄色扳手: 缺工作站
│ │    x1      │
│ └─────────── │
└──────────────┘
```

**实现代码**:
```cpp
void SelectedRecipeSlot::setRecipe(const RecipeDefinition* recipe) {
    _currentRecipe = recipe;

    if (!recipe) {
        clearSlot();
        return;
    }

    // 1. 更新图标
    auto itemData = ItemManager::getInstance()->getItemData(recipe->resultItemId);
    _resultIcon->setSpriteFrame(
        ItemManager::getInstance()->getItemSprite(recipe->resultItemId)
    );

    // 2. 更新数量
    _countLabel->setString(StringUtils::format("x%d", recipe->resultCount));

    // 3. 检查状态
    bool canCraft = CraftingMatcher::getInstance()->canCraft(*recipe);
    bool hasMaterials = checkMaterials(recipe);
    bool hasStation = checkStation(recipe);

    // 4. 更新边框颜色
    if (canCraft) {
        _slotBorder->setColor(Color3B(255, 220, 80));  // 黄色高亮
        _slotBorder->setOpacity(255);
    } else {
        _slotBorder->setColor(Color3B(100, 100, 100)); // 灰色
        _slotBorder->setOpacity(180);
    }

    // 5. 更新状态点
    _statusDot->setColor(!hasMaterials ? Color3B::RED :
                        !hasStation ? Color3B::YELLOW :
                        Color3B::GREEN);

    // 6. 显示缺失图标
    if (!hasStation) {
        _missingStationIcon->setVisible(true);
    }
}
```

---

### 4. 配方列表面板（展开态）- RecipeListPanel

**功能**: 从制作栏向上弹出，显示当前分类的所有配方

**尺寸**: 280x400px

**位置**: 制作栏正上方，间距10px

**布局**:
```
改进中ing
```

**展开/收起动画**:
```cpp
void CraftBar::toggleRecipeList() {
    if (_isExpanded) {
        // 收起动画
        auto moveDown = MoveTo::create(0.2f, Vec2(
            _recipeListPanel->getPositionX(),
            -_recipeListPanel->getContentSize().height  // 移到屏幕外
        ));
        auto fade = FadeOut::create(0.15f);
        _recipeListPanel->runAction(Sequence::create(
            Spawn::create(moveDown, fade, nullptr),
            Hide::create(),
            nullptr
        ));
        _expandButton->setTitleText("▲");
    } else {
        // 展开动画
        _recipeListPanel->setVisible(true);
        auto targetY = this->getContentSize().height + 10;  // 制作栏上方
        auto moveUp = MoveTo::create(0.2f, Vec2(
            _recipeListPanel->getPositionX(),
            targetY
        ));
        auto fade = FadeIn::create(0.15f);
        _recipeListPanel->runAction(Spawn::create(moveUp, fade, nullptr));
        _expandButton->setTitleText("▼");
    }
    _isExpanded = !_isExpanded;
}
```

---

### 5. 配方列表项 (RecipeListItem)

**每行结构**:
```
┌──────────────────────────────────────┐
│ [48x48 图标]  工作台        x1   ●   │
│               (Placeable)            │
└──────────────────────────────────────┘
  ↑ 图标      ↑ 名称  ↑ 类型  ↑数量 ↑状态点
```

**状态点颜色**:
- 🟢 绿色: 可合成
- 🔴 红色: 缺材料
- 🟡 黄色: 缺工作站
- ⚫ 灰色: 同时缺材料和工作站

**交互状态**:
```cpp
void RecipeListItem::setState(ItemState state) {
    switch (state) {
        case Normal:
            _background->setColor(Color3B(30, 40, 60));
            _background->setOpacity(0);
            break;
        case Hovered:  // PC鼠标悬停
            _background->setColor(Color3B(50, 70, 100));
            _background->setOpacity(150);
            break;
        case Selected:  // 选中态
            _background->setColor(Color3B(80, 110, 150));
            _background->setOpacity(200);
            // 黄色描边
            auto border = DrawNode::create();
            border->drawRect(Vec2::ZERO,
                           this->getContentSize(),
                           Color4F(1.0f, 0.9f, 0.3f, 1.0f));
            this->addChild(border, -1);
            break;
        case Disabled:  // 不可合成
            _icon->setOpacity(120);
            _nameLabel->setOpacity(120);
            _statusLabel->setOpacity(120);
            break;
    }
}
```

**点击事件**:
```cpp
void RecipeListItem::onClicked() {
    // 1. 更新制作栏的当前配方槽
    _parentCraftBar->setSelectedRecipe(_recipe);

    // 2. 如果不可合成，显示缺失材料提示
    if (!_canCraft) {
        showMissingMaterialsHint();
    }

    // 3. 更新选中状态
    setState(Selected);
}

void RecipeListItem::showMissingMaterialsHint() {
    std::string hintText = "缺少: ";
    int count = 0;

    auto inv = Inventory::getInstance();
    for (const auto& ingredient : _recipe->ingredients) {
        int owned = inv->getItemCount(ingredient.itemId);
        if (owned < ingredient.count) {
            if (count > 0) hintText += ", ";
            auto itemData = ItemManager::getInstance()->getItemData(ingredient.itemId);
            hintText += itemData->name + " x" +
                       std::to_string(ingredient.count - owned);
            count++;
            if (count >= 2) {  // 最多显示2条
                hintText += "...";
                break;
            }
        }
    }

    // 显示在面板底部
    _parentPanel->setHintText(hintText);
}
```

---

### 6. 合成按钮 (CraftButtons)

**布局**: 配方列表底部，两个按钮横向排列

**Craft x1 按钮**:
```cpp
auto craftOneBtn = ui::Button::create("button_normal.png",
                                      "button_pressed.png",
                                      "button_disabled.png");
craftOneBtn->setTitleText("Craft x1");
craftOneBtn->setTitleFontSize(14);
craftOneBtn->setTitleColor(Color3B::WHITE);

// 根据状态设置启用/禁用
bool canCraft = CraftingMatcher::getInstance()->canCraft(*_currentRecipe);
craftOneBtn->setEnabled(canCraft);

// 点击事件
craftOneBtn->addClickEventListener([this](Ref*) {
    bool success = CraftingExecutor::getInstance()->craft(*_currentRecipe);
    if (success) {
        // 播放合成特效
        showCraftSuccessEffect();
        // 刷新配方列表
        refreshRecipes();
    }
});

// Disabled提示
if (!canCraft) {
    std::string reason = getDisableReason();  // "缺材料" / "缺工作站" / "背包满"
    craftOneBtn->setTitleText("Craft x1\n(" + reason + ")");
}
```

**Craft Max 按钮**（合成最大数量）:
```cpp
auto craftMaxBtn = ui::Button::create(...);
craftMaxBtn->setTitleText("Craft Max");

craftMaxBtn->addClickEventListener([this](Ref*) {
    int maxCount = calculateMaxCraftCount(_currentRecipe);

    for (int i = 0; i < maxCount; ++i) {
        CraftingExecutor::getInstance()->craft(*_currentRecipe);
    }

    showCraftSuccessEffect();
    refreshRecipes();
});

// 计算最大合成数量
int calculateMaxCraftCount(const RecipeDefinition* recipe) {
    int maxCount = INT_MAX;
    auto inv = Inventory::getInstance();

    // 根据每种材料的数量计算
    for (const auto& ingredient : recipe->ingredients) {
        int owned = inv->getItemCount(ingredient.itemId);
        int possibleCount = owned / ingredient.count;
        maxCount = std::min(maxCount, possibleCount);
    }

    // 考虑背包空间
    int emptySlots = inv->getEmptySlotCount();
    maxCount = std::min(maxCount, emptySlots);

    return maxCount;
}
```

**长按连续合成**:
```cpp
void CraftButtons::setupLongPressCraft() {
    auto listener = EventListenerTouchOneByOne::create();

    listener->onTouchBegan = [this](Touch* touch, Event* event) {
        if (!_craftOneBtn->getBoundingBox().containsPoint(touch->getLocation())) {
            return false;
        }

        // 开始长按计时
        _longPressTimer = 0.0f;
        _isLongPressing = false;
        return true;
    };

    listener->onTouchMoved = [this](Touch* touch, Event* event) {
        _longPressTimer += 0.016f;  // 假设60fps

        if (!_isLongPressing && _longPressTimer > 0.5f) {
            // 0.5秒后触发长按
            _isLongPressing = true;
            startContinuousCrafting();
        }
    };

    listener->onTouchEnded = [this](Touch* touch, Event* event) {
        if (_isLongPressing) {
            stopContinuousCrafting();
        } else {
            // 短按：合成一次
            CraftingExecutor::getInstance()->craft(*_currentRecipe);
        }
    };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, _craftOneBtn);
}

void CraftButtons::startContinuousCrafting() {
    // 每0.2秒合成一次
    _continuousCraftAction = this->schedule([this](float dt) {
        bool success = CraftingExecutor::getInstance()->craft(*_currentRecipe);
        if (!success) {
            stopContinuousCrafting();
        } else {
            _craftCount++;
            updateCraftCountDisplay();  // 显示"已合成 x5"
        }
    }, 0.2f, "continuous_craft");
}
```

---

### 7. 合成反馈特效

**成功合成时**:
```cpp
void CraftBar::showCraftSuccessEffect() {
    // 1. 槽位闪光
    auto flash = Sequence::create(
        TintTo::create(0.1f, 255, 255, 150),
        TintTo::create(0.1f, 255, 255, 255),
        TintTo::create(0.1f, 255, 255, 150),
        TintTo::create(0.1f, 255, 255, 255),
        nullptr
    );
    _selectedRecipeSlot->runAction(flash);

    // 2. 轻微抖动
    auto shake = Sequence::create(
        MoveBy::create(0.03f, Vec2(2, 0)),
        MoveBy::create(0.03f, Vec2(-4, 0)),
        MoveBy::create(0.03f, Vec2(4, 0)),
        MoveBy::create(0.03f, Vec2(-2, 0)),
        nullptr
    );
    _selectedRecipeSlot->runAction(shake);

    // 3. 粒子效果（可选）
    auto particles = ParticleSystemQuad::create("craft_sparkle.plist");
    particles->setPosition(_selectedRecipeSlot->getPosition());
    this->addChild(particles);
    particles->setAutoRemoveOnFinish(true);
}
```

**背包满时Toast提示**:
```cpp
void CraftBar::showInventoryFullToast() {
    auto toast = Node::create();
    toast->setContentSize(Size(250, 40));

    // 半透明背景
    auto bg = DrawNode::create();
    bg->drawSolidRect(Vec2::ZERO, Size(250, 40), Color4F(0.8f, 0.2f, 0.2f, 0.9f));
    toast->addChild(bg);

    // 文字
    auto label = Label::createWithSystemFont("背包已满！物品已掉落", "Arial", 14);
    label->setPosition(Vec2(125, 20));
    label->setColor(Color3B::WHITE);
    toast->addChild(label);

    // 位置：制作栏上方
    toast->setPosition(Vec2(
        this->getPositionX() + 40,
        this->getPositionY() + this->getContentSize().height + 10
    ));

    // 添加到游戏场景
    Director::getInstance()->getRunningScene()->addChild(toast, 1000);

    // 2秒后消失
    toast->runAction(Sequence::create(
        DelayTime::create(2.0f),
        FadeOut::create(0.5f),
        RemoveSelf::create(),
        nullptr
    ));
}
```

---

## 🔧 与现有系统集成

### 初始化和事件监听
```cpp
bool CraftBar::init() {
    if (!Layer::init()) return false;

    // 1. 创建UI组件
    buildCategoryButtons();
    buildRecipeSlot();
    buildExpandButton();
    buildRecipeListPanel();

    // 2. 设置位置（左下角）
    auto visibleSize = Director::getInstance()->getVisibleSize();
    this->setPosition(Vec2(20, 20));

    // 3. 监听背包变化
    auto dispatcher = Director::getInstance()->getEventDispatcher();
    dispatcher->addCustomEventListener("Event_InventoryChanged",
        CC_CALLBACK_1(CraftBar::onInventoryChanged, this));

    // 4. 监听工作站变化
    dispatcher->addCustomEventListener("Event_StationChanged",
        CC_CALLBACK_1(CraftBar::onStationChanged, this));

    // 5. 初始刷新
    setCategory(RecipeCategory::All);
    refreshRecipes();

    return true;
}

void CraftBar::onInventoryChanged(EventCustom* event) {
    refreshRecipes();
    updateSelectedRecipeSlot();
}

void CraftBar::onStationChanged(EventCustom* event) {
    refreshRecipes();
}
```

### 配方刷新逻辑
```cpp
void CraftBar::refreshRecipes() {
    // 1. 获取当前分类的所有配方
    auto allRecipes = RecipeManager::getInstance()->getAllRecipes();
    std::vector<const RecipeDefinition*> filteredRecipes;

    for (const auto& recipe : allRecipes) {
        // 根据分类筛选
        if (_currentCategory != RecipeCategory::All) {
            auto itemData = ItemManager::getInstance()->getItemData(recipe.resultItemId);
            if (getCategoryForItem(itemData) != _currentCategory) {
                continue;
            }
        }
        filteredRecipes.push_back(&recipe);
    }

    // 2. 按可合成状态排序（可合成的排前面）
    std::sort(filteredRecipes.begin(), filteredRecipes.end(),
        [](const RecipeDefinition* a, const RecipeDefinition* b) {
            bool canCraftA = CraftingMatcher::getInstance()->canCraft(*a);
            bool canCraftB = CraftingMatcher::getInstance()->canCraft(*b);
            return canCraftA > canCraftB;  // 可合成的在前
        }
    );

    // 3. 更新列表UI
    _recipeListPanel->setRecipes(filteredRecipes);

    // 4. 如果没有选中配方，自动选中第一个可合成的
    if (!_currentRecipe && !filteredRecipes.empty()) {
        setSelectedRecipe(filteredRecipes[0]);
    }
}
```

---

## 📝 实现清单

### 文件结构
```
Classes/items/
├── CraftBar.h/.cpp                // 左下角制作栏主容器
├── CategoryButton.h/.cpp          // 分类按钮
├── SelectedRecipeSlot.h/.cpp      // 当前配方槽
├── RecipeListPanel.h/.cpp         // 配方列表面板（展开态）
├── RecipeListItem.h/.cpp          // 配方列表项
└── CraftButtons.h/.cpp            // Craft x1 / Max 按钮
```

### 实现步骤
1. ✅ 创建 CraftBar 基础框架（收起态）
2. ✅ 实现 CategoryButton（分类筛选）
3. ✅ 实现 SelectedRecipeSlot（当前配方显示）
4. ✅ 实现 RecipeListPanel（展开态）
5. ✅ 实现 RecipeListItem（配方行项）
6. ✅ 实现 CraftButtons（合成按钮+长按）
7. ✅ 添加展开/收起动画
8. ✅ 添加合成特效和反馈

---

## 🎯 关键要点

1. **固定位置**: 左下角固定，不随屏幕滚动
2. **向上展开**: 配方列表从制作栏向上弹出，避免遮挡操作区
3. **分类筛选**: 竖向分类按钮，快速切换配方类型
4. **状态反馈**: 清晰的颜色编码（绿/红/黄）和图标提示
5. **长按连做**: 支持长按连续合成，提升操作效率
6. **像素风格**: 所有UI使用像素字体、九宫格切片、像素描边
7. **性能优化**: 只刷新变化的部分，避免每帧重建

---

## 🎨 交互状态总结

### 分类按钮状态
- **Normal**: 灰色背景(40,50,70)，图标200透明度
- **Selected**: 亮色背景(60,90,130)，图标255透明度+外发光
- **Disabled**: 深灰背景(30,30,30)，图标100透明度

### 当前配方槽状态
- **可合成**: 黄色高亮边框(255,220,80)，绿色状态点
- **缺材料**: 灰色边框，红色状态点
- **缺工作站**: 灰色边框，黄色状态点+扳手图标
- **背包满**: 灰色边框，红色角标

### Craft按钮状态
- **可按**: 亮色按钮，白色文字
- **禁用**: 灰色按钮，显示原因文字
- **长按**: 显示进度条和已合成数量

### 配方列表项状态
- **Normal**: 透明背景
- **Hovered**: 浅色背景高亮(50,70,100)
- **Selected**: 亮色背景+黄色描边(80,110,150)
- **Disabled**: 图标和文字120透明度

---

**状态**: 📝 设计完成，待实现
**风格**: 泰拉瑞亚左下角制作栏
**特点**: 紧凑、高效、像素风
