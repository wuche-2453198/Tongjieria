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
    setupExpandCraftingButton();
    setupEventListeners();
    refreshRecipes();

    CCLOG("CraftBar: initialized with scrolling window support");
    return true;
}

void CraftBar::setupCollapsedUI() {
    // Main background for collapsed state (hidden)
    auto bg = LayerColor::create(Color4B(20, 30, 50, 240), kBarWidth, kCollapsedHeight);
    bg->setPosition(Vec2::ZERO);
    bg->setVisible(false); // Hide the background canvas
    this->addChild(bg, -1);

    // Border removed for cleaner look
    // (No border to match the frameless design)

    // Title at top (hidden)
    _titleLabel = Label::createWithSystemFont("Crafting", "Arial", 18);
    _titleLabel->setPosition(Vec2(kBarWidth * 0.5f, kCollapsedHeight - 25));
    _titleLabel->setColor(Color3B(255, 220, 100));
    _titleLabel->setVisible(false); // Hide the title text
    this->addChild(_titleLabel, 1);

    // Selected recipe slot at bottom (hidden)
    setupSelectedRecipeSlot();
    if (_selectedRecipeSlot) {
        _selectedRecipeSlot->setVisible(false); // Hide the selected recipe display
    }

    // Category scroll view in the middle
    setupCategoryScrollView();
}

void CraftBar::setupSelectedRecipeSlot() {
    float slotY = 60;
    float slotHeight = 80;

    // Background for selected slot (hidden)
    auto slotBg = LayerColor::create(Color4B(40, 50, 70, 200), kBarWidth - 10, slotHeight);
    slotBg->setPosition(Vec2(5, slotY));
    slotBg->setVisible(false); // Hide the slot background
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

    // Get available recipes
    auto recipes = CraftingMatcher::getInstance()->getAvailableRecipes();

    // Layout constants
    float slotSize = 50.0f;        // Square size (slightly larger than inventory slots)
    float iconSize = 42.0f;        // Icon size within square
    float ingredientSlotSize = 40.0f;  // Smaller slots for ingredients
    float ingredientIconSize = 32.0f;  // Smaller icons for ingredients
    float rowSpacing = 5.0f;
    float totalHeight = 0;

    for (size_t i = 0; i < recipes.size(); ++i) {
        auto recipe = recipes[i];
        bool isExpanded = (_expandedRecipeIndex == static_cast<int>(i));

        // Create row container
        auto rowNode = Node::create();
        float rowHeight = slotSize;
        float rowWidth = kBarWidth - 20;

        // Calculate expanded width if needed
        if (isExpanded) {
            // Width = result square + arrow + ingredients (with slots)
            rowWidth = slotSize + 30.0f + (recipe->ingredients.size() * (ingredientSlotSize + 8.0f));
            rowWidth = std::min(rowWidth, kBarWidth * 2.5f); // Cap maximum width
        }

        rowNode->setContentSize(Size(rowWidth, rowHeight));

        // Add item slot background for result item using Inventory.png
        auto resultSlotBg = Sprite::create("items/bottom/Inventory.png");
        if (resultSlotBg) {
            float bgScale = (slotSize * 1.0f) / std::max(resultSlotBg->getContentSize().width,
                                                          resultSlotBg->getContentSize().height);
            resultSlotBg->setScale(bgScale);
            resultSlotBg->setAnchorPoint(Vec2(0, 0));
            resultSlotBg->setPosition(Vec2(0, 0));
            rowNode->addChild(resultSlotBg, -1);
        }

        // Add result item icon
        auto resultIcon = ItemManager::getInstance()->getItemSprite(recipe->resultItemId);
        if (resultIcon) {
            auto sprite = Sprite::createWithSpriteFrame(resultIcon);
            if (sprite) {
                float scale = iconSize / std::max(sprite->getContentSize().width,
                                                  sprite->getContentSize().height);
                sprite->setScale(scale);
                sprite->setPosition(Vec2(slotSize * 0.5f, slotSize * 0.5f));
                sprite->setTag(999); // For highlight
                rowNode->addChild(sprite, 1);
            }
        }

        // If expanded, show ingredients horizontally
        if (isExpanded) {
            float xOffset = slotSize + 15.0f;

            // Add arrow symbol
            auto arrow = Label::createWithSystemFont("<-", "Arial", 16);
            arrow->setPosition(Vec2(xOffset, slotSize * 0.5f));
            arrow->setColor(Color3B(200, 200, 200));
            rowNode->addChild(arrow, 1);
            xOffset += 25.0f;

            // Add ingredient icons with slots
            for (const auto& ingredient : recipe->ingredients) {
                // Add item slot background for ingredient using Inventory.png
                float slotY = (slotSize - ingredientSlotSize) * 0.5f;  // Center vertically
                auto ingSlotBg = Sprite::create("items/bottom/Inventory.png");
                if (ingSlotBg) {
                    float ingBgScale = (ingredientSlotSize * 1.0f) / std::max(ingSlotBg->getContentSize().width,
                                                                               ingSlotBg->getContentSize().height);
                    ingSlotBg->setScale(ingBgScale);
                    ingSlotBg->setAnchorPoint(Vec2(0, 0));
                    ingSlotBg->setPosition(Vec2(xOffset, slotY));
                    rowNode->addChild(ingSlotBg, -1);
                }

                auto ingIcon = ItemManager::getInstance()->getItemSprite(ingredient.itemId);
                if (ingIcon) {
                    auto ingSprite = Sprite::createWithSpriteFrame(ingIcon);
                    if (ingSprite) {
                        float ingScale = ingredientIconSize / std::max(ingSprite->getContentSize().width,
                                                                       ingSprite->getContentSize().height);
                        ingSprite->setScale(ingScale);
                        ingSprite->setPosition(Vec2(xOffset + ingredientSlotSize * 0.5f,
                                                   slotY + ingredientSlotSize * 0.5f));
                        rowNode->addChild(ingSprite, 1);

                        // Add count label at bottom-right of slot
                        auto countLabel = Label::createWithSystemFont(
                            StringUtils::format("x%d", ingredient.count),
                            "Arial", 10);
                        countLabel->setAnchorPoint(Vec2(1.0f, 0.0f));  // Bottom-right anchor
                        countLabel->setPosition(Vec2(xOffset + ingredientSlotSize - 2.0f,
                                                    slotY + 2.0f));
                        countLabel->setColor(Color3B(255, 255, 100));
                        rowNode->addChild(countLabel, 2);
                    }
                }

                xOffset += ingredientSlotSize + 8.0f;
            }
        }

        // Position row
        rowNode->setPosition(Vec2(0, totalHeight));

        // Add touch listener
        auto touchListener = EventListenerTouchOneByOne::create();
        touchListener->setSwallowTouches(true);
        touchListener->onTouchBegan = [rowNode](Touch* touch, Event* event) {
            Vec2 locationInNode = rowNode->convertToNodeSpace(touch->getLocation());
            Size s = rowNode->getContentSize();
            Rect rect(0, 0, s.width, s.height);

            if (rect.containsPoint(locationInNode)) {
                auto sprite = rowNode->getChildByTag(999);
                if (sprite) {
                    sprite->setColor(Color3B(180, 180, 180));
                }
                return true;
            }
            return false;
        };
        touchListener->onTouchEnded = [this, rowNode, i](Touch* touch, Event* event) {
            auto sprite = rowNode->getChildByTag(999);
            if (sprite) {
                sprite->setColor(Color3B::WHITE);
            }
            // Toggle expansion
            if (_expandedRecipeIndex == static_cast<int>(i)) {
                _expandedRecipeIndex = -1;  // Collapse
            } else {
                _expandedRecipeIndex = i;   // Expand this one
            }
            refreshCategoryButtons();  // Refresh to show expansion
            onCategoryButtonClicked(nullptr, i);
        };
        _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, rowNode);

        container->addChild(rowNode);
        totalHeight += rowHeight + rowSpacing;
    }

    // Update inner container size
    container->setContentSize(Size(kBarWidth - 10, totalHeight));
    _categoryScrollView->setInnerContainerSize(container->getContentSize());

    CCLOG("CraftBar: Created %zu recipe rows, expanded index: %d", recipes.size(), _expandedRecipeIndex);
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
    auto itemIcon = ItemManager::getInstance()->getItemSprite(_selectedRecipe->resultItemId);
    if (itemIcon) {
        _selectedIcon->setSpriteFrame(itemIcon);
        _selectedIcon->setVisible(true);
        // Scale to fit slot (assuming slot icon area is around 40x40)
        float iconSize = 40.0f;
        float scale = iconSize / std::max(_selectedIcon->getContentSize().width,
                                         _selectedIcon->getContentSize().height);
        _selectedIcon->setScale(scale);
    } else {
        _selectedIcon->setVisible(false);
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

// Setup expand crafting button (placed below the category scroll view)
void CraftBar::setupExpandCraftingButton() {
    float buttonY = 140.0f - 60.0f;  // Below the category scroll view
    float buttonSize = 50.0f;

    _expandCraftingButton = ui::Button::create("items/bottom/crafting.png",
                                                "items/bottom/crafting.png",
                                                "items/bottom/crafting.png");
    if (_expandCraftingButton) {
        _expandCraftingButton->setPosition(Vec2(kBarWidth * 0.5f, buttonY));
        _expandCraftingButton->setScale(buttonSize / _expandCraftingButton->getContentSize().width);
        _expandCraftingButton->addClickEventListener(CC_CALLBACK_1(CraftBar::onExpandCraftingButtonClicked, this));
        this->addChild(_expandCraftingButton, 10);
        CCLOG("CraftBar: Expand crafting button created at Y=%.1f", buttonY);
    } else {
        CCLOG("Warning: Failed to create expand crafting button");
    }
}

void CraftBar::onExpandCraftingButtonClicked(Ref* sender) {
    CCLOG("CraftBar: Expand crafting button clicked");
    togglePanel();
}

void CraftBar::togglePanel() {
    if (_craftingPanelExpanded) {
        collapsePanel();
    } else {
        expandPanel();
    }
}

void CraftBar::expandPanel() {
    CCLOG("CraftBar: Expanding crafting panel");
    _craftingPanelExpanded = true;

    if (!_expandedCraftingPanel) {
        createExpandedCraftingPanel();
    }

    if (_expandedCraftingPanel) {
        _expandedCraftingPanel->setVisible(true);
        // Animate expansion from left to center
        _expandedCraftingPanel->setScale(0.0f);
        auto scaleAction = ScaleTo::create(0.3f, 1.0f);
        auto easeAction = EaseBackOut::create(scaleAction);
        _expandedCraftingPanel->runAction(easeAction);
    }
}

void CraftBar::collapsePanel() {
    CCLOG("CraftBar: Collapsing crafting panel");
    _craftingPanelExpanded = false;

    if (_expandedCraftingPanel) {
        // Animate collapse
        auto scaleAction = ScaleTo::create(0.2f, 0.0f);
        auto easeAction = EaseBackIn::create(scaleAction);
        auto hideAction = Hide::create();
        auto sequence = Sequence::create(easeAction, hideAction, nullptr);
        _expandedCraftingPanel->runAction(sequence);
    }
}

void CraftBar::createExpandedCraftingPanel() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // Panel size: horizontal layout in center of screen
    float panelWidth = visibleSize.width * 0.6f;  // 60% of screen width
    float panelHeight = visibleSize.height * 0.7f; // 70% of screen height

    // Create main panel node
    _expandedCraftingPanel = Node::create();
    _expandedCraftingPanel->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
    _expandedCraftingPanel->setContentSize(Size(panelWidth, panelHeight));
    _expandedCraftingPanel->setPosition(Vec2(origin.x + visibleSize.width * 0.5f,
                                             origin.y + visibleSize.height * 0.5f));

    // Semi-transparent background
    auto bg = LayerColor::create(Color4B(20, 30, 50, 230), panelWidth, panelHeight);
    bg->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
    bg->setPosition(Vec2(panelWidth * 0.5f, panelHeight * 0.5f));
    _expandedCraftingPanel->addChild(bg, -1);

    // Border
    auto border = DrawNode::create();
    Color4F borderColor(0.85f, 0.74f, 0.40f, 0.9f);
    Vec2 rect[4] = {
        Vec2(2, 2),
        Vec2(panelWidth - 2, 2),
        Vec2(panelWidth - 2, panelHeight - 2),
        Vec2(2, panelHeight - 2)
    };
    border->drawPoly(rect, 4, true, borderColor);
    _expandedCraftingPanel->addChild(border, 0);

    // Title
    auto titleLabel = Label::createWithSystemFont("All Recipes", "Arial", 24);
    titleLabel->setPosition(Vec2(panelWidth * 0.5f, panelHeight - 30));
    titleLabel->setColor(Color3B(255, 220, 100));
    _expandedCraftingPanel->addChild(titleLabel, 1);

    // Close button (X)
    auto closeButton = ui::Button::create();
    closeButton->setTitleText("X");
    closeButton->setTitleFontSize(24);
    closeButton->setTitleColor(Color3B::WHITE);
    closeButton->setPosition(Vec2(panelWidth - 30, panelHeight - 30));
    closeButton->addClickEventListener([this](Ref* sender) {
        collapsePanel();
    });
    _expandedCraftingPanel->addChild(closeButton, 2);

    // Recipe grid view
    setupRecipeListView();

    _expandedCraftingPanel->setVisible(false);

    // Add to scene (parent of CraftBar)
    auto scene = Director::getInstance()->getRunningScene();
    if (scene) {
        scene->addChild(_expandedCraftingPanel, 100);
        CCLOG("CraftBar: Expanded crafting panel created, size=(%.1fx%.1f)", panelWidth, panelHeight);
    }
}

void CraftBar::setupRecipeListView() {
    if (!_expandedCraftingPanel) return;

    auto panelSize = _expandedCraftingPanel->getContentSize();
    float listWidth = panelSize.width - 40;
    float listHeight = panelSize.height - 100;

    // Create scroll view for recipes
    auto recipeScrollView = ui::ScrollView::create();
    recipeScrollView->setContentSize(Size(listWidth, listHeight));
    recipeScrollView->setPosition(Vec2(20, 20));
    recipeScrollView->setDirection(ui::ScrollView::Direction::VERTICAL);
    recipeScrollView->setBounceEnabled(true);
    recipeScrollView->setScrollBarEnabled(true);
    recipeScrollView->setClippingEnabled(true);
    _expandedCraftingPanel->addChild(recipeScrollView, 1);

    refreshRecipeList();
}

void CraftBar::refreshRecipeList() {
    // This will be populated with all available recipes
    // For now, we'll reuse the category buttons logic but in a grid layout
    CCLOG("CraftBar: Refreshing expanded recipe list");
}

void CraftBar::onRecipeItemClicked(int recipeIndex) {
    // Handle recipe selection in expanded mode
    CCLOG("CraftBar: Recipe item %d clicked in expanded mode", recipeIndex);
}

// Expanded panel methods (to be implemented in phase 2)
void CraftBar::setupExpandedUI() {
    // Already implemented above
}
