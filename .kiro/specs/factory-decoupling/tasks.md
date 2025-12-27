# Implementation Plan: Factory Decoupling

## Overview

将现有 MonsterFactory 重构为三层工厂架构：基础工厂 → 专业工厂 → 总工厂。实现对象池预创建、配置缓存和组件内存布局优化。

## Tasks

- [x] 1. 创建基础工厂接口和基类
  - [x] 1.1 更新 IMonsterFactory 接口
    - 添加 `getSupportedIds()` 方法
    - 确保接口与设计文档一致
    - _Requirements: 1.1, 1.2, 1.4_

  - [x] 1.2 创建 BaseMonsterFactory 基类
    - 实现配置加载方法 `loadConfigs()`, `loadSingleConfig()`
    - 实现 `supports()`, `getConfig()`, `getSupportedIds()`
    - 创建文件 `Classes/core/factory/monster/BaseMonsterFactory.h/.cpp`
    - _Requirements: 1.1, 1.2, 1.3, 1.4_

  - [x] 1.3 实现通用组件附加方法
    - `attachCoreComponents()` - Transform, EntityStateFlags
    - `attachPhysicsComponents()` - PhysicsBodyComponent
    - `attachRenderComponents()` - RenderComponent, ParentNodeComponent, AnimationComponent
    - `attachCombatComponents()` - HealthComponent, AggroComponent
    - `registerSpriteResource()` - 注册精灵资源到 SpriteManager
    - 按设计文档中的组件顺序附加以优化内存布局
    - _Requirements: 1.3, 6.1, 6.2, 6.3_

- [x] 2. 实现专业工厂 - SlimeFactory
  - [x] 2.1 创建 SlimeFactory 类
    - 创建文件 `Classes/core/factory/monster/SlimeFactory.h/.cpp`
    - 继承 BaseMonsterFactory
    - 构造函数中加载 `config/slimes` 目录配置（排除 KingSlime）
    - _Requirements: 2.1_

  - [x] 2.2 实现 SlimeFactory::create()
    - 调用基类方法附加通用组件
    - 实现 `attachSlimeMovement()` - JumpMovementComponent
    - 实现 `attachSpecialSlimeComponents()` - 处理 Umbrella/Spiked 等特殊史莱姆
    - _Requirements: 2.1, 2.8_

  - [ ]* 2.3 编写 SlimeFactory 单元测试
    - 测试所有普通史莱姆变体的创建
    - 验证组件正确附加
    - _Requirements: 2.1, 2.8_

- [x] 3. 实现专业工厂 - ZombieFactory
  - [x] 3.1 创建 ZombieFactory 类
    - 创建文件 `Classes/core/factory/monster/ZombieFactory.h/.cpp`
    - 继承 BaseMonsterFactory
    - 构造函数中加载 `config/zombies` 目录配置
    - _Requirements: 2.2_

  - [x] 3.2 实现 ZombieFactory::create()
    - 调用基类方法附加通用组件
    - 实现 `attachZombieMovement()` - WarriorMovementComponent, GroundDetectorComponent
    - _Requirements: 2.2, 2.6_

  - [ ]* 3.3 编写 ZombieFactory 单元测试
    - 测试所有僵尸变体的创建
    - _Requirements: 2.2, 2.6_

- [x] 4. 实现专业工厂 - DemonEyeFactory
  - [x] 4.1 创建 DemonEyeFactory 类
    - 创建文件 `Classes/core/factory/monster/DemonEyeFactory.h/.cpp`
    - 继承 BaseMonsterFactory
    - 构造函数中加载 `config/eyes` 目录配置
    - _Requirements: 2.3_

  - [x] 4.2 实现 DemonEyeFactory::create()
    - 调用基类方法附加通用组件
    - 实现 `attachFlyingComponents()` - DemonEyeMovementComponent
    - 配置无重力物理体
    - _Requirements: 2.3, 2.6_

  - [ ]* 4.3 编写 DemonEyeFactory 单元测试
    - 测试所有恶魔眼变体的创建
    - _Requirements: 2.3, 2.6_

- [x] 5. 实现专业工厂 - EaterFactory
  - [x] 5.1 创建 EaterFactory 类
    - 创建文件 `Classes/core/factory/monster/EaterFactory.h/.cpp`
    - 继承 BaseMonsterFactory
    - 构造函数中加载 `config/eaters` 目录配置
    - _Requirements: 2.4_

  - [x] 5.2 实现 EaterFactory::create()
    - 调用基类方法附加通用组件
    - 实现 `attachCircleFlightComponents()` - EaterOfSoulsMovementComponent
    - 根据怪物ID判断尺寸变体 (Small/Medium/Large)
    - _Requirements: 2.4, 2.6_

  - [ ]* 5.3 编写 EaterFactory 单元测试
    - 测试所有噬魂怪和血腥怪变体的创建
    - _Requirements: 2.4, 2.6_

- [x] 6. 实现专业工厂 - AntlionFactory
  - [x] 6.1 创建 AntlionFactory 类
    - 创建文件 `Classes/core/factory/monster/AntlionFactory.h/.cpp`
    - 继承 BaseMonsterFactory
    - 构造函数中加载 `config/desert/Antlion.json` 配置
    - _Requirements: 2.5_

  - [x] 6.2 实现 AntlionFactory::create()
    - 调用基类方法附加通用组件
    - 实现 `attachAntlionComponents()` - AntlionMovementComponent, AnimationStateComponent
    - 配置静止射击行为（有重力、高摩擦力、锁定旋转）
    - _Requirements: 2.5, 2.7_

  - [ ]* 6.3 编写 AntlionFactory 单元测试
    - 测试蚁狮的创建
    - _Requirements: 2.5, 2.7_

- [x] 7. 实现专业工厂 - VultureFactory
  - [x] 7.1 创建 VultureFactory 类
    - 创建文件 `Classes/core/factory/monster/VultureFactory.h/.cpp`
    - 继承 BaseMonsterFactory
    - 构造函数中加载 `config/desert/Vulture.json` 配置
    - _Requirements: 2.6_

  - [x] 7.2 实现 VultureFactory::create()
    - 调用基类方法附加通用组件
    - 实现 `attachVultureComponents()` - VultureMovementComponent, AnimationStateComponent
    - 配置飞行猛扑行为（无重力、状态驱动动画）
    - 支持 `createFlyingVulture()` 变体
    - _Requirements: 2.6, 2.7_

  - [ ]* 7.3 编写 VultureFactory 单元测试
    - 测试秃鹰的创建（包括飞行变体）
    - _Requirements: 2.6, 2.7_

- [-] 8. 实现专业工厂 - KingSlimeFactory
  - [ ] 8.1 创建 KingSlimeFactory 类
    - 创建文件 `Classes/core/factory/monster/KingSlimeFactory.h/.cpp`
    - 继承 BaseMonsterFactory
    - 构造函数中加载 `config/bosses/KingSlime.json` 配置
    - _Requirements: 2.7_

  - [ ] 8.2 实现 KingSlimeFactory::create()
    - 调用基类方法附加通用组件
    - 实现 `attachKingSlimeComponents()` - KingSlimeComponent
    - 配置 Boss 特有机制：传送、生成小史莱姆、缩放阶段变化
    - 配置更高的速度限制（velocityLimit = 1200）
    - _Requirements: 2.7, 2.8_

  - [ ]* 8.3 编写 KingSlimeFactory 单元测试
    - 测试史莱姆王的创建
    - 验证 Boss 组件正确附加
    - _Requirements: 2.7, 2.8_

- [ ] 9. Checkpoint - 确保所有专业工厂测试通过
  - 确保所有测试通过，如有问题请询问用户

- [ ] 10. 实现 ConfigCache 配置缓存
  - [ ] 10.1 创建 ConfigCache 类
    - 创建文件 `Classes/core/factory/ConfigCache.h/.cpp`
    - 实现单例模式
    - 实现 `getConfig()` - 获取配置（自动缓存）
    - 实现 `preloadDirectory()` - 预加载目录
    - 实现 `clear()` - 清空缓存
    - 实现 `isLoaded()`, `getCacheSize()`
    - _Requirements: 5.1, 5.2, 5.3, 5.4, 5.5_

  - [ ]* 10.2 编写 ConfigCache 属性测试
    - **Property 7: Config Cache Idempotence**
    - **Validates: Requirements 5.2, 5.3**
    - _Requirements: 5.2, 5.3_

- [ ] 11. 实现 MonsterEntityPool 实体池
  - [ ] 11.1 创建 MonsterEntityPool 类
    - 创建文件 `Classes/core/factory/MonsterEntityPool.h/.cpp`
    - 实现 `preallocate()` - 预分配实体
    - 实现 `acquire()` - 获取实体（池空时动态创建）
    - 实现 `release()` - 归还实体
    - 实现 `getStatistics()` - 获取池统计
    - 实现 `resetEntity()`, `activateEntity()`, `deactivateEntity()` 辅助方法
    - _Requirements: 4.1, 4.2, 4.3, 4.4, 4.5, 4.6_

  - [ ]* 11.2 编写 MonsterEntityPool 属性测试
    - **Property 5: Entity Pool Acquire-Release Invariant**
    - **Property 6: Pool Acquire Always Succeeds**
    - **Validates: Requirements 4.3, 4.4, 4.5, 4.6**
    - _Requirements: 4.3, 4.4, 4.5, 4.6_

- [ ] 12. 实现 MasterFactory 总工厂
  - [ ] 12.1 创建 MasterFactory 类
    - 创建文件 `Classes/core/factory/MasterFactory.h/.cpp`
    - 实现单例模式
    - 实现 `registerFactory()`, `unregisterFactory()`
    - 实现 `findFactory()` - 查找支持指定ID的工厂
    - _Requirements: 3.1, 3.5_

  - [ ] 12.2 实现 MasterFactory::createMonster()
    - 查找支持的工厂并委托创建
    - 未找到工厂时返回 INVALID_ENTITY 并记录日志
    - _Requirements: 3.2, 3.3, 3.4_

  - [ ] 12.3 实现批量创建和对象池集成
    - 实现 `createMonsterBatch()` - 使用缓存配置批量创建
    - 实现 `preallocateEntities()` - 委托到 MonsterEntityPool
    - 集成 ConfigCache 用于配置获取
    - _Requirements: 4.1, 5.6, 6.4_

  - [ ] 12.4 实现查询方法
    - 实现 `getAllSupportedMonsterIds()` - 聚合所有工厂支持的ID
    - 实现 `isSupported()` - 检查是否支持指定ID
    - 实现 `getConfig()` - 委托到 ConfigCache
    - _Requirements: 3.6_

  - [ ]* 12.5 编写 MasterFactory 属性测试
    - **Property 1: Factory Support Consistency**
    - **Property 2: Master Factory Delegation**
    - **Property 3: Unsupported Monster Handling**
    - **Property 4: Factory Registration Effects**
    - **Validates: Requirements 1.2, 2.7, 2.8, 3.3, 3.4, 3.5, 3.6**
    - _Requirements: 3.3, 3.4, 3.5, 3.6_

- [ ] 13. Checkpoint - 确保 MasterFactory 测试通过
  - 确保所有测试通过，如有问题请询问用户

- [ ] 14. 实现向后兼容层
  - [ ] 14.1 更新 MonsterFactory 为 MasterFactory 的包装器
    - 修改 `MonsterFactory::getInstance()` 返回 MasterFactory 的兼容接口
    - 保持 `createMonster()`, `getConfig()`, `loadConfigsFromDir()` 等方法签名不变
    - 内部委托到 MasterFactory
    - 添加 `[[deprecated]]` 属性标记旧方法
    - _Requirements: 7.1, 7.2, 7.5_

  - [ ] 14.2 初始化所有专业工厂
    - 在 MasterFactory 构造函数中注册所有专业工厂
    - 确保所有现有怪物ID都被支持
    - _Requirements: 7.3_

  - [ ]* 14.3 编写向后兼容性属性测试
    - **Property 8: Backward Compatibility**
    - 对比新旧工厂创建的实体组件
    - **Validates: Requirements 7.2, 7.3, 7.4**
    - _Requirements: 7.2, 7.3, 7.4_

- [ ] 15. 更新现有代码引用
  - [ ] 15.1 更新测试场景
    - 更新 SlimeTestScene, ZombieTestScene 等使用新工厂
    - 验证功能正常
    - _Requirements: 7.2_

  - [ ] 15.2 更新 AllComponents.h 和 AllSystems.h
    - 添加新工厂头文件引用
    - 确保编译通过
    - _Requirements: 7.1_

- [ ] 16. Final Checkpoint - 确保所有测试通过
  - 确保所有测试通过，如有问题请询问用户
  - 验证现有功能不受影响

## Notes

- 任务标记 `*` 的为可选测试任务，可跳过以加快 MVP 开发
- 每个任务引用具体需求以便追溯
- 检查点确保增量验证
- 属性测试验证通用正确性属性
- 单元测试验证具体示例和边界情况

