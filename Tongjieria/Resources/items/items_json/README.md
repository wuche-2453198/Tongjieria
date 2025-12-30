# Item JSON Structure

## ID Numbering Scheme

All items follow a structured ID numbering system:

### 1000+ Equipment
- **1100-1199**: Armor
  - 1101-1104: Copper Armor (Helmet, Chestplate, Greaves, Leggings)
  - 1111-1114: Iron Armor
  - 1121-1124: Silver Armor
  - 1131-1134: Gold Armor
  - 1141-1144: Aluminum Armor
  - 1151-1154: Lead Armor
  - 1161-1164: Zinc Armor
  - 1171-1174: Obdurite Armor
  - 1181-1184: Rhymestone Armor
  - 1191-1194: Zythium Armor

- **1200-1399**: Tools
  - 1201-1203: Wooden Tools (Sword, Axe, Pick)
  - 1211-1214: Stone Tools (Sword, Axe, Pick, Lighter)
  - 1221-1223: Copper Tools
  - 1231-1233: Iron Tools
  - 1241-1243: Silver Tools
  - 1251-1253: Gold Tools
  - 1261-1263: Aluminum Tools
  - 1271-1273: Lead Tools
  - 1281-1283: Zinc Tools
  - 1291-1293: Obdurite Tools
  - 1301-1303: Rhymestone Tools
  - 1311-1313: Magnetite Tools
  - 1321-1323: Irradium Tools
  - 1331: Wrench

### 2000+ Placeables
- **2001-2099**: Blocks
  - 2001-2006: Stone-based blocks
  - 2007-2008: Wood blocks
  - 2009-2010: Sand blocks
  - 2011-2012: Clay blocks
  - 2013: Glass
  - 2014: Mud
  - 2015: Snow
  - 2016: Zythium Lamp

- **2100-2199**: Machines
  - 2101: Workbench
  - 2102: Furnace
  - 2111-2119: Chests (Wooden, Stone, Copper, Iron, Silver, Gold, Zinc, Obdurite, Rhymestone)

### 3000+ Materials
- **3001-3099**: Ores
  - 3001-3014: Metal ores (Copper, Iron, Silver, Gold, Aluminum, Lead, Zinc, Obdurite, Rhymestone, Zythium, Magnetite, Irradium, Silicon, Uranium)
  - 3015: Coal
  - 3016-3019: Special stones (Lumenstone, Meltstone, Nullstone, Skystone)

- **3100-3199**: Ingots/Bars
  - 3101-3111: Metal ingots
  - 3112-3114: Special bars (Silicon, Uranium, Refined Uranium)
  - 3115-3118: Stone bars (Meltstone, Nullstone, Skystone, Zythium)

- **3200-3299**: Goo and drops
  - 3201-3206: Colored goo (Green, Blue, Red, Yellow, White, Black)
  - 3207: Rotten Chunk
  - 3208: Astral Shard
  - 3209: Bark

- **3300-3399**: Herbs
  - 3301-3308: Normal herbs (Greenleaf, Sunflower, Moonflower, Skyblossom, Frostleaf, Dryweed, Marshleaf, Caveroot)
  - 3309: Void Rot

- **3400-3499**: Seeds
  - 3401-3408: Normal seeds
  - 3409: Void Rot Seeds

- **3500-3599**: Other materials
  - 3501: Charcoal

### 4000+ Consumables
- **4001-4099**: Potions
  - 4001: Varnish

## JSON Format

Each item has the following structure:

```json
{
  "id": 1234,
  "name": "Item Name",
  "type": 1,
  "maxStack": 999,
  "icon": "items/Category/subcategory/item_name.png",
  "value": 10,
  "tags": [1, 101]
}
```

### Type Values
- 1: Armor (Equipment)
- 2: Tools (Equipment)
- 3: Materials
- 4: Consumables
- 5: Placeables

### Tag System
- First tag: Primary category (1=Equipment, 2=Tools, 3=Materials, 4=Consumables, 5=Placeables)
- Second tag: Subcategory
  - Armor: 101=Helmet, 102=Chestplate, 103=Greaves/Leggings
  - Tools: 201=Sword, 202=Axe, 203=Pick, 204=Lighter, 205=Wrench
  - Materials: 301=Ore, 302=Coal, 303=Special Stone, 311=Ingot, 312=Bar, 313=Stone Bar, 321=Goo, 322=Rotten, 323=Astral, 324=Bark, 331=Herb, 332=Corrupted, 341=Seed, 342=Corrupted Seed, 351=Dust
  - Placeables: 501-508=Block types, 511=Workbench, 512=Furnace, 513=Chest

## File Organization

- `items_equipment.json` - All armor and tools (ID 1000+)
- `items_placeables.json` - All blocks and machines (ID 2000+)
- `items_materials.json` - All crafting materials (ID 3000+)
- `items_consumables.json` - All potions and consumables (ID 4000+)

## Naming Convention

All file paths use lowercase with underscores (snake_case):
- `copper_helmet.png` ✓
- `CopperHelmet.png` ✗
- `copperhelmet.png` ✗
