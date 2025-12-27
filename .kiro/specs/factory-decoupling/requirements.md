# Requirements Document

## Introduction

本文档定义了将现有 MonsterFactory 解耦为分层工厂架构的需求。目标是将单一的大型工厂类拆分为三层结构：

1. **基础工厂** - 单一工厂的基础，定义通用接口和辅助方法
2. **专业工厂** - 处理特定怪物类型的具体工厂（SlimeFactory、ZombieFactory 等）
3. **总工厂** - 协调所有专业工厂，提供统一的对外接口

同时集成对象池预创建、配置缓存和组件内存布局优化。

## Glossary

- **Base_Factory**: 基础工厂抽象类，定义所有专业工厂的通用接口和组件创建辅助方法
- **Specialized_Factory**: 专业工厂，继承自 Base_Factory，处理特定类型怪物的创建逻辑
- **Master_Factory**: 总工厂，持有所有专业工厂实例，提供统一的实体创建入口
- **Entity_Pool**: 实体对象池，预创建并复用常用实体以减少运行时分配
- **Config_Cache**: 配置缓存，存储已解析的怪物配置以避免重复解析
- **Component_Layout**: 组件内存布局，优化 EnTT 组件的内存排列以提高缓存命中率
- **Monster_Type**: 怪物类型标识（如 "Slime"、"Zombie"、"DemonEye"）
- **Registry**: EnTT 注册表，管理所有实体和组件

## Requirements

### Requirement 1: 基础工厂接口

**User Story:** As a developer, I want a base factory interface that defines common entity creation patterns, so that all specialized factories follow a consistent contract.

#### Acceptance Criteria

1. THE Base_Factory SHALL define a pure virtual method `create()` that accepts registry, position, and parent node parameters
2. THE Base_Factory SHALL define a pure virtual method `supports()` that returns whether the factory can handle a given monster ID
3. THE Base_Factory SHALL provide protected helper methods for common component attachment (Transform, Health, Aggro, StateFlags)
4. THE Base_Factory SHALL provide a virtual method `getConfig()` that returns the configuration for a given monster ID
5. WHEN a Specialized_Factory inherits from Base_Factory, THE Specialized_Factory SHALL implement all pure virtual methods

### Requirement 2: 专业工厂实现

**User Story:** As a developer, I want specialized factories for each monster category, so that monster-specific creation logic is encapsulated and maintainable.

#### Acceptance Criteria

1. THE SlimeFactory SHALL handle all normal Slime-type monsters including GreenSlime, BlueSlime, IceSlime, SpikedSlime, UmbrellaSlime, MotherSlime, and BabySlime
2. THE ZombieFactory SHALL handle all Zombie-type monsters including Zombie, BaldZombie, and PincushionZombie variants
3. THE DemonEyeFactory SHALL handle all DemonEye-type monsters including DemonEye, PurpleEye, GreenEye, CataractEye, and DilatedEye
4. THE EaterFactory SHALL handle all EaterOfSouls and Crimera variants
5. THE AntlionFactory SHALL handle Antlion-type monsters with stationary shooting behavior
6. THE VultureFactory SHALL handle Vulture-type monsters with flying and diving behavior
7. THE KingSlimeFactory SHALL handle KingSlime boss with teleportation, slime spawning, and scale-based phase mechanics
8. WHEN a Specialized_Factory receives a monster ID it supports, THE Specialized_Factory SHALL create the entity with all required components
9. WHEN a Specialized_Factory receives a monster ID it does not support, THE Specialized_Factory SHALL return INVALID_ENTITY

### Requirement 3: 总工厂协调

**User Story:** As a developer, I want a master factory that coordinates all specialized factories, so that I have a single entry point for creating any monster type.

#### Acceptance Criteria

1. THE Master_Factory SHALL maintain a registry of all Specialized_Factory instances
2. WHEN `createMonster()` is called, THE Master_Factory SHALL iterate through registered factories to find one that supports the monster ID
3. WHEN a supporting factory is found, THE Master_Factory SHALL delegate creation to that factory
4. WHEN no supporting factory is found, THE Master_Factory SHALL log an error and return INVALID_ENTITY
5. THE Master_Factory SHALL provide methods to register and unregister Specialized_Factory instances at runtime
6. THE Master_Factory SHALL provide a method `getAllSupportedMonsterIds()` that aggregates IDs from all registered factories

### Requirement 4: 对象池预创建

**User Story:** As a developer, I want to pre-create commonly used entities in object pools, so that runtime entity creation is faster and causes less memory fragmentation.

#### Acceptance Criteria

1. THE Entity_Pool SHALL support pre-allocation of a specified number of entities for each monster type
2. WHEN `preallocate()` is called with a monster type and count, THE Entity_Pool SHALL create inactive entities with all required components
3. WHEN `acquire()` is called, THE Entity_Pool SHALL return an inactive entity from the pool and activate it
4. IF the pool is empty when `acquire()` is called, THEN THE Entity_Pool SHALL create a new entity dynamically
5. WHEN `release()` is called, THE Entity_Pool SHALL deactivate the entity and return it to the pool
6. THE Entity_Pool SHALL track pool statistics including total size, active count, and available count

### Requirement 5: 配置缓存

**User Story:** As a developer, I want configuration data to be cached after first load, so that batch creation operations don't repeatedly parse JSON files.

#### Acceptance Criteria

1. THE Config_Cache SHALL store parsed MonsterConfig objects indexed by monster ID
2. WHEN a configuration is requested, THE Config_Cache SHALL return the cached version if available
3. WHEN a configuration is not cached, THE Config_Cache SHALL load and parse the JSON file, then cache the result
4. THE Config_Cache SHALL provide a method to preload all configurations from a directory
5. THE Config_Cache SHALL provide a method to clear the cache for scene transitions
6. WHEN batch creating monsters, THE Master_Factory SHALL use cached configurations to avoid repeated parsing

### Requirement 6: 组件内存布局优化

**User Story:** As a developer, I want components to be arranged in memory for optimal cache performance, so that system updates are faster.

#### Acceptance Criteria

1. THE Base_Factory SHALL attach components in a consistent order optimized for common access patterns
2. THE component attachment order SHALL group frequently co-accessed components together (Transform, Physics, Render)
3. THE Base_Factory SHALL use EnTT's `emplace` method to ensure components are stored contiguously
4. WHEN creating entities in batch, THE Master_Factory SHALL use EnTT's group functionality to optimize iteration
5. THE Specialized_Factory SHALL document the component attachment order for each monster type

### Requirement 7: 向后兼容性

**User Story:** As a developer, I want the new factory system to be backward compatible with existing code, so that the refactoring doesn't break current functionality.

#### Acceptance Criteria

1. THE Master_Factory SHALL provide the same public interface as the current MonsterFactory
2. WHEN existing code calls `MonsterFactory::getInstance().createMonster()`, THE call SHALL work without modification
3. THE Master_Factory SHALL support all existing monster IDs and configurations
4. THE Master_Factory SHALL produce entities with identical component configurations as the current implementation
5. WHEN migrating to the new system, THE existing MonsterFactory class SHALL be deprecated but remain functional

