#ifndef __EQUIPMENT_PANEL_H__
#define __EQUIPMENT_PANEL_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "systems/items/Inventory.h"
#include "systems/items/ItemManager.h"
#include "systems/player/PlayerInventoryIntegration.h"

#include <vector>
#include <string>

// Equipment panel UI for displaying armor and accessory slots
class EquipmentPanel : public cocos2d::Layer {
public:
    static EquipmentPanel* create();
    virtual bool init() override;

    // Refresh all equipment slots from player equipment data
    void refresh();

protected:
    enum class EquipSlotType {
        Helmet,       // 头盔
        Chestplate,   // 胸甲
        Leggings,     // 护腿
        Accessory0,   // 饰品槽 1
        Accessory1,   // 饰品槽 2
        Accessory2,   // 饰品槽 3
        Accessory3    // 饰品槽 4
    };

    static const int EQUIPMENT_SLOT_COUNT = 7;  // 3 armor + 4 accessories

    // Slot helpers
    int hitTestEquipSlot(const cocos2d::Vec2& worldPos) const;
    cocos2d::Rect getEquipSlotRect(int index) const;
    cocos2d::Vec2 getEquipSlotPos(int index) const;
    EquipSlotType getEquipSlotType(int index) const;
    std::string getEquipSlotTexture(EquipSlotType type) const;

    void updateTooltip(const std::string& text, const cocos2d::Vec2& worldPos);
    void updateHighlights();

    void beginDrag(int index, const cocos2d::Vec2& worldPos);
    void endDrag(int targetIndex);
    void updateDragSprite(const cocos2d::Vec2& worldPos);

private:
    float _slotSize = 52.0f;
    float _slotPadding = 8.0f;
    cocos2d::Vec2 _originOffset;

    std::vector<cocos2d::Sprite*> _slotBg;
    std::vector<cocos2d::Sprite*> _slotIcons;
    std::vector<cocos2d::DrawNode*> _slotOutline;

    cocos2d::Label* _nameLabel = nullptr;
    cocos2d::Node* _tooltipBg = nullptr;
    cocos2d::Label* _tooltipLabel = nullptr;
    int _hoverIndex = -1;
    int _selectedIndex = -1;

    // Drag state
    bool _dragging = false;
    int _dragSource = -1;
    cocos2d::Sprite* _dragSprite = nullptr;

    void buildSlots();
    void attachMouseHandlers();
};

#endif // __EQUIPMENT_PANEL_H__
