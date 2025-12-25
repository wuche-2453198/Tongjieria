#ifndef __INVENTORY_LAYER_H__
#define __INVENTORY_LAYER_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "systems/items/Inventory.h"
#include "systems/items/ItemManager.h"

#include <vector>
#include <string>

// Forward declaration
class EquipmentPanel;

// Simple 10x5 inventory UI with mouse drag/drop and count/name display
//UI类的定义，包含鼠标拖拽，
class InventoryLayer : public cocos2d::Layer {
public:
    static InventoryLayer* create();
    virtual bool init() override;

    // Refresh all slot visuals from Inventory data
    void refresh();

    // Set equipment panel for cross-UI drag support
    void setEquipmentPanel(EquipmentPanel* panel);

protected:

    enum class SlotKind { Normal, Coin, Ammo, Trash, Weapon };

    // 
    int hitTestSlot(const cocos2d::Vec2& worldPos) const;
 
    cocos2d::Rect getSlotRect(int index) const;
    //
    cocos2d::Vec2 getSlotPos(int index) const;
    float getSlotSize(SlotKind kind) const;
    SlotKind getSlotKind(int index) const;
    cocos2d::Color3B getSlotColor(SlotKind kind) const;
    std::string getSlotLabel(SlotKind kind) const;
    void drawSlotPattern(cocos2d::DrawNode* node, SlotKind kind, float size);
    void updateTooltip(const std::string& text, const cocos2d::Vec2& worldPos);

    void beginDrag(int index, const cocos2d::Vec2& worldPos);
    void endDrag(int targetIndex);
    void updateDragSprite(const cocos2d::Vec2& worldPos);

    void onInventoryChanged(cocos2d::EventCustom* event);
    void updateHighlights();

private:
    int _cols = 10;
    int _rows = 5;
    int _slotCount = 0;
    float _slotSize = 64.0f;//
    float _slotPadding = 6.0f;
    float _specialGap = 6.0f;
    float _layoutHeight = 0.0f;
    cocos2d::Vec2 _originOffset;

    std::vector<cocos2d::Node*> _slotBg;
    std::vector<cocos2d::Sprite*> _slotIcons;
    std::vector<cocos2d::Label*> _slotCounts;
    std::vector<cocos2d::Label*> _slotTags;
    std::vector<cocos2d::DrawNode*> _slotOutline;
    std::vector<float> _slotSizes;

    cocos2d::Label* _nameLabel = nullptr;
    cocos2d::Node* _tooltipBg = nullptr;
    cocos2d::Label* _tooltipLabel = nullptr;
    int _hoverIndex = -1;
    int _selectedIndex = -1;

    // Drag state
    bool _dragging = false;
    int _dragSource = -1;
    cocos2d::Sprite* _dragSprite = nullptr;

    // Organization button
    cocos2d::ui::Button* _organizationButton = nullptr;

    // Reference to equipment panel for cross-UI dragging
    EquipmentPanel* _equipmentPanel = nullptr;

    void buildSlots();
    void attachMouseHandlers();
    void onOrganizationButtonClicked(cocos2d::Ref* sender);
    void sortInventory();

    // Helper to try equipping item to equipment panel
    bool tryEquipToPanel(int inventoryIndex, const cocos2d::Vec2& worldPos);
};

#endif // __INVENTORY_LAYER_H__
