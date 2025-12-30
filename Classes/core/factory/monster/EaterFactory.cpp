#include "EaterFactory.h"
#include "components/AllComponents.h"
#include "systems/render/SpriteManager.h"

USING_NS_CC;

// ==================== 构造函数 ====================

EaterFactory::EaterFactory() {
  _monsterType = "EaterOfSouls";
  
  // 加载 config/eaters 目录下的所有配置
  loadConfigs("config/eaters");
  
  CCLOG("EaterFactory: Initialized with %zu eater configs", _configs.size());
}

// ==================== 创建方法 ====================

ecs::EntityId EaterFactory::create(entt::registry &registry,
                                   const std::string &monsterId,
                                   float x, float y,
                                   cocos2d::Node *parentNode) {
  // 检查是否支持该怪物ID
  if (!supports(monsterId)) {
    CCLOG("EaterFactory: Unsupported monster ID: %s", monsterId.c_str());
    return ecs::INVALID_ENTITY;
  }

  const MonsterConfig *configPtr = getConfig(monsterId);
  if (!configPtr) {
    CCLOG("EaterFactory: Config not found for: %s", monsterId.c_str());
    return ecs::INVALID_ENTITY;
  }

  const MonsterConfig &config = *configPtr;

  // 创建实体
  auto entity = registry.create();

  // ==================== 按优化顺序附加组件 ====================
  
  // 1. 核心组件（高频访问）
  attachCoreComponents(registry, entity, config, x, y);

  // 2. 物理组件（噬魂怪使用特殊的无重力矩形物理体）
  configurePhysicsForFlying(registry, entity, config);

  // 3. 渲染组件
  attachRenderComponents(registry, entity, config, parentNode);

  // 4. 战斗组件
  attachCombatComponents(registry, entity, config);

  // 5. 绕圈飞行移动组件（噬魂怪不需要地面检测）
  attachCircleFlightComponents(registry, entity, monsterId, config);

  ecs::EntityId entityId = entt::to_integral(entity);
  CCLOG("EaterFactory: Created %s at (%.1f, %.1f) with EntityId %u",
        monsterId.c_str(), x, y, entityId);

  return entityId;
}

// ==================== 私有方法 ====================

void EaterFactory::configurePhysicsForFlying(entt::registry &registry, entt::entity entity,
                                              const MonsterConfig &config) {
  // 噬魂怪使用矩形物理体，无重力，有弹性
  auto &physics = registry.emplace<ecs::PhysicsBodyComponent>(entity);
  
  // 物理体大小需要考虑精灵的缩放因子
  float scaledWidth = config.physics.bodyWidth * config.display.scale;
  float scaledHeight = config.physics.bodyHeight * config.display.scale;
  float scaledHeightOffset = config.physics.bodyHeightOffset * config.display.scale;
  
  // 矩形物理体
  physics.shape = ecs::PhysicsBodyComponent::BodyShape::Box;
  physics.width = scaledWidth;
  physics.height = scaledHeight;
  physics.offset = Vec2(0, scaledHeightOffset);
  
  // 物理属性
  physics.density = config.physics.mass;
  physics.restitution = config.physics.restitution;
  physics.friction = config.physics.friction;
  
  // 无重力飞行
  physics.gravityEnabled = false;
  physics.velocityLimit = 500.0f;
  physics.linearDamping = 0.05f;
  physics.angularDamping = 0.3f;
  
  // 碰撞设置
  physics.dynamic = true;
  physics.rotationEnabled = false;
  physics.categoryBitmask = 0x0002;
  physics.contactTestBitmask = 0xFFFFFFFF;
  physics.collisionBitmask = 0x0001 | 0x0008;  // 只与地形/玩家接触
  physics.group = config.physics.collisionGroup;

  CCLOG("  Added PhysicsBodyComponent (box, %.1fx%.1f, no gravity)",
        physics.width, physics.height);
}

void EaterFactory::attachCircleFlightComponents(entt::registry &registry, entt::entity entity,
                                                 const std::string &monsterId,
                                                 const MonsterConfig &config) {
  // 根据怪物ID判断尺寸变体
  ecs::EaterOfSoulsMovementComponent::SizeVariant variant = 
      ecs::EaterOfSoulsMovementComponent::MEDIUM;
  
  if (monsterId.find("Small") != std::string::npos) {
    variant = ecs::EaterOfSoulsMovementComponent::SMALL;
  } else if (monsterId.find("Large") != std::string::npos) {
    variant = ecs::EaterOfSoulsMovementComponent::LARGE;
  }
  
  // 使用带变体的构造函数创建组件
  auto &eater = registry.emplace<ecs::EaterOfSoulsMovementComponent>(entity, variant);
  
  // 可以从配置文件覆盖默认值
  if (config.movement.flySpeed > 0) {
    eater.flySpeed = config.movement.flySpeed;
  }
  if (config.movement.maxSpeed > 0) {
    eater.maxSpeed = config.movement.maxSpeed;
  }
  if (config.movement.acceleration > 0) {
    eater.acceleration = config.movement.acceleration;
  }
  if (config.movement.turnRate > 0) {
    eater.turnRate = config.movement.turnRate;
  }

  CCLOG("  Added EaterOfSoulsMovementComponent (variant=%d, flySpeed=%.1f, maxSpeed=%.1f)",
        variant, eater.flySpeed, eater.maxSpeed);
}
