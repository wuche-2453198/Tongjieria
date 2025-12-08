#include "Inventory.h"
#include "ItemManager.h"
#include "cocos2d.h"

using namespace cocos2d;

Inventory* Inventory::_instance = nullptr;

Inventory* Inventory::getInstance() {
    if (!_instance) {
        _instance = new Inventory();
        _instance->init();
    }
    return _instance;
}

void Inventory::init(size_t capacity) {
    _slots.clear();
    _slots.assign(capacity, InventorySlot{});
    dispatchInventoryChanged();
}

void Inventory::clear() {
    for (auto& slot : _slots) {
        slot = InventorySlot{};
    }
    dispatchInventoryChanged();
}

bool Inventory::addItem(int id, int count) {
    int remaining = addItemInternal(id, count);
    dispatchInventoryChanged();
    return remaining == 0;
}

std::vector<InventorySlot> Inventory::addItemWithOverflow(int id, int count) {
    std::vector<InventorySlot> drops;
    int remaining = addItemInternal(id, count);
    if (remaining > 0) {
        drops.push_back({id, remaining, 0});
    }
    dispatchInventoryChanged();
    return drops;
}

bool Inventory::removeItem(int id, int count) {
    if (id == 0 || count <= 0) return false;
    int total = getItemCount(id);
    if (total < count) return false;

    int remaining = count;
    for (auto& slot : _slots) {
        if (slot.itemId != id) continue;
        int toRemove = std::min(slot.count, remaining);
        slot.count -= toRemove;
        remaining -= toRemove;
        if (slot.count == 0) {
            slot.itemId = 0;
            slot.prefixId = 0;
        }
        if (remaining <= 0) break;
    }

    dispatchInventoryChanged();
    return true;
}

int Inventory::getItemCount(int id) const {
    int total = 0;
    for (const auto& slot : _slots) {
        if (slot.itemId == id) {
            total += slot.count;
        }
    }
    return total;
}

bool Inventory::swapSlots(size_t a, size_t b) {
    if (a >= _slots.size() || b >= _slots.size()) return false;
    if (a == b) return true;
    std::swap(_slots[a], _slots[b]);
    dispatchInventoryChanged();
    return true;
}

bool Inventory::moveItem(size_t from, size_t to, int count) {
    if (count <= 0) return false;
    if (from >= _slots.size() || to >= _slots.size()) return false;
    auto& src = _slots[from];
    auto& dst = _slots[to];
    if (src.itemId == 0 || src.count == 0) return false;

    int moveCount = std::min(count, src.count);
    if (dst.itemId == 0) {
        dst = src;
        dst.count = moveCount;
        src.count -= moveCount;
        if (src.count == 0) {
            src.itemId = 0;
            src.prefixId = 0;
        }
    } else if (dst.itemId == src.itemId) {
        auto def = ItemManager::getInstance()->getItemData(dst.itemId);
        int maxStack = def ? def->maxStack : 999;
        int canTake = maxStack - dst.count;
        int toAdd = std::min(canTake, moveCount);
        dst.count += toAdd;
        src.count -= toAdd;
        if (src.count == 0) {
            src.itemId = 0;
            src.prefixId = 0;
        }
        if (toAdd == 0) return false;
    } else {
        std::swap(src, dst);
    }

    dispatchInventoryChanged();
    return true;
}

int Inventory::getFirstEmptySlot() const {
    for (size_t i = 0; i < _slots.size(); ++i) {
        if (_slots[i].itemId == 0) return static_cast<int>(i);
    }
    return -1;
}

int Inventory::addItemInternal(int id, int count) {
    if (id == 0 || count <= 0) return count;
    auto def = ItemManager::getInstance()->getItemData(id);
    if (!def) return count;
    int maxStack = def->maxStack > 0 ? def->maxStack : 999;
    int remaining = count;

    // First pass: fill existing stacks
    for (auto& slot : _slots) {
        if (slot.itemId != id) continue;
        if (slot.count >= maxStack) continue;
        int canTake = maxStack - slot.count;
        int toAdd = std::min(canTake, remaining);
        slot.count += toAdd;
        remaining -= toAdd;
        if (remaining <= 0) break;
    }

    // Second pass: use empty slots
    if (remaining > 0) {
        for (auto& slot : _slots) {
            if (slot.itemId != 0) continue;
            int toAdd = std::min(maxStack, remaining);
            slot.itemId = id;
            slot.count = toAdd;
            slot.prefixId = 0;
            remaining -= toAdd;
            if (remaining <= 0) break;
        }
    }

    return remaining;
}

void Inventory::dispatchInventoryChanged() {
    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("Event_InventoryChanged");
}
