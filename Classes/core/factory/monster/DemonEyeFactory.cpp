#include "DemonEyeFactory.h"
#include "components/AllComponents.h"
#include "systems/render/SpriteManager.h"

USING_NS_CC;

// ==================== 构造函数 ====================

DemonEyeFactory::DemonEyeFactory() {
  _monsterType = "DemonEye";
  
  // 加载 config/eyes 目录下的所有配置
  loadConfigs("config/eyes");
  
  CCLOG("DemonEyeFactory: Initialized with %zu demon eye configs", _configs.size());
}

// ==================== 创建方法 ====================

ecs::EntityId DemonEyeFactory::create(entt::registry &registry,
                                      const std::string &monsterId,
                                      float x, float y,
                                      cocos2d::Node *parentNode) {
  // 检查是否支持该怪物ID
  if (!supports(monsterId)) {
    CCLOG("DemonEyeFactory: Unsupported monster ID: %s", monsterId.c_str());
    return ecs::INVALID_ENTITY;
  }

  const MonsterConfig *configPtr = getConfig(monsterId);
  if (!configPtr) {
    CCLOG("DemonEyeFactory: Config not found for: %s", monsterId.c_str());
    return ecs::INVALID_ENTITY;
  }

  const MonsterConfig &config = *configPtr;

  // 创建实体
  auto entity = registry.create();

  // ==================== 按优化顺序附加组件 ====================
  
  // 1. 核心组件（高频访问）
  attachCoreComponents(registry, entity, config, x, y);

  // 2. 物理组件（恶魔眼使用特殊的无重力圆形物理体）
  configurePhysicsForFlying(registry, entity, config);

  // 3. 渲染组件
  attachRenderComponents(registry, entity, config, parentNode);

  // 4. 战斗组件
  attachCombatComponents(registry, entity, config);

  // 5. 飞行移动组件（恶魔眼不需要地面检测）
  attachFlyingComponents(registry, entity, config);

  ecs::EntityId entityId = entt::to_integral(entity);
  CCLOG("DemonEyeFactory: Created %s at (%.1f, %.1f) with EntityId %u",
        monsterId.c_str(), x, y, entityId);

  return entityId;
}

// ==================== 私有方法 ====================

void DemonEyeFactory::configurePhysicsForFlying(entt::registry &registry, entt::entity entity,
                                                 const MonsterConfig &config) {
  attachPhysicsComponents(registry, entity, config);

  auto &physics = registry.get<ecs::PhysicsBodyComponent>(entity);
  CCLOG("  Added PhysicsBodyComponent (circle, radius=%.1f, no gravity)", physics.radius);
}

void DemonEyeFactory::attachFlyingComponents(entt::registry &registry, entt::entity entity,
                                              const MonsterConfig &config) {
  // 恶魔眼使用飞行移动（DemonEyeMovementComponent）
  if (config.movement.type == "fly") {
    auto &fly = registry.emplace<ecs::DemonEyeMovementComponent>(entity);
    fly.flySpeed = config.movement.flySpeed;
    fly.maxSpeed = config.movement.maxSpeed;
    fly.acceleration = config.movement.acceleration;
    fly.turnRate = config.movement.turnRate;
    fly.wobbleAmplitude = config.movement.wobbleAmplitude;
    fly.wobbleFrequency = config.movement.wobbleFrequency;

    CCLOG("  Added DemonEyeMovementComponent (flySpeed=%.1f, maxSpeed=%.1f, turnRate=%.2f)",
          fly.flySpeed, fly.maxSpeed, fly.turnRate);
  }
}
