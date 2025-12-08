#include "InventoryLayer.h"

using namespace cocos2d;
using namespace ui;

InventoryLayer* InventoryLayer::create() {
    auto ret = new (std::nothrow) InventoryLayer();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool InventoryLayer::init() {
    if (!Layer::init()) return false;

    // Anchor layer center; compute origin offset so grid is centered
    float width = _cols * _slotSize + (_cols - 1) * _slotPadding;
    float height = _rows * _slotSize + (_rows - 1) * _slotPadding;
    _originOffset = Vec2(-width * 0.5f, -height * 0.5f);

    buildSlots();
    refresh();
    attachMouseHandlers();

    // Listen to inventory change events
    auto listener = EventListenerCustom::create("Event_InventoryChanged", CC_CALLBACK_1(InventoryLayer::onInventoryChanged, this));
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    // Name label shown near bottom
    _nameLabel = Label::createWithSystemFont("", "Arial", 16);
    _nameLabel->setAnchorPoint(Vec2(0.5f, 1.0f));
    _nameLabel->setPosition(Vec2(0, height * 0.5f + 20));
    this->addChild(_nameLabel, 5);

    return true;
}

void InventoryLayer::buildSlots() {
    _slotBg.resize(_cols * _rows);
    _slotIcons.resize(_cols * _rows);
    _slotCounts.resize(_cols * _rows);

    for (int r = 0; r < _rows; ++r) {
        for (int c = 0; c < _cols; ++c) {
            int idx = r * _cols + c;
            Vec2 pos(_originOffset.x + c * (_slotSize + _slotPadding) + _slotSize * 0.5f,
                     _originOffset.y + (_rows - 1 - r) * (_slotSize + _slotPadding) + _slotSize * 0.5f);

            auto bg = Sprite::create();
            bg->setTextureRect(Rect(0, 0, _slotSize, _slotSize));
            bg->setColor(Color3B(80, 80, 80));
            bg->setOpacity(120);
            bg->setPosition(pos);
            this->addChild(bg, 0);
            _slotBg[idx] = bg;

            auto icon = Sprite::create();
            icon->setPosition(pos);
            this->addChild(icon, 1);
            _slotIcons[idx] = icon;

            auto countLabel = Label::createWithSystemFont("", "Arial", 18);
            countLabel->setAnchorPoint(Vec2(1.0f, 0.0f));
            countLabel->setPosition(pos + Vec2(_slotSize * 0.5f - 4, -_slotSize * 0.5f + 2));
            this->addChild(countLabel, 2);
            _slotCounts[idx] = countLabel;
        }
    }
}

void InventoryLayer::refresh() {
    const auto& slots = Inventory::getInstance()->getSlots();
    for (size_t i = 0; i < slots.size() && i < _slotIcons.size(); ++i) {
        const auto& slot = slots[i];
        auto icon = _slotIcons[i];
        auto countLabel = _slotCounts[i];

        if (slot.itemId == 0 || slot.count == 0) {
            icon->setVisible(false);
            countLabel->setString("");
            continue;
        }

        auto frame = ItemManager::getInstance()->getItemSprite(slot.itemId);
        if (frame) {
            icon->setSpriteFrame(frame);
            icon->setVisible(true);
            icon->setScale(_slotSize / icon->getContentSize().width * 0.8f);
        } else {
            icon->setVisible(false);
        }

        if (slot.count > 1) {
            countLabel->setString(StringUtils::format("%d", slot.count));
        } else {
            countLabel->setString("");
        }
    }
}

void InventoryLayer::attachMouseHandlers() {
    auto mouseListener = EventListenerMouse::create();
    mouseListener->onMouseDown = [this](EventMouse* event) {
        Vec2 pos = event->getLocation();
        int idx = hitTestSlot(pos);
        if (idx < 0) return;
        if (event->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {
            beginDrag(idx, pos);
        } else if (event->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT) {
            // Split half to first empty slot
            int target = Inventory::getInstance()->getFirstEmptySlot();
            if (target >= 0 && target != idx) {
                const auto& slots = Inventory::getInstance()->getSlots();
                int half = slots[idx].count / 2;
                if (half > 0) {
                    Inventory::getInstance()->moveItem(idx, target, half);
                }
            }
        }
    };

    mouseListener->onMouseUp = [this](EventMouse* event) {
        if (!_dragging) return;
        Vec2 pos = event->getLocation();
        int target = hitTestSlot(pos);
        endDrag(target);
    };

    mouseListener->onMouseMove = [this](EventMouse* event) {
        if (_dragging) {
            updateDragSprite(event->getLocation());
        }
        int idx = hitTestSlot(event->getLocation());
        if (idx >= 0) {
            const auto& slots = Inventory::getInstance()->getSlots();
            if (idx < (int)slots.size() && slots[idx].itemId != 0) {
                auto def = ItemManager::getInstance()->getItemData(slots[idx].itemId);
                _nameLabel->setString(def ? def->name : "");
                return;
            }
        }
        _nameLabel->setString("");
    };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);
}

int InventoryLayer::hitTestSlot(const Vec2& worldPos) const {
    for (int i = 0; i < _cols * _rows; ++i) {
        if (getSlotRect(i).containsPoint(worldPos)) {
            return i;
        }
    }
    return -1;
}

Rect InventoryLayer::getSlotRect(int index) const {
    int r = index / _cols;
    int c = index % _cols;
    Vec2 pos(_originOffset.x + c * (_slotSize + _slotPadding) + _slotSize * 0.5f,
             _originOffset.y + (_rows - 1 - r) * (_slotSize + _slotPadding) + _slotSize * 0.5f);
    Vec2 world = this->convertToWorldSpace(pos);
    float half = _slotSize * 0.5f;
    return Rect(world.x - half, world.y - half, _slotSize, _slotSize);
}

void InventoryLayer::beginDrag(int index, const Vec2& worldPos) {
    const auto& slots = Inventory::getInstance()->getSlots();
    if (index < 0 || index >= (int)slots.size()) return;
    if (slots[index].itemId == 0) return;
    _dragging = true;
    _dragSource = index;

    if (!_dragSprite) {
        _dragSprite = Sprite::create();
        this->addChild(_dragSprite, 10);
    }
    auto frame = ItemManager::getInstance()->getItemSprite(slots[index].itemId);
    _dragSprite->setSpriteFrame(frame);
    _dragSprite->setVisible(true);
    _dragSprite->setScale(_slotSize / _dragSprite->getContentSize().width * 0.8f);
    updateDragSprite(worldPos);
}

void InventoryLayer::endDrag(int targetIndex) {
    if (!_dragging) return;
    _dragging = false;
    if (_dragSprite) _dragSprite->setVisible(false);

    if (targetIndex < 0) return;
    if (targetIndex == _dragSource) return;

    auto inv = Inventory::getInstance();
    const auto& slots = inv->getSlots();
    if (_dragSource < (int)slots.size()) {
        int count = slots[_dragSource].count;
        inv->moveItem(_dragSource, targetIndex, count);
    }
    _dragSource = -1;
}

void InventoryLayer::updateDragSprite(const Vec2& worldPos) {
    if (_dragSprite) {
        Vec2 local = this->convertToNodeSpace(worldPos);
        _dragSprite->setPosition(local);
    }
}

void InventoryLayer::onInventoryChanged(EventCustom* event) {
    refresh();
}
