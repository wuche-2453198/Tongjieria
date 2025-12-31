#include "EquipmentValidator.h"
#include "ItemManager.h"
#include "cocos2d.h"

EquipmentValidator* EquipmentValidator::_instance = nullptr;

EquipmentValidator* EquipmentValidator::getInstance() {
    if (!_instance) {
        _instance = new (std::nothrow) EquipmentValidator();
    }
    return _instance;
}

bool EquipmentValidator::canEquip(int itemId, EquipSlotType slotType) const {
    if (itemId == 0) return false;  // Empty item cannot be equipped
    if (slotType == EquipSlotType::None) return false;

    auto itemDef = ItemManager::getInstance()->getItemData(itemId);
    if (!itemDef) {
        CCLOG("EquipmentValidator: Item %d not found", itemId);
        return false;
    }

    // Check if item type matches slot type
    bool match = typesMatch(itemDef->equipType, slotType);

    CCLOG("EquipmentValidator: Item %d (%s) equipType=%d, slotType=%d, match=%d",
          itemId, itemDef->name.c_str(), (int)itemDef->equipType, (int)slotType, match);

    return match;
}

bool EquipmentValidator::isEquipment(int itemId) const {
    if (itemId == 0) return false;

    auto itemDef = ItemManager::getInstance()->getItemData(itemId);
    if (!itemDef) return false;

    return itemDef->equipType != EquipType::None;
}

EquipType EquipmentValidator::getEquipType(int itemId) const {
    if (itemId == 0) return EquipType::None;

    auto itemDef = ItemManager::getInstance()->getItemData(itemId);
    if (!itemDef) return EquipType::None;

    return itemDef->equipType;
}

EquipType EquipmentValidator::slotTypeToEquipType(EquipSlotType slotType) const {
    switch (slotType) {
        case EquipSlotType::Helmet:
            return EquipType::Helmet;
        case EquipSlotType::Chestplate:
            return EquipType::Chestplate;
        case EquipSlotType::Leggings:
            return EquipType::Leggings;
        case EquipSlotType::Accessory0:
        case EquipSlotType::Accessory1:
        case EquipSlotType::Accessory2:
        case EquipSlotType::Accessory3:
            return EquipType::Accessory;
        default:
            return EquipType::None;
    }
}

bool EquipmentValidator::typesMatch(EquipType itemType, EquipSlotType slotType) const {
    if (itemType == EquipType::None) return false;

    EquipType requiredType = slotTypeToEquipType(slotType);
    return itemType == requiredType;
}
