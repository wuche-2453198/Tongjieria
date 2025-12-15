#include "InventoryLayer.h"
#include <algorithm>
#include <cmath>
#include <string>

namespace {
constexpr int kNormalSlots = 40;
constexpr int kCoinSlots = 4;
constexpr int kTrashSlots = 1;
constexpr int kAmmoSlots = 4;  // Ammo slots (changed from Armor)
constexpr int kWeaponSlots = 10;
constexpr int kTotalSlots = kNormalSlots + kCoinSlots + kAmmoSlots + kTrashSlots + kWeaponSlots;
constexpr int kNormalCols = 10;
constexpr int kNormalRows = 4; // 10 cols x 4 rows = 40 slots
constexpr float kWeaponScale = 1.1f;
}

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

    // Derive slot count and grid rows from inventory size (supports new special slots)
    _slotCount = static_cast<int>(Inventory::getInstance()->getSlots().size());
    if (_slotCount == 0) {
        _slotCount = kTotalSlots; // fallback to expected layout if inventory not initialized yet
    }

    _cols = kNormalCols;
    _rows = kNormalRows;
    _slotSize = 40.0f;       // target size ~40px
    _slotPadding = 3.0f;     // 2~4px gap
    _specialGap = 6.0f;      // gap between main grid and special column

    auto director = Director::getInstance();
    const auto visibleSize = director->getVisibleSize();
    const auto origin = director->getVisibleOrigin();

    // Panel occupies top-left quarter of the screen
    float panelWidth = visibleSize.width * 0.25f;
    float panelHeight = visibleSize.height * 0.25f;
    setIgnoreAnchorPointForPosition(false);
    setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
    setContentSize(Size(panelWidth, panelHeight));
    setPosition(Vec2(origin.x, origin.y + visibleSize.height - panelHeight));

    // Background panel: semi-transparent dark with light border
    auto panelBg = LayerColor::create(Color4B(18, 22, 32, 180));
    panelBg->setContentSize(Size(panelWidth, panelHeight));
    panelBg->setPosition(Vec2::ZERO);
    this->addChild(panelBg, -2);

    auto border = DrawNode::create();
    Color4F borderColor(0.85f, 0.74f, 0.40f, 0.9f); // warm gold-ish
    Vec2 rect[4] = {Vec2(1, 1), Vec2(panelWidth - 1, 1), Vec2(panelWidth - 1, panelHeight - 1), Vec2(1, panelHeight - 1)};
    border->drawPoly(rect, 4, true, borderColor);
    border->setPosition(Vec2::ZERO);
    this->addChild(border, -1);

    // Layout sizes (fixed slot size; panel will expand if needed)
    float weaponSize = _slotSize * kWeaponScale; // weapon slightly larger
    float gridMainWidth = kNormalCols * _slotSize + (kNormalCols - 1) * _slotPadding;
    float gridMainHeight = kNormalRows * _slotSize + (kNormalRows - 1) * _slotPadding;
    float coinWidth = _slotSize;
    float coinHeight = (kCoinSlots + kTrashSlots) * _slotSize + (kCoinSlots + kTrashSlots - 1) * _slotPadding;
    float ammoWidth = kAmmoSlots * _slotSize + (kAmmoSlots - 1) * _slotPadding;
    float ammoHeight = _slotSize;
    float weaponWidth = kWeaponSlots * weaponSize + (kWeaponSlots - 1) * _slotPadding;
    float weaponHeight = weaponSize;

    // Right block width: max of weapon row vs (ammo row + gap + coin column)
    float rightBlockWidth = std::max(weaponWidth, ammoWidth + _specialGap + coinWidth);
    float totalWidth = gridMainWidth + _specialGap + rightBlockWidth;
    float rightBlockHeight = weaponHeight + _slotPadding + std::max(ammoHeight, coinHeight);
    float totalHeight = std::max(gridMainHeight, rightBlockHeight);

    _layoutHeight = totalHeight;

    // panel ensures it can hold grid while approaching 1/4 of screen
    panelWidth = std::max(panelWidth, totalWidth + 12.0f);
    panelHeight = std::max(panelHeight, totalHeight + 12.0f);
    setContentSize(Size(panelWidth, panelHeight));
    setPosition(Vec2(origin.x, origin.y + visibleSize.height - panelHeight));
    panelBg->setContentSize(Size(panelWidth, panelHeight));

    float borderWidth = panelWidth - 1;
    float borderHeight = panelHeight - 1;
    border->clear();
    Vec2 rect2[4] = {Vec2(1, 1), Vec2(borderWidth, 1), Vec2(borderWidth, borderHeight), Vec2(1, borderHeight)};
    Color4F borderColor2(0.85f, 0.74f, 0.40f, 0.9f);
    border->drawPoly(rect2, 4, true, borderColor2);

    float marginX = 6.0f;
    float marginTop = 6.0f;
    _originOffset = Vec2(marginX, panelHeight - marginTop - totalHeight);

    buildSlots();
    refresh();
    attachMouseHandlers();

    // Listen to inventory change events
    auto listener = EventListenerCustom::create("Event_InventoryChanged", CC_CALLBACK_1(InventoryLayer::onInventoryChanged, this));
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    // Name label shown near top inside the panel
    _nameLabel = Label::createWithSystemFont("", "Arial", 16);
    _nameLabel->setAnchorPoint(Vec2(0.5f, 1.0f));
    float labelY = panelHeight - 4.0f;
    _nameLabel->setPosition(Vec2(panelWidth * 0.5f, labelY));
    this->addChild(_nameLabel, 5);

    // Tooltip (outside panel right side)
    _tooltipBg = LayerColor::create(Color4B(12, 16, 26, 230));
    _tooltipLabel = Label::createWithSystemFont("", "Arial", 16);
    _tooltipLabel->setColor(Color3B::WHITE);
    _tooltipLabel->enableOutline(Color4B(0, 0, 0, 160), 1);
    _tooltipLabel->setAnchorPoint(Vec2(0, 0.5f));
    _tooltipBg->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
    _tooltipBg->addChild(_tooltipLabel, 1);
    _tooltipBg->setVisible(false);
    this->addChild(_tooltipBg, 20);

    return true;
}

void InventoryLayer::buildSlots() {
    _slotBg.resize(_slotCount);
    _slotIcons.resize(_slotCount);
    _slotCounts.resize(_slotCount);
    _slotTags.resize(_slotCount);
    _slotOutline.resize(_slotCount);
    _slotSizes.resize(_slotCount, _slotSize);

    for (int idx = 0; idx < _slotCount; ++idx) {
        auto kind = getSlotKind(idx);
        float size = getSlotSize(kind);
        _slotSizes[idx] = size;
        Vec2 pos = getSlotPos(idx);

        auto bg = Sprite::create();
        bg->setTextureRect(Rect(0, 0, size, size));
        bg->setColor(getSlotColor(kind));
        bg->setOpacity(150);
        bg->setPosition(pos);
        bg->setScale(kind == SlotKind::Weapon ? 1.0f : 0.94f); // weapon slightly larger, normal slightly smaller
        this->addChild(bg, 0);
        _slotBg[idx] = bg;

        auto pattern = DrawNode::create();
        drawSlotPattern(pattern, kind, size);
        pattern->setPosition(pos);
        this->addChild(pattern, 1);

        auto outline = DrawNode::create();
        float half = size * 0.5f;
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

        auto countLabel = Label::createWithSystemFont("", "Arial", 14);
        countLabel->setColor(Color3B::WHITE);
        countLabel->setAnchorPoint(Vec2(0.0f, 1.0f));
        countLabel->setPosition(pos + Vec2(-size * 0.5f + 2, size * 0.5f - 2));
        this->addChild(countLabel, 4);
        _slotCounts[idx] = countLabel;

        // Generate slot label based on kind
        std::string tagText = "";
        if (kind == SlotKind::Weapon) {
            int num = idx - kNormalSlots + 1;
            // Weapon slots display 1-9 and 0 (10th slot shows as 0)
            int displayNum = (num == 10) ? 0 : num;
            tagText = std::to_string(displayNum);
            CCLOG("InventoryLayer: Weapon slot %d -> label '%s' (idx=%d)", num, tagText.c_str(), idx);
        } else if (kind == SlotKind::Trash) {
            tagText = "X";
        } else {
            // Ammo, Coin, and Normal slots don't need labels
            tagText = "";
        }

        auto tagLabel = Label::createWithSystemFont(tagText, "Arial", kind == SlotKind::Weapon ? 20.0f : 12.0f);
        tagLabel->setColor(Color3B(255, 255, 255));  // Bright white for better visibility

        // Use a stronger shadow for visibility
        tagLabel->enableShadow(Color4B(0, 0, 0, 255), Size(2, -2), 2);

        if (kind == SlotKind::Weapon) {
            // Weapon slot hotkeys display above the slot
            tagLabel->setAnchorPoint(Vec2(0.5f, 1.0f));  // Changed to bottom-center anchor
            Vec2 labelPos = pos + Vec2(0, size * 0.5f + 2);  // Position just above slot
            tagLabel->setPosition(labelPos);
            CCLOG("InventoryLayer: Weapon tag '%s' at pos (%.1f, %.1f), slot pos (%.1f, %.1f), size=%.1f",
                  tagText.c_str(), labelPos.x, labelPos.y, pos.x, pos.y, size);
        } else {
            tagLabel->setAnchorPoint(Vec2(0.5f, 1.0f));  // Changed to bottom-center anchor
            tagLabel->setPosition(pos + Vec2(0, -size * 0.5f - 1));
        }
        tagLabel->setVisible(!tagText.empty());
        this->addChild(tagLabel, 100);  // Very high z-order to ensure it's on top
        _slotTags[idx] = tagLabel;

        if (kind == SlotKind::Weapon) {
            auto labelSize = tagLabel->getContentSize();
            CCLOG("InventoryLayer: Tag label added for weapon slot %d, visible=%d, text='%s', size=(%.1f,%.1f)",
                  idx, tagLabel->isVisible(), tagText.c_str(), labelSize.width, labelSize.height);
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
            float targetSize = _slotSizes.empty() ? _slotSize : _slotSizes[i];
            icon->setScale(targetSize / icon->getContentSize().width * 0.8f);
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
        // Manually convert screen coordinates (Y-down) to OpenGL coordinates (Y-up)
        Vec2 mousePos = event->getLocation();
        auto visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 pos(mousePos.x, visibleSize.height - mousePos.y);  // Flip Y-axis
        int idx = hitTestSlot(pos);

        CCLOG("InventoryLayer: Mouse down at screen=(%.1f, %.1f) -> GL=(%.1f, %.1f), hit slot: %d",
              mousePos.x, mousePos.y, pos.x, pos.y, idx);

        if (idx < 0) return;

        const auto& slots = Inventory::getInstance()->getSlots();
        if (idx < (int)slots.size() && slots[idx].itemId != 0) {
            auto def = ItemManager::getInstance()->getItemData(slots[idx].itemId);
            CCLOG("InventoryLayer: Clicked on slot %d, item: %s (ID:%d) x%d",
                  idx, def ? def->name.c_str() : "Unknown", slots[idx].itemId, slots[idx].count);
        } else {
            CCLOG("InventoryLayer: Clicked on empty slot %d", idx);
        }

        if (event->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {
            _selectedIndex = idx;
            updateHighlights();
            beginDrag(idx, pos);
        } else if (event->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT) {
            // Split half to first empty slot
            int target = Inventory::getInstance()->getFirstEmptySlot();
            if (target >= 0 && target != idx) {
                int half = slots[idx].count / 2;
                if (half > 0) {
                    CCLOG("InventoryLayer: Right-click split half (%d) from slot %d to slot %d", half, idx, target);
                    Inventory::getInstance()->moveItem(idx, target, half);
                }
            }
        }
    };

    mouseListener->onMouseUp = [this](EventMouse* event) {
        if (!_dragging) return;
        // Manually convert screen coordinates (Y-down) to OpenGL coordinates (Y-up)
        Vec2 mousePos = event->getLocation();
        auto visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 pos(mousePos.x, visibleSize.height - mousePos.y);  // Flip Y-axis
        int target = hitTestSlot(pos);
        endDrag(target);
    };

    mouseListener->onMouseMove = [this](EventMouse* event) {
        // Manually convert screen coordinates (Y-down) to OpenGL coordinates (Y-up)
        Vec2 mousePos = event->getLocation();
        auto visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 pos(mousePos.x, visibleSize.height - mousePos.y);  // Flip Y-axis

        if (_dragging) {
            updateDragSprite(pos);
        }
        int idx = hitTestSlot(pos);
        if (idx >= 0) {
            if (idx != _hoverIndex) {
                _hoverIndex = idx;
                updateHighlights();
            }
            const auto& slots = Inventory::getInstance()->getSlots();
            if (idx < (int)slots.size() && slots[idx].itemId != 0) {
                auto def = ItemManager::getInstance()->getItemData(slots[idx].itemId);
                _nameLabel->setString(def ? def->name : "");
                updateTooltip(def ? def->name : "", pos);
                return;
            }
        } else if (_hoverIndex != -1) {
            _hoverIndex = -1;
            updateHighlights();
        }
        _nameLabel->setString("");
        updateTooltip("", Vec2::ZERO);
    };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);
}

int InventoryLayer::hitTestSlot(const Vec2& worldPos) const {
    // Convert world position to local coordinate space
    Vec2 localPos = this->convertToNodeSpace(worldPos);

    // Debug: log every click with coordinate conversion and layer info
    CCLOG("InventoryLayer hitTest: world=(%.1f, %.1f) -> local=(%.1f, %.1f), LayerPos=(%.1f, %.1f), ContentSize=(%.1f, %.1f)",
          worldPos.x, worldPos.y, localPos.x, localPos.y,
          this->getPosition().x, this->getPosition().y,
          this->getContentSize().width, this->getContentSize().height);

    // Debug: log coordinate conversion (only once)
    static bool debugLogged = false;
    if (!debugLogged) {
        CCLOG("=== InventoryLayer Slot Rect Debug ===");
        CCLOG("Layer anchor point: (%.1f, %.1f)", this->getAnchorPoint().x, this->getAnchorPoint().y);
        CCLOG("Layer position: (%.1f, %.1f)", this->getPosition().x, this->getPosition().y);
        CCLOG("Layer content size: (%.1f, %.1f)", this->getContentSize().width, this->getContentSize().height);
        CCLOG("_originOffset: (%.1f, %.1f)", _originOffset.x, _originOffset.y);
        CCLOG("_layoutHeight: %.1f", _layoutHeight);

        CCLOG("--- Normal slots (first 5) ---");
        for (int i = 0; i < std::min(5, _slotCount); ++i) {
            auto rect = getSlotRect(i);
            Vec2 slotPos = getSlotPos(i);
            CCLOG("Slot %d: pos=(%.1f, %.1f) rect=(%.1f, %.1f, %.1f x %.1f)",
                  i, slotPos.x, slotPos.y, rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);
        }
        // Also log weapon slots (indices 40-44)
        CCLOG("--- Weapon slots (40-44) ---");
        for (int i = 40; i < std::min(45, _slotCount); ++i) {
            auto rect = getSlotRect(i);
            Vec2 slotPos = getSlotPos(i);
            CCLOG("Slot %d: pos=(%.1f, %.1f) rect=(%.1f, %.1f, %.1f x %.1f)",
                  i, slotPos.x, slotPos.y, rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);
        }
        debugLogged = true;
    }

    // Check if local position is within any slot rect
    for (int i = 0; i < _slotCount; ++i) {
        if (getSlotRect(i).containsPoint(localPos)) {
            return i;
        }
    }
    return -1;
}

Rect InventoryLayer::getSlotRect(int index) const {
    if (index < 0 || index >= _slotCount) return Rect::ZERO;
    Vec2 localPos = getSlotPos(index);
    float size = getSlotSize(getSlotKind(index));
    float half = size * 0.5f;
    // Return rect in local coordinate space
    return Rect(localPos.x - half, localPos.y - half, size, size);
}

Vec2 InventoryLayer::getSlotPos(int index) const {
    constexpr int GRID_COLS = 10;
    const float START_X = _originOffset.x;
    // Use _originOffset.y as the bottom-left starting point
    const float BASE_Y = _originOffset.y;
    const SlotKind kind = getSlotKind(index);
    const float weaponSize = _slotSize * kWeaponScale;

    if (kind == SlotKind::Weapon) {
        const int localIndex = index - kNormalSlots;
        const float x = START_X + localIndex * (weaponSize + _slotPadding) + weaponSize * 0.5f;
        const float y = BASE_Y + _layoutHeight - weaponSize * 0.5f;
        return Vec2(x, y);
    }

    if (kind == SlotKind::Normal) {
        const int row = index / GRID_COLS;
        const int col = index % GRID_COLS;
        const float x = START_X + col * (_slotSize + _slotPadding) + _slotSize * 0.5f;
        const float y = BASE_Y + _layoutHeight - weaponSize - _slotPadding - (row * (_slotSize + _slotPadding)) - _slotSize * 0.5f;
        return Vec2(x, y);
    }

    const float rightSectionX = START_X + GRID_COLS * (_slotSize + _slotPadding) + _specialGap;

    if (kind == SlotKind::Ammo) {
        const int ammoIndex = index - (kNormalSlots + kWeaponSlots);
        const float x = rightSectionX + _slotSize * 0.5f;
        const float y = BASE_Y + _layoutHeight - weaponSize - _slotPadding - ammoIndex * (_slotSize + _slotPadding) - _slotSize * 0.5f;
        return Vec2(x, y);
    }

    if (kind == SlotKind::Coin) {
        const int coinIndex = index - (kNormalSlots + kWeaponSlots + kAmmoSlots);
        const float x = rightSectionX + _slotSize + _slotPadding + _slotSize * 0.5f;
        const float y = BASE_Y + _layoutHeight - weaponSize - _slotPadding - coinIndex * (_slotSize + _slotPadding) - _slotSize * 0.5f;
        return Vec2(x, y);
    }

    if (kind == SlotKind::Trash) {
        const float x = rightSectionX + (_slotSize + _slotPadding) * 0.5f;
        const float y = BASE_Y + _layoutHeight - weaponSize - _slotPadding - kAmmoSlots * (_slotSize + _slotPadding) - _slotSize * 0.5f;
        return Vec2(x, y);
    }

    return Vec2::ZERO;
}

InventoryLayer::SlotKind InventoryLayer::getSlotKind(int index) const {
    if (index < kNormalSlots) return SlotKind::Normal;
    if (index < kNormalSlots + kWeaponSlots) return SlotKind::Weapon;
    int specialIndex = index - (kNormalSlots + kWeaponSlots);
    if (specialIndex < kAmmoSlots) return SlotKind::Ammo;
    specialIndex -= kAmmoSlots;
    if (specialIndex < kCoinSlots) return SlotKind::Coin;
    if (specialIndex == kCoinSlots) return SlotKind::Trash;
    return SlotKind::Normal;
}

float InventoryLayer::getSlotSize(SlotKind kind) const {
    return kind == SlotKind::Weapon ? _slotSize * kWeaponScale : _slotSize;
}

cocos2d::Color3B InventoryLayer::getSlotColor(SlotKind kind) const {
    switch (kind) {
        case SlotKind::Coin:   return Color3B(184, 146, 64);
        case SlotKind::Ammo:   return Color3B(120, 140, 180);
        case SlotKind::Trash:  return Color3B(180, 70, 70);
        case SlotKind::Weapon: return Color3B(90, 150, 210);
        case SlotKind::Normal:
        default:                 return Color3B(80, 80, 80);
    }
}

std::string InventoryLayer::getSlotLabel(SlotKind kind) const {
    switch (kind) {
        case SlotKind::Coin:   return "Coin";
        case SlotKind::Ammo:   return "Ammo";
        case SlotKind::Trash:  return "Trash";
        case SlotKind::Weapon: return "Weapon";
        case SlotKind::Normal:
        default:               return "";
    }
}

void InventoryLayer::drawSlotPattern(DrawNode* node, SlotKind kind, float size) {
    if (!node) return;
    node->clear();
    float half = size * 0.5f;
    Color4F tint(1,1,1,0.08f);
    switch (kind) {
        case SlotKind::Coin: {
            Vec2 bag[5] = {Vec2(-half * 0.5f, -half * 0.2f), Vec2(0, -half * 0.1f), Vec2(half * 0.5f, -half * 0.2f),
                           Vec2(half * 0.35f, -half * 0.6f), Vec2(-half * 0.35f, -half * 0.6f)};
            node->drawSolidPoly(bag, 5, Color4F(0.95f, 0.8f, 0.3f, 0.1f));
            node->drawSolidCircle(Vec2(0, half * 0.1f), half * 0.25f, CC_DEGREES_TO_RADIANS(360), 12, Color4F(0.95f, 0.8f, 0.3f, 0.08f));
            break;
        }
        case SlotKind::Trash: {
            node->drawLine(Vec2(-half * 0.5f, -half * 0.5f), Vec2(half * 0.5f, half * 0.5f), Color4F(0.9f, 0.3f, 0.3f, 0.35f));
            node->drawLine(Vec2(-half * 0.5f, half * 0.5f), Vec2(half * 0.5f, -half * 0.5f), Color4F(0.9f, 0.3f, 0.3f, 0.35f));
            break;
        }
        case SlotKind::Weapon: {
            Vec2 bar[4] = {Vec2(-half * 0.6f, -half * 0.05f), Vec2(half * 0.6f, -half * 0.05f),
                           Vec2(half * 0.6f, half * 0.05f), Vec2(-half * 0.6f, half * 0.05f)};
            node->drawSolidPoly(bar, 4, Color4F(0.9f, 0.9f, 0.9f, 0.1f));
            break;
        }
        case SlotKind::Normal:
        default:
            break;
    }
}

void InventoryLayer::updateTooltip(const std::string& text, const Vec2& worldPos) {
    if (!_tooltipBg || !_tooltipLabel) return;
    if (text.empty()) {
        _tooltipBg->setVisible(false);
        return;
    }
    _tooltipLabel->setString(text);
    auto size = _tooltipLabel->getContentSize();
    float padding = 6.0f;
    _tooltipBg->setContentSize(Size(size.width + padding * 2, size.height + padding * 2));
    _tooltipLabel->setPosition(Vec2(padding, size.height * 0.5f + padding));

    Vec2 local = this->convertToNodeSpace(worldPos);
    float x = this->getContentSize().width + 8.0f;
    float y = std::min(std::max(local.y, padding), this->getContentSize().height - padding);
    _tooltipBg->setPosition(Vec2(x, y));
    _tooltipBg->setVisible(true);
}

void InventoryLayer::updateHighlights() {
    for (int i = 0; i < _slotCount; ++i) {
        bool hover = (i == _hoverIndex);
        bool selected = (i == _selectedIndex) || (i == _dragSource);
        bool droppingTrash = _dragging && hover && getSlotKind(i) == SlotKind::Trash;
        auto outline = _slotOutline[i];
        if (!outline) continue;
        outline->clear();
        float half = _slotSizes.empty() ? _slotSize * 0.5f : _slotSizes[i] * 0.5f;
        Vec2 rect[4] = {Vec2(-half, -half), Vec2(half, -half), Vec2(half, half), Vec2(-half, half)};
        if (droppingTrash) {
            outline->drawPoly(rect, 4, true, Color4F(0.95f, 0.2f, 0.2f, 0.9f));
            outline->setVisible(true);
        } else if (selected) {
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
    float dragSize = (_slotSizes.empty() || index >= (int)_slotSizes.size()) ? _slotSize : _slotSizes[index];
    _dragSprite->setScale(dragSize / _dragSprite->getContentSize().width * 0.8f);
    updateDragSprite(worldPos);
    updateTooltip("", worldPos);
}

void InventoryLayer::endDrag(int targetIndex) {
    if (!_dragging) return;
    _dragging = false;
    if (_dragSprite) _dragSprite->setVisible(false);

    if (targetIndex < 0) { updateHighlights(); return; }
    if (targetIndex == _dragSource) { updateHighlights(); return; }

    auto inv = Inventory::getInstance();
    const auto& slots = inv->getSlots();
    if (_dragSource < (int)slots.size()) {
        int count = slots[_dragSource].count;
        inv->moveItem(_dragSource, targetIndex, count);
    }
    _dragSource = -1;
    updateHighlights();
}

void InventoryLayer::updateDragSprite(const Vec2& worldPos) {
    if (_dragSprite) {
        Vec2 local = this->convertToNodeSpace(worldPos);
        _dragSprite->setPosition(local);
    }
}

void InventoryLayer::onInventoryChanged(EventCustom* event) {
    refresh();
    updateHighlights();
}
