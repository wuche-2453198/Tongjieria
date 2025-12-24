#include "EquipmentPanel.h"
#include <algorithm>
#include <cmath>

using namespace cocos2d;
using namespace ui;

EquipmentPanel* EquipmentPanel::create() {
    auto ret = new (std::nothrow) EquipmentPanel();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool EquipmentPanel::init() {
    if (!Layer::init()) return false;

    auto director = Director::getInstance();
    const auto visibleSize = director->getVisibleSize();
    const auto origin = director->getVisibleOrigin();

    // Panel positioned on the right side of the screen, aligned with CraftBar
    float panelWidth = 70.0f;  // Narrow panel for equipment slots
    float panelHeight = 450.0f;  // Match CraftBar height

    setIgnoreAnchorPointForPosition(false);
    setAnchorPoint(Vec2::ANCHOR_BOTTOM_RIGHT);
    setContentSize(Size(panelWidth, panelHeight));
    // Position aligned with CraftBar (bottom at y=70, same as CraftBar)
    setPosition(Vec2(origin.x + visibleSize.width, origin.y+70.0f ));

    // Background panel: semi-transparent dark without border
    auto panelBg = LayerColor::create(Color4B(18, 22, 32, 180));
    panelBg->setContentSize(Size(panelWidth, panelHeight));
    panelBg->setPosition(Vec2::ZERO);
    this->addChild(panelBg, -2);

    float marginX = 9.0f;
    float marginTop = 12.0f;
    _originOffset = Vec2(marginX, panelHeight - marginTop);

    buildSlots();
    refresh();
    attachMouseHandlers();

    // Name label shown near top inside the panel
    _nameLabel = Label::createWithSystemFont("", "Arial", 14);
    _nameLabel->setAnchorPoint(Vec2(0.5f, 1.0f));
    float labelY = panelHeight - 4.0f;
    _nameLabel->setPosition(Vec2(panelWidth * 0.5f, labelY));
    this->addChild(_nameLabel, 5);

    // Tooltip (outside panel left side)
    _tooltipBg = LayerColor::create(Color4B(12, 16, 26, 230));
    _tooltipLabel = Label::createWithSystemFont("", "Arial", 14);
    _tooltipLabel->setColor(Color3B::WHITE);
    _tooltipLabel->enableOutline(Color4B(0, 0, 0, 160), 1);
    _tooltipLabel->setAnchorPoint(Vec2(1.0f, 0.5f));  // Right-aligned
    _tooltipBg->setAnchorPoint(Vec2::ANCHOR_BOTTOM_RIGHT);
    _tooltipBg->addChild(_tooltipLabel, 1);
    _tooltipBg->setVisible(false);
    this->addChild(_tooltipBg, 20);

    return true;
}

void EquipmentPanel::buildSlots() {
    _slotBg.resize(EQUIPMENT_SLOT_COUNT);
    _slotIcons.resize(EQUIPMENT_SLOT_COUNT);
    _slotOutline.resize(EQUIPMENT_SLOT_COUNT);

    for (int idx = 0; idx < EQUIPMENT_SLOT_COUNT; ++idx) {
        auto slotType = getEquipSlotType(idx);
        Vec2 pos = getEquipSlotPos(idx);

        // Create background sprite using equipment slot textures
        std::string texturePath = getEquipSlotTexture(slotType);
        Sprite* bg = Sprite::create(texturePath);
        if (bg) {
            bg->setPosition(pos);
            // Scale to fit slot size
            float scale = (_slotSize * 0.95f) / std::max(bg->getContentSize().width, bg->getContentSize().height);
            bg->setScale(scale);
            bg->setOpacity(200);
            this->addChild(bg, 0);
        } else {
            CCLOG("Warning: Failed to load %s, using default sprite", texturePath.c_str());
            // Fallback to colored sprite if image fails to load
            bg = Sprite::create();
            bg->setTextureRect(Rect(0, 0, _slotSize, _slotSize));
            bg->setColor(Color3B(80, 80, 80));
            bg->setOpacity(150);
            bg->setPosition(pos);
            bg->setScale(0.95f);
            this->addChild(bg, 0);
        }
        _slotBg[idx] = bg;

        auto outline = DrawNode::create();
        float half = _slotSize * 0.5f;
        Vec2 rect[4] = {Vec2(-half, -half), Vec2(half, -half), Vec2(half, half), Vec2(-half, half)};
        outline->drawPoly(rect, 4, true, Color4F(0.7f, 0.7f, 0.7f, 0.0f));
        outline->setPosition(pos);
        outline->setVisible(false);
        this->addChild(outline, 2);
        _slotOutline[idx] = outline;

        auto icon = Sprite::create();
        icon->setPosition(pos);
        this->addChild(icon, 3);
        _slotIcons[idx] = icon;
    }
}

void EquipmentPanel::refresh() {
    // TODO: Integrate with player equipment system
    // For now, just hide all icons
    for (size_t i = 0; i < _slotIcons.size(); ++i) {
        auto icon = _slotIcons[i];
        icon->setVisible(false);
    }
}

void EquipmentPanel::attachMouseHandlers() {
    auto mouseListener = EventListenerMouse::create();

    mouseListener->onMouseDown = [this](EventMouse* event) {
        Vec2 mousePos = event->getLocation();
        auto visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 pos(mousePos.x, visibleSize.height - mousePos.y);  // Flip Y-axis
        int idx = hitTestEquipSlot(pos);

        CCLOG("EquipmentPanel: Mouse down at screen=(%.1f, %.1f) -> GL=(%.1f, %.1f), hit slot: %d",
              mousePos.x, mousePos.y, pos.x, pos.y, idx);

        if (idx < 0) return;

        if (event->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {
            _selectedIndex = idx;
            updateHighlights();
            beginDrag(idx, pos);
        }
    };

    mouseListener->onMouseUp = [this](EventMouse* event) {
        if (!_dragging) return;
        Vec2 mousePos = event->getLocation();
        auto visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 pos(mousePos.x, visibleSize.height - mousePos.y);  // Flip Y-axis
        int target = hitTestEquipSlot(pos);
        endDrag(target);
    };

    mouseListener->onMouseMove = [this](EventMouse* event) {
        Vec2 mousePos = event->getLocation();
        auto visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 pos(mousePos.x, visibleSize.height - mousePos.y);  // Flip Y-axis

        if (_dragging) {
            updateDragSprite(pos);
        }

        int idx = hitTestEquipSlot(pos);
        if (idx >= 0) {
            if (idx != _hoverIndex) {
                _hoverIndex = idx;
                updateHighlights();
            }
            // TODO: Show equipment item info in tooltip
            _nameLabel->setString("");
            updateTooltip("", pos);
            return;
        } else if (_hoverIndex != -1) {
            _hoverIndex = -1;
            updateHighlights();
        }
        _nameLabel->setString("");
        updateTooltip("", Vec2::ZERO);
    };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);
}

int EquipmentPanel::hitTestEquipSlot(const Vec2& worldPos) const {
    Vec2 localPos = this->convertToNodeSpace(worldPos);

    CCLOG("EquipmentPanel hitTest: world=(%.1f, %.1f) -> local=(%.1f, %.1f), LayerPos=(%.1f, %.1f)",
          worldPos.x, worldPos.y, localPos.x, localPos.y,
          this->getPosition().x, this->getPosition().y);

    for (int i = 0; i < EQUIPMENT_SLOT_COUNT; ++i) {
        if (getEquipSlotRect(i).containsPoint(localPos)) {
            return i;
        }
    }
    return -1;
}

Rect EquipmentPanel::getEquipSlotRect(int index) const {
    if (index < 0 || index >= EQUIPMENT_SLOT_COUNT) return Rect::ZERO;
    Vec2 localPos = getEquipSlotPos(index);
    float half = _slotSize * 0.5f;
    return Rect(localPos.x - half, localPos.y - half, _slotSize, _slotSize);
}

Vec2 EquipmentPanel::getEquipSlotPos(int index) const {
    if (index < 0 || index >= EQUIPMENT_SLOT_COUNT) return Vec2::ZERO;

    float x = _originOffset.x + _slotSize * 0.5f;
    float y = _originOffset.y - (index * (_slotSize + _slotPadding)) - _slotSize * 0.5f;

    return Vec2(x, y);
}

EquipmentPanel::EquipSlotType EquipmentPanel::getEquipSlotType(int index) const {
    switch (index) {
        case 0: return EquipSlotType::Helmet;
        case 1: return EquipSlotType::Chestplate;
        case 2: return EquipSlotType::Leggings;
        case 3: return EquipSlotType::Accessory0;
        case 4: return EquipSlotType::Accessory1;
        case 5: return EquipSlotType::Accessory2;
        case 6: return EquipSlotType::Accessory3;
        default: return EquipSlotType::Helmet;
    }
}

std::string EquipmentPanel::getEquipSlotTexture(EquipSlotType type) const {
    switch (type) {
        case EquipSlotType::Helmet:
            return "items/bottom/Helmet.png";
        case EquipSlotType::Chestplate:
            return "items/bottom/Chestplate.png";
        case EquipSlotType::Leggings:
            return "items/bottom/Leggings.png";
        case EquipSlotType::Accessory0:
        case EquipSlotType::Accessory1:
        case EquipSlotType::Accessory2:
        case EquipSlotType::Accessory3:
            return "items/bottom/Accessory.png";
        default:
            return "items/bottom/Inventory.png";
    }
}

void EquipmentPanel::updateTooltip(const std::string& text, const Vec2& worldPos) {
    if (!_tooltipBg || !_tooltipLabel) return;
    if (text.empty()) {
        _tooltipBg->setVisible(false);
        return;
    }
    _tooltipLabel->setString(text);
    auto size = _tooltipLabel->getContentSize();
    float padding = 6.0f;
    _tooltipBg->setContentSize(Size(size.width + padding * 2, size.height + padding * 2));
    _tooltipLabel->setPosition(Vec2(size.width + padding, size.height * 0.5f + padding));

    Vec2 local = this->convertToNodeSpace(worldPos);
    float x = -8.0f;  // Position to the left of the panel
    float y = std::min(std::max(local.y, padding), this->getContentSize().height - padding);
    _tooltipBg->setPosition(Vec2(x, y));
    _tooltipBg->setVisible(true);
}

void EquipmentPanel::updateHighlights() {
    for (int i = 0; i < EQUIPMENT_SLOT_COUNT; ++i) {
        bool hover = (i == _hoverIndex);
        bool selected = (i == _selectedIndex) || (i == _dragSource);
        auto outline = _slotOutline[i];
        if (!outline) continue;
        outline->clear();
        float half = _slotSize * 0.5f;
        Vec2 rect[4] = {Vec2(-half, -half), Vec2(half, -half), Vec2(half, half), Vec2(-half, half)};
        if (selected) {
            outline->drawPoly(rect, 4, true, Color4F(0.95f, 0.85f, 0.45f, 0.9f));
            outline->setVisible(true);
        } else if (hover) {
            outline->drawPoly(rect, 4, true, Color4F(0.8f, 0.8f, 0.8f, 0.7f));
            outline->setVisible(true);
        } else {
            outline->setVisible(false);
        }
    }
}

void EquipmentPanel::beginDrag(int index, const Vec2& worldPos) {
    if (index < 0 || index >= EQUIPMENT_SLOT_COUNT) return;
    // TODO: Check if equipment slot has an item
    _dragging = false;  // Disabled for now until integration
    _dragSource = index;
    updateHighlights();
}

void EquipmentPanel::endDrag(int targetIndex) {
    if (!_dragging) return;
    _dragging = false;
    if (_dragSprite) _dragSprite->setVisible(false);

    if (targetIndex < 0) { updateHighlights(); return; }
    if (targetIndex == _dragSource) { updateHighlights(); return; }

    // TODO: Implement equipment swap/move logic

    _dragSource = -1;
    updateHighlights();
}

void EquipmentPanel::updateDragSprite(const Vec2& worldPos) {
    if (_dragSprite) {
        Vec2 local = this->convertToNodeSpace(worldPos);
        _dragSprite->setPosition(local);
    }
}
