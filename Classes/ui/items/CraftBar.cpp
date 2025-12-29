#include "CraftBar.h"
#include "systems/items/RecipeManager.h"
#include "systems/items/StationDetector.h"
#include "systems/items/CraftingMatcher.h"
#include "systems/items/CraftingExecutor.h"
#include "systems/items/ItemManager.h"
#include "systems/items/Inventory.h"

using namespace cocos2d;
using namespace ui;

// Define static const members
const float CraftBar::kBarWidth = 200.0f;
const float CraftBar::kCollapsedHeight = 350.0f;
const float CraftBar::kExpandedWidth = 280.0f;
const float CraftBar::kExpandedHeight = 450.0f;
const float CraftBar::kCategoryScrollHeight = 280.0f;  // Increased from 240 to 280
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

    // Initial position at bottom-left corner with margin
    // This will be updated in update() to follow camera
    this->setPosition(Vec2(20, 70));  // Moved up 50 pixels (from 20 to 70)

    setupCollapsedUI();
    setupExpandCraftingButton();
    setupEventListeners();
    refreshRecipes();

    // CRITICAL: Enable update to keep CraftBar fixed on screen (same as InventoryLayer)
    this->scheduleUpdate();

    CCLOG("CraftBar: initialized with scrolling window support");
    return true;
}

void CraftBar::update(float dt) {
    // CRITICAL: Update position every frame to follow camera and stay fixed on screen
    // This is the EXACT same logic as InventoryLayer::update()
    auto scene = Director::getInstance()->getRunningScene();
    if (!scene) return;

    auto camera = scene->getDefaultCamera();
    if (!camera) return;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec3 camPos = camera->getPosition3D();

    // Calculate CraftBar position relative to camera
    // CraftBar is at bottom-left of screen with margin
    float marginX = 20.0f;
    float marginY = 70.0f;

    // Position = camera position + offset from camera center to screen bottom-left + margin
    float posX = camPos.x - (visibleSize.width / 2.0f) + marginX;
    float posY = camPos.y - (visibleSize.height / 2.0f) + marginY;

    setPosition(Vec2(posX, posY));
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

    // Category scroll view (removed selected recipe slot)
    setupCategoryScrollView();
}

void CraftBar::setupCategoryScrollView() {
    float scrollY = 50;  // Moved down from 130
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

    // Save current scroll position to restore after refresh
    Vec2 savedScrollPos = _categoryScrollView->getInnerContainerPosition();

    // Clear existing buttons
    auto container = _categoryScrollView->getInnerContainer();
    container->removeAllChildren();

    // Get available recipes
    auto recipes = CraftingMatcher::getInstance()->getAvailableRecipes();

    // Sort recipes: craftable first, uncraftable last
    std::vector<const RecipeDefinition*> sortedRecipes(recipes.begin(), recipes.end());
    std::stable_sort(sortedRecipes.begin(), sortedRecipes.end(),
        [](const RecipeDefinition* a, const RecipeDefinition* b) {
            bool canCraftA = CraftingMatcher::getInstance()->canCraft(*a);
            bool canCraftB = CraftingMatcher::getInstance()->canCraft(*b);
            return canCraftA > canCraftB;  // true (craftable) comes before false
        });

    // Layout constants
    float slotSize = 50.0f;        // Square size (slightly larger than inventory slots)
    float iconSize = 42.0f;        // Icon size within square
    float ingredientSlotSize = 40.0f;  // Smaller slots for ingredients
    float ingredientIconSize = 32.0f;  // Smaller icons for ingredients
    float rowSpacing = 5.0f;
    float totalHeight = 0;

    for (size_t i = 0; i < sortedRecipes.size(); ++i) {
        auto recipe = sortedRecipes[i];
        bool isExpanded = (_expandedRecipeIndex == static_cast<int>(i));
        bool isSelected = (_selectedRecipeIndex == static_cast<int>(i));
        bool canCraft = CraftingMatcher::getInstance()->canCraft(*recipe);

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

        // Choose background based on state:
        // - Cannot craft: dark_bottom.png
        // - Can craft + selected: light_bottom.png
        // - Can craft + not selected: Inventory.png
        std::string bgPath;
        if (!canCraft) {
            bgPath = "items/bottom/dark_bottom.png";
        } else if (isSelected) {
            bgPath = "items/bottom/light_bottom.png";
        } else {
            bgPath = "items/bottom/Inventory.png";
        }

        auto resultSlotBg = Sprite::create(bgPath);
        if (resultSlotBg) {
            float bgScale = (slotSize * 1.0f) / std::max(resultSlotBg->getContentSize().width,
                                                          resultSlotBg->getContentSize().height);
            resultSlotBg->setScale(bgScale);
            resultSlotBg->setAnchorPoint(Vec2(0, 0));
            resultSlotBg->setPosition(Vec2(0, 0));
            resultSlotBg->setTag(888);  // Tag for background update
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

    // Restore scroll position after refresh
    _categoryScrollView->setInnerContainerPosition(savedScrollPos);

    CCLOG("CraftBar: Created %zu recipe rows, expanded index: %d", sortedRecipes.size(), _expandedRecipeIndex);
}

void CraftBar::onCategoryButtonClicked(Ref* sender, int categoryIndex) {
    auto recipes = CraftingMatcher::getInstance()->getAvailableRecipes();

    // Sort recipes the same way as in refreshCategoryButtons
    std::vector<const RecipeDefinition*> sortedRecipes(recipes.begin(), recipes.end());
    std::stable_sort(sortedRecipes.begin(), sortedRecipes.end(),
        [](const RecipeDefinition* a, const RecipeDefinition* b) {
            bool canCraftA = CraftingMatcher::getInstance()->canCraft(*a);
            bool canCraftB = CraftingMatcher::getInstance()->canCraft(*b);
            return canCraftA > canCraftB;
        });

    if (categoryIndex >= 0 && categoryIndex < static_cast<int>(sortedRecipes.size())) {
        auto recipe = sortedRecipes[categoryIndex];
        bool canCraft = CraftingMatcher::getInstance()->canCraft(*recipe);

        // Cannot craft at all - do nothing
        if (!canCraft) {
            CCLOG("CraftBar: Cannot craft - missing materials or station");
            return;
        }

        // First click: Select the recipe (highlight it)
        if (_selectedRecipeIndex != categoryIndex) {
            _selectedRecipeIndex = categoryIndex;
            _selectedRecipe = recipe;
            CCLOG("CraftBar: Recipe %d selected (first click)", categoryIndex);
            refreshCategoryButtons();  // Refresh to update background
        }
        // Second click: Execute crafting
        else {
            bool success = CraftingExecutor::getInstance()->craft(*recipe);
            CCLOG("CraftBar: Craft %s (second click)", success ? "SUCCESS" : "FAILED");

            // Reset selection after crafting
            _selectedRecipeIndex = -1;
            _selectedRecipe = nullptr;
            // No need to refresh here as inventory change will trigger refresh
        }
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
    // Reset selection when recipes refresh (due to inventory/station changes)
    _selectedRecipeIndex = -1;
    _selectedRecipe = nullptr;
    _selectedQuickCraftIndex = -1;  // Also reset quick craft selection
    refreshCategoryButtons();
    // Refresh quick craft bar if it exists and is visible
    if (_quickCraftingBar && _quickCraftingBar->isVisible()) {
        refreshQuickCraftingBar();
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

    // Listen to PlayerCraftingSystem station detection events
    auto craftingStationListener = EventListenerCustom::create("Event_CraftingStationsChanged",
        CC_CALLBACK_1(CraftBar::onStationChanged, this));
    dispatcher->addEventListenerWithSceneGraphPriority(craftingStationListener, this);
}

void CraftBar::onInventoryChanged(EventCustom* event) {
    refreshRecipes();
}

void CraftBar::onStationChanged(EventCustom* event) {
    refreshRecipes();
}

// Setup expand crafting button (placed at the bottom of the scroll view)
void CraftBar::setupExpandCraftingButton() {
    float scrollY = 50;  // Match scroll view position
    float buttonY = scrollY;  // Align with bottom of scroll view
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

    // Show and refresh quick crafting bar
    if (_quickCraftingBar) {
        refreshQuickCraftingBar();
        _quickCraftingBar->setVisible(true);
        _quickCraftingBar->setOpacity(0);
        auto fadeIn = FadeIn::create(0.3f);
        _quickCraftingBar->runAction(fadeIn);
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

    // Hide quick crafting bar
    if (_quickCraftingBar) {
        auto fadeOut = FadeOut::create(0.2f);
        auto hideAction = Hide::create();
        auto sequence = Sequence::create(fadeOut, hideAction, nullptr);
        _quickCraftingBar->runAction(sequence);
    }
}

void CraftBar::createExpandedCraftingPanel() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // Panel size: narrower to avoid covering inventory and equipment panels
    float panelWidth = visibleSize.width * 0.3f;  // 30% of screen width (reduced from 40%)
    float panelHeight = visibleSize.height * 0.55f; // 55% of screen height (reduced from 60%)

    // Create main panel node
    _expandedCraftingPanel = Node::create();
    _expandedCraftingPanel->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
    _expandedCraftingPanel->setContentSize(Size(panelWidth, panelHeight));
    _expandedCraftingPanel->setPosition(Vec2(origin.x + visibleSize.width * 0.5f,
                                             origin.y + visibleSize.height * 0.5f));

    // Recipe grid view
    setupRecipeListView();

    _expandedCraftingPanel->setVisible(false);

    // Add to scene (parent of CraftBar)
    auto scene = Director::getInstance()->getRunningScene();
    if (scene) {
        scene->addChild(_expandedCraftingPanel, 100);
        CCLOG("CraftBar: Expanded crafting panel created, size=(%.1fx%.1f)", panelWidth, panelHeight);
    }

    // Create quick crafting bar
    createQuickCraftingBar();
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

// Quick crafting bar implementation
void CraftBar::createQuickCraftingBar() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // Create horizontal bar node
    _quickCraftingBar = Node::create();

    // Position at screen center, lowered by 70 pixels
    float barY = origin.y + visibleSize.height * 0.65f - 70.0f;
    _quickCraftingBar->setPosition(Vec2(origin.x + visibleSize.width * 0.5f, barY));

    _quickCraftingBar->setVisible(false);

    // Add to scene
    auto scene = Director::getInstance()->getRunningScene();
    if (scene) {
        scene->addChild(_quickCraftingBar, 101);  // Above expanded panel
        CCLOG("CraftBar: Quick crafting bar created at y=%.1f", barY);
    }
}

void CraftBar::refreshQuickCraftingBar() {
    if (!_quickCraftingBar) return;

    // Clear existing buttons
    _quickCraftingBar->removeAllChildren();
    _quickCraftButtons.clear();

    // Get all available recipes
    auto matcher = CraftingMatcher::getInstance();
    auto recipes = matcher->getAvailableRecipes();

    if (recipes.empty()) {
        CCLOG("CraftBar: No recipes available for quick crafting bar");
        return;
    }

    // Layout constants
    float iconSize = 52.0f;
    float iconPadding = 6.0f;
    int maxIcons = std::min((int)recipes.size(), 20);  // Limit to 20 icons

    // Calculate total width and starting position
    float totalWidth = maxIcons * (iconSize + iconPadding) - iconPadding;
    float startX = -totalWidth * 0.5f;

    // Create icon buttons for each recipe
    for (int i = 0; i < maxIcons; ++i) {
        auto recipe = recipes[i];
        auto itemMgr = ItemManager::getInstance();
        auto itemDef = itemMgr->getItemData(recipe->resultItemId);

        if (!itemDef) continue;

        bool canCraft = matcher->canCraft(*recipe);
        bool isSelected = (_selectedQuickCraftIndex == i);

        // Create button
        auto button = ui::Button::create();
        button->setScale9Enabled(true);
        button->setContentSize(Size(iconSize, iconSize));

        // Set background based on state:
        // - Cannot craft: dark_bottom.png
        // - Can craft + selected: light_bottom.png
        // - Can craft + not selected: Inventory.png
        std::string bgPath;
        if (!canCraft) {
            bgPath = "items/bottom/dark_bottom.png";
        } else if (isSelected) {
            bgPath = "items/bottom/light_bottom.png";
        } else {
            bgPath = "items/bottom/Inventory.png";
        }
        button->loadTextureNormal(bgPath);

        // Add item icon
        auto itemIcon = itemMgr->getItemSprite(recipe->resultItemId);
        if (itemIcon) {
            auto iconSprite = Sprite::createWithSpriteFrame(itemIcon);
            iconSprite->setPosition(Vec2(iconSize * 0.5f, iconSize * 0.5f));
            iconSprite->setScale(iconSize * 0.7f / iconSprite->getContentSize().width);
            iconSprite->setTag(999);  // Tag for reference
            button->addChild(iconSprite, 1);
        }

        // Position
        float posX = startX + i * (iconSize + iconPadding) + iconSize * 0.5f;
        button->setPosition(Vec2(posX, 0));

        // Click handler
        button->addClickEventListener([this, i](Ref* sender) {
            onQuickCraftButtonClicked(i);
        });

        _quickCraftingBar->addChild(button);
        _quickCraftButtons.push_back(button);
    }

    CCLOG("CraftBar: Quick crafting bar refreshed with %d icons", maxIcons);
}

void CraftBar::onQuickCraftButtonClicked(int recipeIndex) {
    auto matcher = CraftingMatcher::getInstance();
    auto recipes = matcher->getAvailableRecipes();

    if (recipeIndex < 0 || recipeIndex >= (int)recipes.size()) {
        CCLOG("CraftBar: Invalid recipe index %d", recipeIndex);
        return;
    }

    auto recipe = recipes[recipeIndex];

    // Check if can craft
    bool canCraft = matcher->canCraft(*recipe);
    if (!canCraft) {
        CCLOG("CraftBar: Cannot craft recipe %d - missing materials or station", recipeIndex);
        return;
    }

    // First click: Select the recipe (highlight it)
    if (_selectedQuickCraftIndex != recipeIndex) {
        _selectedQuickCraftIndex = recipeIndex;
        CCLOG("CraftBar: Quick craft recipe %d selected (first click)", recipeIndex);
        refreshQuickCraftingBar();  // Refresh to update background
    }
    // Second click: Execute crafting
    else {
        auto executor = CraftingExecutor::getInstance();
        bool success = executor->craft(*recipe);

        if (success) {
            CCLOG("CraftBar: Quick craft successful for recipe %d (second click)", recipeIndex);
            // Reset selection after crafting
            _selectedQuickCraftIndex = -1;
            // No need to refresh here as inventory change will trigger refresh
        } else {
            CCLOG("CraftBar: Quick craft failed for recipe %d", recipeIndex);
        }
    }
}

// Expanded panel methods (to be implemented in phase 2)
void CraftBar::setupExpandedUI() {
    // Already implemented above
}
