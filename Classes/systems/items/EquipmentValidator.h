#ifndef __EQUIPMENT_VALIDATOR_H__
#define __EQUIPMENT_VALIDATOR_H__

#include "components/items/InventoryDef.h"

// Equipment validation system
// Validates whether an item can be equipped to a specific slot
class EquipmentValidator {
public:
    static EquipmentValidator* getInstance();

    // Check if an item can be equipped to a specific slot type
    // Returns true if the item's equipment type matches the slot type
    bool canEquip(int itemId, EquipSlotType slotType) const;

    // Check if an item is equipment
    bool isEquipment(int itemId) const;

    // Get the equipment type of an item
    EquipType getEquipType(int itemId) const;

    // Convert EquipSlotType to EquipType for matching
    // Accessory slots (0-3) all map to EquipType::Accessory
    EquipType slotTypeToEquipType(EquipSlotType slotType) const;

    // Check if an EquipType can fit in an EquipSlotType
    bool typesMatch(EquipType itemType, EquipSlotType slotType) const;

private:
    EquipmentValidator() = default;
    ~EquipmentValidator() = default;

    static EquipmentValidator* _instance;
};

#endif // __EQUIPMENT_VALIDATOR_H__
