#ifndef __CRAFT_BAR_H__
#define __CRAFT_BAR_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "CraftingDef.h"
#include <vector>

// Crafting bar UI with scrolling window support
// Bottom-left fixed position with collapsible category list
// Features: mouse wheel scroll, drag scroll, clipping, bounce effect
class CraftBar : public cocos2d::Layer {
public:
    static CraftBar* create();
    virtual bool init() override;

    void refreshRecipes();
    void onInventoryChanged(cocos2d::EventCustom* event);
    void onStationChanged(cocos2d::EventCustom* event);

private:
    // UI State
    bool _expanded = false;

    // Fixed UI elements
    cocos2d::Label* _titleLabel = nullptr;
    cocos2d::Node* _selectedRecipeSlot = nullptr;
    cocos2d::Sprite* _selectedIcon = nullptr;
    cocos2d::Label* _selectedNameLabel = nullptr;
    cocos2d::Label* _selectedCountLabel = nullptr;

    // Collapsed state: Category scroll view
    cocos2d::ui::ScrollView* _categoryScrollView = nullptr;
    std::vector<cocos2d::ui::Button*> _categoryButtons;

    // Expanded state: Recipe list panel
    cocos2d::Node* _recipeListPanel = nullptr;
    cocos2d::ui::ListView* _recipeListView = nullptr;
    std::vector<const RecipeDefinition*> _currentRecipes;
    const RecipeDefinition* _selectedRecipe = nullptr;

    // Visual elements
    cocos2d::DrawNode* _scrollIndicator = nullptr;
    cocos2d::Sprite* _fadeGradientTop = nullptr;
    cocos2d::Sprite* _fadeGradientBottom = nullptr;

    // Layout constants (defined in .cpp)
    static const float kBarWidth;
    static const float kCollapsedHeight;
    static const float kExpandedWidth;
    static const float kExpandedHeight;
    static const float kCategoryScrollHeight;
    static const float kRecipeScrollHeight;
    static const float kRecipeItemHeight;
    static const float kScrollWheelSpeed;

    // Setup methods
    void setupEventListeners();
    void setupCollapsedUI();
    void setupSelectedRecipeSlot();
    void setupCategoryScrollView();
    void setupExpandedUI();
    void setupRecipeListView();
    void setupMouseWheelListener();

    // Category management
    void onCategoryButtonClicked(cocos2d::Ref* sender, int categoryIndex);
    void refreshCategoryButtons();

    // Recipe list management
    void refreshRecipeList();
    void onRecipeItemClicked(int recipeIndex);
    void updateSelectedRecipeSlot();

    // Scroll visual feedback
    void updateScrollIndicator();
    void updateFadeGradients();

    // Toggle expanded/collapsed
    void togglePanel();
    void expandPanel();
    void collapsePanel();
};

#endif // __CRAFT_BAR_H__
