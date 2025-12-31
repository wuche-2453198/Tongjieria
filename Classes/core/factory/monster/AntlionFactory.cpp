#include "AntlionFactory.h"
#include "components/AllComponents.h"
#include "systems/render/SpriteManager.h"

USING_NS_CC;

// ==================== 构造函数 ====================

AntlionFactory::AntlionFactory() {
  _monsterType = "Antlion";
  
  // 加载 config/desert/Antlion.json 配置
  loadSingleConfig("config/desert/Antlion.json");
  
  CCLOG("AntlionFactory: Initialized with %zu antlion configs", _configs.size());
}

// ==================== 创建方法 ====================

ecs::EntityId AntlionFactory::create(entt::registry &registry,
                                     const std::string &monsterId,
                                     float x, float y,
                                     cocos2d::Node *parentNode) {
  // 检查是否支持该怪物ID
  if (!supports(monsterId)) {
    CCLOG("AntlionFactory: Unsupported monster ID: %s", monsterId.c_str());
    return ecs::INVALID_ENTITY;
  }

  const MonsterConfig *configPtr = getConfig(monsterId);
  if (!configPtr) {
    CCLOG("AntlionFactory: Config not found for: %s", monsterId.c_str());
    return ecs::INVALID_ENTITY;
  }

  const MonsterConfig &config = *configPtr;

  // 创建实体
  auto entity = registry.create();

  // ==================== 按优化顺序附加组件 ====================
  
  // 1. 核心组件（高频访问）
  attachCoreComponents(registry, entity, config, x, y);

  // 2. 物理组件（蚁狮使用特殊的有重力、高摩擦力物理体）
  configurePhysicsForAntlion(registry, entity, config);

  // 3. 渲染组件
  attachRenderComponents(registry, entity, config, parentNode);

  // 4. 战斗组件
  attachCombatComponents(registry, entity, config);

  // 5. 蚁狮专用组件（移动组件 + 动画状态组件）
  attachAntlionComponents(registry, entity, config);

  ecs::EntityId entityId = entt::to_integral(entity);
  CCLOG("AntlionFactory: Created %s at (%.1f, %.1f) with EntityId %u",
        monsterId.c_str(), x, y, entityId);

  return entityId;
}

// ==================== 私有方法 ====================

void AntlionFactory::configurePhysicsForAntlion(entt::registry &registry, entt::entity entity,
                                                 const MonsterConfig &config) {
  // 蚁狮使用圆形物理体，有重力，高摩擦力，锁定旋转
  auto &physics = registry.emplace<ecs::PhysicsBodyComponent>(entity);
  
  // 物理体大小需要考虑精灵的缩放因子
  float scaledWidth = config.physics.bodyWidth * config.display.scale;
  float scaledHeight = config.physics.bodyHeight * config.display.scale;
  
  // 圆形物理体（使用较小的尺寸作为半径）
  physics.shape = ecs::PhysicsBodyComponent::BodyShape::Circle;
  physics.radius = std::min(scaledWidth, scaledHeight) / 2.0f;
  physics.offset = Vec2(0, 0);
  
  // 物理属性 - 高摩擦力使蚁狮保持静止
  physics.density = config.physics.mass;
  physics.restitution = 0.0f;  // 无弹性
  physics.friction = 10.0f;    // 高摩擦力
  
  // 有重力，但高阻尼使其快速停止
  physics.gravityEnabled = true;
  physics.velocityLimit = 500.0f;
  physics.linearDamping = 5.0f;   // 高线性阻尼
  physics.angularDamping = 10.0f; // 高角阻尼
  
  // 锁定旋转 - 蚁狮不应该翻滚
  physics.rotationEnabled = false;
  
  // 碰撞设置
  physics.dynamic = true;
  physics.categoryBitmask = 0x0002;
  physics.contactTestBitmask = 0xFFFFFFFF;
  physics.collisionBitmask = 0x0001 | 0x0008;  // 只与地形/玩家接触
  physics.group = config.physics.collisionGroup;

  CCLOG("  Added PhysicsBodyComponent (circle, radius=%.1f, gravity=true, high friction)",
        physics.radius);
}

void AntlionFactory::attachAntlionComponents(entt::registry &registry, entt::entity entity,
                                              const MonsterConfig &config) {
  // 1. 添加 AntlionMovementComponent - 静止射击行为
  auto &antlion = registry.emplace<ecs::AntlionMovementComponent>(entity);
  
  // 从配置文件读取参数
  if (config.movement.detectionRange > 0) {
    antlion.detectionRange = config.movement.detectionRange;
  }
  if (config.movement.shootInterval > 0) {
    antlion.shootInterval = config.movement.shootInterval;
  }
  if (config.movement.projectileSpeed > 0) {
    antlion.projectileSpeed = config.movement.projectileSpeed;
  }
  if (config.movement.rotationSpeed > 0) {
    antlion.rotationSpeed = config.movement.rotationSpeed;
  }
  
  CCLOG("  Added AntlionMovementComponent (detectionRange=%.1f, shootInterval=%.1f, projectileSpeed=%.1f)",
        antlion.detectionRange, antlion.shootInterval, antlion.projectileSpeed);

  // 2. 添加 AnimationStateComponent - 状态驱动动画
  auto &animState = registry.emplace<ecs::AnimationStateComponent>(entity);
  
  // 获取资源ID（与渲染组件使用相同的ID）
  std::string resourceId = config.id + "_sprite";
  
  // 配置动画状态映射
  // idle 状态 - 待机动画
  ecs::AnimationStateData idleAnim;
  idleAnim.animationSetId = resourceId;
  idleAnim.frameSequence = {1, 2, 3, 4, 5};  // 使用配置中的帧序列
  idleAnim.frameTime = config.display.frameTime;
  idleAnim.loop = true;
  idleAnim.priority = 0;
  animState.addStateAnimation("idle", idleAnim);
  
  // tracking 状态 - 追踪玩家（头部朝向）
  ecs::AnimationStateData trackingAnim;
  trackingAnim.animationSetId = resourceId;
  trackingAnim.frameSequence = {1, 2, 3, 4, 5};
  trackingAnim.frameTime = config.display.frameTime * 0.8f;  // 稍快的动画
  trackingAnim.loop = true;
  trackingAnim.priority = 1;
  animState.addStateAnimation("tracking", trackingAnim);
  
  // shooting 状态 - 发射射弹
  ecs::AnimationStateData shootingAnim;
  shootingAnim.animationSetId = resourceId;
  shootingAnim.frameSequence = {1, 2, 3, 4, 5};
  shootingAnim.frameTime = config.display.frameTime * 0.5f;  // 更快的动画
  shootingAnim.loop = false;  // 射击动画不循环
  shootingAnim.priority = 2;  // 最高优先级
  animState.addStateAnimation("shooting", shootingAnim);
  
  // 设置默认状态
  animState.defaultState = "idle";
  animState.currentState = "idle";
  animState.enableStateDriven = true;
  
  CCLOG("  Added AnimationStateComponent (states: idle, tracking, shooting)");
}
