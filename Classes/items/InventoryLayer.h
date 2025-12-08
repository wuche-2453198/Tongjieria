#ifndef __INVENTORY_LAYER_H__
#define __INVENTORY_LAYER_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "items/Inventory.h"
#include "items/ItemManager.h"

#include <vector>

// Simple 10x5 inventory UI with mouse drag/drop and count/name display
class InventoryLayer : public cocos2d::Layer {
public:
    static InventoryLayer* create();
    virtual bool init() override;

    // Refresh all slot visuals from Inventory data
    void refresh();

protected:
    // Slot index helpers
    int hitTestSlot(const cocos2d::Vec2& worldPos) const;
    cocos2d::Rect getSlotRect(int index) const;

    void beginDrag(int index, const cocos2d::Vec2& worldPos);
    void endDrag(int targetIndex);
    void updateDragSprite(const cocos2d::Vec2& worldPos);

    void onInventoryChanged(cocos2d::EventCustom* event);

private:
    int _cols = 10;
    int _rows = 5;
    float _slotSize = 64.0f;
    float _slotPadding = 6.0f;
    cocos2d::Vec2 _originOffset;

    std::vector<cocos2d::Node*> _slotBg;
    std::vector<cocos2d::Sprite*> _slotIcons;
    std::vector<cocos2d::Label*> _slotCounts;

    cocos2d::Label* _nameLabel = nullptr;

    // Drag state
    bool _dragging = false;
    int _dragSource = -1;
    cocos2d::Sprite* _dragSprite = nullptr;

    void buildSlots();
    void attachMouseHandlers();
};

#endif // __INVENTORY_LAYER_H__
