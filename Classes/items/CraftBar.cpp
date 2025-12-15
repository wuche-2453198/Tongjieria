#include "CraftBar.h"
#include "RecipeManager.h"
#include "StationDetector.h"
#include "CraftingMatcher.h"
#include "CraftingExecutor.h"
#include "ItemManager.h"
#include "Inventory.h"

using namespace cocos2d;
using namespace ui;

// Define static const members
const float CraftBar::kBarWidth = 200.0f;
const float CraftBar::kCollapsedHeight = 350.0f;
const float CraftBar::kExpandedWidth = 280.0f;
const float CraftBar::kExpandedHeight = 450.0f;
const float CraftBar::kCategoryScrollHeight = 240.0f;
const float CraftBar::kRecipeScrollHeight = 350.0f;
const float CraftBar::kRecipeItemHeight = 70.0f;
const float CraftBar::kScrollWheelSpeed = 60.0f;

CraftBar* CraftBar::create() {
    auto ret = new (std::nothrow) CraftBar();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool CraftBar::init() {
    if (!Layer::init()) return false;

    // Position at bottom-left corner with margin
    this->setPosition(Vec2(20, 20));

    setupCollapsedUI();
    setupEventListeners();
    refreshRecipes();

    CCLOG("CraftBar: initialized with scrolling window support");
    return true;
}

void CraftBar::setupCollapsedUI() {
    // Main background for collapsed state
    auto bg = LayerColor::create(Color4B(20, 30, 50, 240), kBarWidth, kCollapsedHeight);
    bg->setPosition(Vec2::ZERO);
    this->addChild(bg, -1);

    // Border
    auto border = DrawNode::create();
    Vec2 rect[4] = {
        Vec2(1, 1),
        Vec2(kBarWidth - 1, 1),
        Vec2(kBarWidth - 1, kCollapsedHeight - 1),
        Vec2(1, kCollapsedHeight - 1)
    };
    border->drawPoly(rect, 4, true, Color4F(0.85f, 0.74f, 0.40f, 0.9f));
    this->addChild(border, 0);

    // Title at top
    _titleLabel = Label::createWithSystemFont("Crafting", "Arial", 18);
    _titleLabel->setPosition(Vec2(kBarWidth * 0.5f, kCollapsedHeight - 25));
    _titleLabel->setColor(Color3B(255, 220, 100));
    this->addChild(_titleLabel, 1);

    // Selected recipe slot at bottom (fixed)
    setupSelectedRecipeSlot();

    // Category scroll view in the middle
    setupCategoryScrollView();
}

void CraftBar::setupSelectedRecipeSlot() {
    float slotY = 60;
    float slotHeight = 80;

    // Background for selected slot
    auto slotBg = LayerColor::create(Color4B(40, 50, 70, 200), kBarWidth - 10, slotHeight);
    slotBg->setPosition(Vec2(5, slotY));
    this->addChild(slotBg, 1);

    _selectedRecipeSlot = Node::create();
    _selectedRecipeSlot->setPosition(Vec2(kBarWidth * 0.5f, slotY + slotHeight * 0.5f));
    this->addChild(_selectedRecipeSlot, 2);

    // Icon placeholder
    _selectedIcon = Sprite::create();
    _selectedIcon->setPosition(Vec2(-60, 0));
    _selectedRecipeSlot->addChild(_selectedIcon);

    // Name label
    _selectedNameLabel = Label::createWithSystemFont("Select Recipe", "Arial", 12);
    _selectedNameLabel->setPosition(Vec2(20, 15));
    _selectedNameLabel->setDimensions(100, 0);
    _selectedNameLabel->setAlignment(TextHAlignment::LEFT);
    _selectedNameLabel->setColor(Color3B(220, 220, 220));
    _selectedRecipeSlot->addChild(_selectedNameLabel);

    // Count label
    _selectedCountLabel = Label::createWithSystemFont("", "Arial", 10);
    _selectedCountLabel->setPosition(Vec2(20, -5));
    _selectedCountLabel->setColor(Color3B(180, 180, 180));
    _selectedRecipeSlot->addChild(_selectedCountLabel);

    // Click to craft hint
    auto craftHint = Label::createWithSystemFont("Click to craft", "Arial", 9);
    craftHint->setPosition(Vec2(20, -18));
    craftHint->setColor(Color3B(150, 200, 150));
    _selectedRecipeSlot->addChild(craftHint);
}

void CraftBar::setupCategoryScrollView() {
    float scrollY = 150;
    float scrollWidth = kBarWidth - 10;

    // Create ScrollView with VERTICAL direction
    _categoryScrollView = ScrollView::create();
    _categoryScrollView->setContentSize(Size(scrollWidth, kCategoryScrollHeight));
    _categoryScrollView->setPosition(Vec2(5, scrollY));
    _categoryScrollView->setDirection(ScrollView::Direction::VERTICAL);
    _categoryScrollView->setBounceEnabled(true);
    _categoryScrollView->setScrollBarEnabled(false);
    _categoryScrollView->setClippingEnabled(true); // CRITICAL: Enable clipping
    _categoryScrollView->setSwallowTouches(true);
    this->addChild(_categoryScrollView, 1);

    // Create inner container for buttons
    auto container = Layout::create();
    container->setLayoutType(Layout::Type::VERTICAL);
    container->setContentSize(Size(scrollWidth, 0)); // Height will be calculated
    _categoryScrollView->setInnerContainerSize(container->getContentSize());
    _categoryScrollView->addChild(container);

    refreshCategoryButtons();

    // Setup mouse wheel listener for this scroll view
    setupMouseWheelListener();

    // Fade gradients to indicate scrollability
    updateFadeGradients();
}

void CraftBar::setupMouseWheelListener() {
    auto mouseListener = EventListenerMouse::create();
    mouseListener->onMouseScroll = [this](EventMouse* event) {
        if (!_categoryScrollView) return;

        // Check if mouse is over the scroll view
        Vec2 mousePos = event->getLocation();
        auto visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 worldPos(mousePos.x, visibleSize.height - mousePos.y);

        // Convert to scroll view local space
        Vec2 scrollViewWorldPos = _categoryScrollView->getParent()->convertToWorldSpace(
            _categoryScrollView->getPosition());

        Rect scrollViewRect(
            scrollViewWorldPos.x,
            scrollViewWorldPos.y,
            _categoryScrollView->getContentSize().width,
            _categoryScrollView->getContentSize().height
        );

        if (scrollViewRect.containsPoint(worldPos)) {
            // Scroll with mouse wheel
            float scrollY = event->getScrollY();
            Vec2 currentOffset = _categoryScrollView->getInnerContainerPosition();
            float newY = currentOffset.y + scrollY * kScrollWheelSpeed;

            // Clamp to valid range
            float minY = _categoryScrollView->getContentSize().height -
                         _categoryScrollView->getInnerContainerSize().height;
            float maxY = 0;
            if (minY > 0) minY = 0;

            newY = std::max(minY, std::min(maxY, newY));
            _categoryScrollView->setInnerContainerPosition(Vec2(0, newY));

            updateScrollIndicator();
            event->stopPropagation();
        }
    };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);
}

void CraftBar::refreshCategoryButtons() {
    if (!_categoryScrollView) return;

    // Clear existing buttons
    auto container = _categoryScrollView->getInnerContainer();
    container->removeAllChildren();

    // Get available recipes grouped by result type
    auto recipes = CraftingMatcher::getInstance()->getAvailableRecipes();

    // For now, create a simple list of all available recipes as categories
    // In a full implementation, you would group by tags/categories
    float buttonHeight = 45;
    float buttonSpacing = 5;
    float totalHeight = 0;

    for (size_t i = 0; i < recipes.size(); ++i) {
        auto recipe = recipes[i];
        auto itemDef = ItemManager::getInstance()->getItemData(recipe->resultItemId);

        // Debug: Log item lookup result
        if (!itemDef) {
            CCLOG("CraftBar WARNING: Cannot find item definition for ID %d", recipe->resultItemId);
            CCLOG("  Total items in ItemManager: %zu", ItemManager::getInstance()->getItemCount());
        } else {
            CCLOG("CraftBar: Found item ID %d = %s", recipe->resultItemId, itemDef->name.c_str());
        }

        // Create button background manually
        auto buttonBg = LayerColor::create(Color4B(60, 80, 120, 200),
                                           kBarWidth - 20, buttonHeight);

        // Create label for button text
        auto label = Label::createWithSystemFont(
            itemDef ? itemDef->name : "Unknown",
            "Arial", 12);
        label->setPosition(Vec2((kBarWidth - 20) * 0.5f, buttonHeight * 0.5f));
        label->setColor(Color3B::WHITE);
        buttonBg->addChild(label);

        // Position button
        buttonBg->setPosition(Vec2(0, totalHeight));

        // Add touch listener for clicking
        auto touchListener = EventListenerTouchOneByOne::create();
        touchListener->setSwallowTouches(true);
        touchListener->onTouchBegan = [buttonBg, i](Touch* touch, Event* event) {
            Vec2 locationInNode = buttonBg->convertToNodeSpace(touch->getLocation());
            Size s = buttonBg->getContentSize();
            Rect rect(0, 0, s.width, s.height);

            if (rect.containsPoint(locationInNode)) {
                buttonBg->setColor(Color3B(120, 160, 200)); // Highlight
                return true;
            }
            return false;
        };
        touchListener->onTouchEnded = [this, buttonBg, i](Touch* touch, Event* event) {
            buttonBg->setColor(Color3B(60, 80, 120)); // Reset color
            onCategoryButtonClicked(nullptr, i);
        };
        _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, buttonBg);

        container->addChild(buttonBg);
        totalHeight += buttonHeight + buttonSpacing;
    }

    // Update inner container size
    container->setContentSize(Size(kBarWidth - 10, totalHeight));
    _categoryScrollView->setInnerContainerSize(container->getContentSize());

    CCLOG("CraftBar: Created %zu category buttons, total height: %.1f",
          _categoryButtons.size(), totalHeight);
}

void CraftBar::onCategoryButtonClicked(Ref* sender, int categoryIndex) {
    auto recipes = CraftingMatcher::getInstance()->getAvailableRecipes();

    if (categoryIndex >= 0 && categoryIndex < static_cast<int>(recipes.size())) {
        _selectedRecipe = recipes[categoryIndex];
        updateSelectedRecipeSlot();

        // Try to craft when clicking
        if (_selectedRecipe) {
            bool canCraft = CraftingMatcher::getInstance()->canCraft(*_selectedRecipe);
            if (canCraft) {
                bool success = CraftingExecutor::getInstance()->craft(*_selectedRecipe);
                CCLOG("CraftBar: Craft %s", success ? "SUCCESS" : "FAILED");
            } else {
                CCLOG("CraftBar: Cannot craft - missing materials or station");
            }
        }
    }
}

void CraftBar::updateSelectedRecipeSlot() {
    if (!_selectedRecipe) {
        _selectedNameLabel->setString("No Recipe");
        _selectedCountLabel->setString("");
        return;
    }

    auto itemDef = ItemManager::getInstance()->getItemData(_selectedRecipe->resultItemId);

    // Update name
    std::string name = itemDef ? itemDef->name : "Unknown";
    _selectedNameLabel->setString(name);

    // Update count and materials
    int maxCraft = CraftingMatcher::getInstance()->getMaxCraftCount(*_selectedRecipe);
    bool canCraft = CraftingMatcher::getInstance()->canCraft(*_selectedRecipe);

    std::string countText = StringUtils::format("x%d (Max: %d)",
        _selectedRecipe->resultCount, maxCraft);
    _selectedCountLabel->setString(countText);

    // Color based on availability
    if (canCraft) {
        _selectedNameLabel->setColor(Color3B(100, 255, 100));
    } else {
        _selectedNameLabel->setColor(Color3B(255, 150, 150));
    }

    // Update icon if possible
    if (itemDef && !itemDef->iconPath.empty()) {
        _selectedIcon->setTexture(itemDef->iconPath);
        _selectedIcon->setScale(0.8f);
    }
}

void CraftBar::updateScrollIndicator() {
    // Visual feedback for scroll position (optional)
    // Could show a small scrollbar on the right edge
}

void CraftBar::updateFadeGradients() {
    // Add fade gradients at top/bottom to indicate more content
    // This is optional visual polish
}

void CraftBar::refreshRecipes() {
    refreshCategoryButtons();

    // Update selected recipe slot
    if (_selectedRecipe) {
        updateSelectedRecipeSlot();
    }
}

void CraftBar::setupEventListeners() {
    auto dispatcher = Director::getInstance()->getEventDispatcher();

    auto invListener = EventListenerCustom::create("Event_InventoryChanged",
        CC_CALLBACK_1(CraftBar::onInventoryChanged, this));
    dispatcher->addEventListenerWithSceneGraphPriority(invListener, this);

    auto stationListener = EventListenerCustom::create("Event_StationChanged",
        CC_CALLBACK_1(CraftBar::onStationChanged, this));
    dispatcher->addEventListenerWithSceneGraphPriority(stationListener, this);
}

void CraftBar::onInventoryChanged(EventCustom* event) {
    refreshRecipes();
}

void CraftBar::onStationChanged(EventCustom* event) {
    refreshRecipes();
}

// Expanded panel methods (to be implemented in phase 2)
void CraftBar::setupExpandedUI() {
    // TODO: Implement expanded recipe list panel
}

void CraftBar::setupRecipeListView() {
    // TODO: Implement ListView for recipes with 70px row height
}

void CraftBar::refreshRecipeList() {
    // TODO: Refresh expanded recipe list
}

void CraftBar::onRecipeItemClicked(int recipeIndex) {
    // TODO: Handle recipe selection in expanded mode
}

void CraftBar::togglePanel() {
    // TODO: Toggle between collapsed and expanded
}

void CraftBar::expandPanel() {
    // TODO: Expand to show full recipe list
}

void CraftBar::collapsePanel() {
    // TODO: Collapse to category view
}
