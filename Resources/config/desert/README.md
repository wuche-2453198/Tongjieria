# Desert Monsters Configuration

This directory contains configuration files for desert-themed monsters.

## Usage

Place JSON configuration files for desert monsters here. The files should follow the same format as other monster configs in the project.

## Example Structure

```
desert/
├── DesertScorpion_Small.json
├── DesertScorpion_Medium.json  
├── DesertScorpion_Large.json
├── SandWorm.json
├── Vulture.json
└── ...
```

## Test Scene

Use `DesertTestScene` to test desert monsters. The scene includes:
- Desert-themed environment (sand colors, rocks, dunes)
- Virtual player with WASD movement
- All necessary ECS systems for testing various monster types
- Physics environment suitable for desert creatures

## Monster Types Support

The desert test scene supports:
- Flying monsters (EaterOfSoulsAI, DemonEyeAI)
- Ground-based monsters (WarriorAI, SlimeAI)  
- Jumping monsters (JumpMovement system)
- Special abilities (debuffs, slow fall, etc.)
