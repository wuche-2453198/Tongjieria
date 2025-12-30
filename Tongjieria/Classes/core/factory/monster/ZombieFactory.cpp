#include "ZombieFactory.h"
#include "components/AllComponents.h"
#include "systems/render/SpriteManager.h"

USING_NS_CC;

// ==================== 构造函数 ====================

ZombieFactory::ZombieFactory() {
  _monsterType = "Zombie";
  
  // 加载 config/zombies 目录下的所有配置
  loadConfigs("config/zombies");
  
  CCLOG("ZombieFactory: Initialized with %zu zombie configs", _configs.size());
}

// ==================== 创建方法 ====================

ecs::EntityId ZombieFactory::create(entt::registry &registry,
                                    const std::string &monsterId,
                                    float x, float y,
                                    cocos2d::Node *parentNode) {
  // 检查是否支持该怪物ID
  if (!supports(monsterId)) {
    CCLOG("ZombieFactory: Unsupported monster ID: %s", monsterId.c_str());
    return ecs::INVALID_ENTITY;
  }

  const MonsterConfig *configPtr = getConfig(monsterId);
  if (!configPtr) {
    CCLOG("ZombieFactory: Config not found for: %s", monsterId.c_str());
    return ecs::INVALID_ENTITY;
  }

  const MonsterConfig &config = *configPtr;

  // 创建实体
  auto entity = registry.create();

  // ==================== 按优化顺序附加组件 ====================
  
  // 1. 核心组件（高频访问）
  attachCoreComponents(registry, entity, config, x, y);

  // 2. 物理组件
  attachPhysicsComponents(registry, entity, config);

  // 3. 渲染组件
  attachRenderComponents(registry, entity, config, parentNode);

  // 4. 战斗组件
  attachCombatComponents(registry, entity, config);

  // 5. 地面检测组件（僵尸需要地面检测）
  registry.emplace<ecs::GroundDetectorComponent>(entity);

  // 6. 移动组件
  attachZombieMovement(registry, entity, config);

  ecs::EntityId entityId = entt::to_integral(entity);
  CCLOG("ZombieFactory: Created %s at (%.1f, %.1f) with EntityId %u",
        monsterId.c_str(), x, y, entityId);

  return entityId;
}

// ==================== 私有方法 ====================

void ZombieFactory::attachZombieMovement(entt::registry &registry, entt::entity entity,
                                         const MonsterConfig &config) {
  // 僵尸使用行走移动（WarriorMovementComponent）
  if (config.movement.type == "walk") {
    auto &walk = registry.emplace<ecs::WarriorMovementComponent>(entity);
    walk.walkSpeed = config.movement.walkSpeed;
    walk.jumpForce = config.movement.jumpForce;
    walk.obstacleJumpEnabled = config.movement.obstacleJumpEnabled;
    walk.targetJumpEnabled = config.movement.targetJumpEnabled;
    walk.targetJumpReactionTime = config.movement.targetJumpReactionTime;
    walk.patrolDirectionChangeInterval = config.movement.patrolDirectionChangeInterval;
    walk.initialized = true;  // 标记为已初始化，避免初始延迟

    CCLOG("  Added WarriorMovementComponent (walkSpeed=%.1f, jumpForce=%.1f)",
          walk.walkSpeed, walk.jumpForce);
  }
}
