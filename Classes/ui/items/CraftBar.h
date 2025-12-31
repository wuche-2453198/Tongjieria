#ifndef __CRAFT_BAR_H__
#define __CRAFT_BAR_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "components/item/CraftingDef.h"
#include <vector>

// 带滚动窗口支持的合成栏UI
// 左下角固定位置，带可折叠的分类列表
// 功能：鼠标滚轮滚动、拖拽滚动、裁剪、回弹效果
// 负责合成界面的UI合成
class CraftBar : public cocos2d::Layer {
public:
    static CraftBar* create();
    virtual bool init() override;
    virtual void update(float dt) override;

    void refreshRecipes();
    void onInventoryChanged(cocos2d::EventCustom* event);
    void onStationChanged(cocos2d::EventCustom* event);

private:
    // UI状态
    bool _expanded = false;

    // 固定UI元素
    cocos2d::Label* _titleLabel = nullptr;

    // 折叠状态：分类滚动视图
    cocos2d::ui::ScrollView* _categoryScrollView = nullptr;
    std::vector<cocos2d::ui::Button*> _categoryButtons;

    // 展开状态：配方列表面板
    cocos2d::Node* _recipeListPanel = nullptr;
    cocos2d::ui::ListView* _recipeListView = nullptr;
    std::vector<const RecipeDefinition*> _currentRecipes;
    const RecipeDefinition* _selectedRecipe = nullptr;
    int _expandedRecipeIndex = -1;  // 跟踪哪个配方被展开（-1 = 无）
    int _selectedRecipeIndex = -1;  // 跟踪哪个配方被选中用于合成（-1 = 无）

    // 视觉元素
    cocos2d::DrawNode* _scrollIndicator = nullptr;
    cocos2d::Sprite* _fadeGradientTop = nullptr;
    cocos2d::Sprite* _fadeGradientBottom = nullptr;

    // 展开合成按钮和面板
    cocos2d::ui::Button* _expandCraftingButton = nullptr;
    cocos2d::Node* _expandedCraftingPanel = nullptr;
    bool _craftingPanelExpanded = false;

    // 快速合成栏（屏幕中心的水平图标栏）
    cocos2d::Node* _quickCraftingBar = nullptr;
    std::vector<cocos2d::ui::Button*> _quickCraftButtons;
    int _selectedQuickCraftIndex = -1;  // 跟踪哪个快速合成按钮被选中（-1 = 无）

    // 布局常量（在.cpp中定义）
    static const float kBarWidth;
    static const float kCollapsedHeight;
    static const float kExpandedWidth;
    static const float kExpandedHeight;
    static const float kCategoryScrollHeight;
    static const float kRecipeScrollHeight;
    static const float kRecipeItemHeight;
    static const float kScrollWheelSpeed;

    // 设置方法
    void setupEventListeners();
    void setupCollapsedUI();
    void setupCategoryScrollView();
    void setupExpandedUI();
    void setupRecipeListView();
    void setupMouseWheelListener();

    // 分类管理
    void onCategoryButtonClicked(cocos2d::Ref* sender, int categoryIndex);
    void refreshCategoryButtons();

    // 配方列表管理
    void refreshRecipeList();
    void onRecipeItemClicked(int recipeIndex);

    // 滚动视觉反馈
    void updateScrollIndicator();
    void updateFadeGradients();

    // 切换展开/折叠
    void togglePanel();
    void expandPanel();
    void collapsePanel();

    // 展开合成按钮处理器
    void onExpandCraftingButtonClicked(cocos2d::Ref* sender);
    void setupExpandCraftingButton();
    void createExpandedCraftingPanel();

    // 快速合成栏方法
    void createQuickCraftingBar();
    void refreshQuickCraftingBar();
    void onQuickCraftButtonClicked(int recipeIndex);
};

#endif // __CRAFT_BAR_H__
