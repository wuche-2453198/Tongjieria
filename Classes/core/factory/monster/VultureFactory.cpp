#include "VultureFactory.h"
#include "components/AllComponents.h"
#include "systems/render/SpriteManager.h"
#include "systems/core/AnimationConfigLoader.h"

USING_NS_CC;

// ==================== 构造函数 ====================

VultureFactory::VultureFactory() {
  _monsterType = "Vulture";
  
  // 加载 config/desert/Vulture.json 配置
  loadSingleConfig("config/desert/Vulture.json");
  
  CCLOG("VultureFactory: Initialized with %zu vulture configs", _configs.size());
}

// ==================== 接口实现 ====================

bool VultureFactory::supports(const std::string &monsterId) const {
  // 支持 Vulture（栖息形态）和 FlyingVulture（飞行形态）
  return monsterId == "Vulture" || monsterId == "FlyingVulture";
}

std::vector<std::string> VultureFactory::getSupportedIds() const {
  return {"Vulture", "FlyingVulture"};
}

// ==================== 创建方法 ====================

ecs::EntityId VultureFactory::create(entt::registry &registry,
                                     const std::string &monsterId,
                                     float x, float y,
                                     cocos2d::Node *parentNode) {
  // 检查是否支持该怪物ID
  if (!supports(monsterId)) {
    CCLOG("VultureFactory: Unsupported monster ID: %s", monsterId.c_str());
    return ecs::INVALID_ENTITY;
  }

  // FlyingVulture 使用专门的创建方法
  if (monsterId == "FlyingVulture") {
    return createFlyingVulture(registry, x, y, parentNode);
  }

  // 获取 Vulture 配置
  const MonsterConfig *configPtr = getConfig("Vulture");
  if (!configPtr) {
    CCLOG("VultureFactory: Config not found for Vulture");
    return ecs::INVALID_ENTITY;
  }

  const MonsterConfig &config = *configPtr;

  // 创建实体
  auto entity = registry.create();

  // ==================== 注册精灵资源 ====================
  std::string resourceId = registerSpriteResource(config);
  
  // 构建帧路径列表用于动画资源注册
  std::vector<std::string> framePaths;
  for (int i = 1; i <= config.display.frameCount; i++) {
    std::string framePath = config.display.spriteFolder + "/" +
                            config.display.spritePrefix + std::to_string(i) + ".png";
    framePaths.push_back(framePath);
  }
  
  // 注册秃鹰专用动画资源
  registerVultureAnimationResources(config, framePaths);

  // ==================== 按优化顺序附加组件 ====================
  
  // 1. 核心组件（高频访问）
  attachCoreComponents(registry, entity, config, x, y);

  // 2. 物理组件（栖息形态使用有重力的物理体）
  configurePhysicsForIdle(registry, entity, config);

  // 3. 渲染组件
  attachRenderComponents(registry, entity, config, parentNode);
  
  // 修改渲染组件使用栖息形态资源
  auto &render = registry.get<ecs::RenderComponent>(entity);
  render.spriteResourceId = "vulture_idle";

  // 4. 战斗组件
  attachCombatComponents(registry, entity, config);

  // 5. 秃鹰专用组件（移动组件 + 动画状态组件）
  attachVultureComponents(registry, entity, config, false);

  // 修改动画组件使用栖息形态
  if (registry.all_of<ecs::AnimationComponent>(entity)) {
    auto &anim = registry.get<ecs::AnimationComponent>(entity);
    anim.animationSetId = "vulture_idle";
    anim.frameTime = 0.1f;
    anim.isPlaying = true;
    anim.loop = true;
    anim.frameSequence = {6};  // 栖息形态使用第6帧
    anim.reset();
  }

  ecs::EntityId entityId = entt::to_integral(entity);
  CCLOG("VultureFactory: Created Vulture (idle) at (%.1f, %.1f) with EntityId %u",
        x, y, entityId);

  return entityId;
}

ecs::EntityId VultureFactory::createFlyingVulture(entt::registry &registry,
                                                   float x, float y,
                                                   cocos2d::Node *parentNode) {
  // 获取 Vulture 配置（FlyingVulture 复用 Vulture 配置）
  const MonsterConfig *configPtr = getConfig("Vulture");
  if (!configPtr) {
    CCLOG("VultureFactory: Config not found for Vulture");
    return ecs::INVALID_ENTITY;
  }

  const MonsterConfig &config = *configPtr;

  // 创建实体
  auto entity = registry.create();

  // ==================== 注册精灵资源 ====================
  std::string resourceId = registerSpriteResource(config);
  
  // 构建帧路径列表用于动画资源注册
  std::vector<std::string> framePaths;
  for (int i = 1; i <= config.display.frameCount; i++) {
    std::string framePath = config.display.spriteFolder + "/" +
                            config.display.spritePrefix + std::to_string(i) + ".png";
    framePaths.push_back(framePath);
  }
  
  // 注册秃鹰专用动画资源
  registerVultureAnimationResources(config, framePaths);

  // ==================== 按优化顺序附加组件 ====================
  
  // 1. 核心组件（高频访问）
  attachCoreComponents(registry, entity, config, x, y);

  // 2. 物理组件（飞行形态使用无重力的物理体）
  configurePhysicsForFlying(registry, entity, config);

  // 3. 渲染组件
  auto &render = registry.emplace<ecs::RenderComponent>(entity);
  render.spriteResourceId = "vulture_fly";
  render.scale = config.display.scale;
  render.flipX = false;
  render.visible = true;
  render.zOrder = config.display.zOrder;
  render.color = Color3B::WHITE;
  render.opacity = 255;

  auto &parentComp = registry.emplace<ecs::ParentNodeComponent>(entity);
  parentComp.parentNode = parentNode;
  parentComp.attachedToParent = false;

  // 4. 战斗组件
  attachCombatComponents(registry, entity, config);

  // 5. 秃鹰专用组件（移动组件 + 动画状态组件）- 飞行形态
  attachVultureComponents(registry, entity, config, true);

  // 添加飞行形态动画组件
  auto &anim = registry.emplace<ecs::AnimationComponent>(entity);
  anim.animationSetId = "vulture_fly";
  anim.frameTime = 0.1f;
  anim.isPlaying = true;
  anim.loop = true;
  anim.frameSequence = {1, 2, 3, 4, 5};
  anim.reset();

  ecs::EntityId entityId = entt::to_integral(entity);
  CCLOG("VultureFactory: Created FlyingVulture at (%.1f, %.1f) with EntityId %u",
        x, y, entityId);

  return entityId;
}

// ==================== 私有方法 ====================

void VultureFactory::configurePhysicsForIdle(entt::registry &registry, entt::entity entity,
                                              const MonsterConfig &config) {
  // 栖息形态：矩形物理体，有重力，落地后静止
  auto &physics = registry.emplace<ecs::PhysicsBodyComponent>(entity);
  
  // 物理体大小需要考虑精灵的缩放因子
  float scaledWidth = config.physics.bodyWidth * config.display.scale;
  float scaledHeight = config.physics.bodyHeight * config.display.scale;
  
  physics.shape = ecs::PhysicsBodyComponent::BodyShape::Box;
  physics.width = scaledWidth;
  physics.height = scaledHeight;
  physics.offset = Vec2(0, 0);
  
  // 物理属性
  physics.density = config.physics.mass;
  physics.restitution = config.physics.restitution;
  physics.friction = config.physics.friction;
  
  // 有重力，允许下落
  physics.gravityEnabled = true;
  physics.velocityLimit = 500.0f;
  physics.linearDamping = 0.1f;
  physics.angularDamping = 0.3f;
  
  // 碰撞设置
  physics.dynamic = true;
  physics.rotationEnabled = false;
  physics.categoryBitmask = 0x0002;
  physics.contactTestBitmask = 0xFFFFFFFF;
  physics.collisionBitmask = 0x0001;  // 只与地形碰撞
  physics.group = config.physics.collisionGroup;

  CCLOG("  Added PhysicsBodyComponent (box, %.1fx%.1f, gravity=true)",
        physics.width, physics.height);
}

void VultureFactory::configurePhysicsForFlying(entt::registry &registry, entt::entity entity,
                                                const MonsterConfig &config) {
  // 飞行形态：矩形物理体，无重力，低阻尼
  auto &physics = registry.emplace<ecs::PhysicsBodyComponent>(entity);
  
  // 飞行形态使用固定的物理体尺寸（70x58）
  physics.shape = ecs::PhysicsBodyComponent::BodyShape::Box;
  physics.width = 70.0f * config.display.scale;
  physics.height = 58.0f * config.display.scale;
  physics.offset = Vec2(0, 0);
  
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
  physics.collisionBitmask = 0x0001;  // 只与地形碰撞
  physics.group = config.physics.collisionGroup;

  CCLOG("  Added PhysicsBodyComponent (box, %.1fx%.1f, gravity=false)",
        physics.width, physics.height);
}

void VultureFactory::attachVultureComponents(entt::registry &registry, entt::entity entity,
                                              const MonsterConfig &config, bool isFlying) {
  // 1. 添加 VultureMovementComponent
  auto &vulture = registry.emplace<ecs::VultureMovementComponent>(entity);
  vulture.flySpeed = config.movement.flySpeed;
  vulture.maxSpeed = config.movement.maxSpeed;
  vulture.acceleration = config.movement.acceleration;
  vulture.wobbleAmplitude = config.movement.wobbleAmplitude;
  vulture.wobbleFrequency = config.movement.wobbleFrequency;
  vulture.activationRange = config.movement.activationRange;
  vulture.hoverDistance = config.movement.hoverDistance;
  vulture.dashTriggerDistance = config.movement.dashTriggerDistance;
  vulture.dashReturnDistance = config.movement.dashReturnDistance;
  
  // 设置初始状态
  if (isFlying) {
    vulture.aiState = ecs::VultureMovementComponent::HOVERING;
    vulture.activated = true;
  } else {
    vulture.aiState = ecs::VultureMovementComponent::IDLE;
    vulture.activated = false;
  }
  vulture.aiTimer = 0.0f;

  CCLOG("  Added VultureMovementComponent (flySpeed=%.1f, maxSpeed=%.1f, isFlying=%d)",
        vulture.flySpeed, vulture.maxSpeed, isFlying);

  // 2. 添加 AnimationStateComponent - 状态驱动动画
  auto &animState = registry.emplace<ecs::AnimationStateComponent>(entity);
  
  // 尝试从JSON加载动画配置
  if (ecs::AnimationConfigLoader::loadFromJson("animations/vulture_animations.json", animState)) {
    animState.enableStateDriven = true;
    CCLOG("  Loaded AnimationStateComponent for vulture from JSON");
  } else {
    // 如果JSON加载失败，手动配置动画状态
    CCLOG("  WARNING: Failed to load vulture animation config, using defaults");
    
    std::string resourceId = config.id + "_sprite";
    
    // idle 状态 - 栖息动画（单帧）
    ecs::AnimationStateData idleAnim;
    idleAnim.animationSetId = "vulture_idle";
    idleAnim.frameSequence = {6};
    idleAnim.frameTime = 0.1f;
    idleAnim.loop = true;
    idleAnim.priority = 0;
    animState.addStateAnimation("idle", idleAnim);
    
    // hovering 状态 - 盘旋动画
    ecs::AnimationStateData hoveringAnim;
    hoveringAnim.animationSetId = "vulture_fly";
    hoveringAnim.frameSequence = {1, 2, 3, 4, 5};
    hoveringAnim.frameTime = 0.1f;
    hoveringAnim.loop = true;
    hoveringAnim.priority = 1;
    animState.addStateAnimation("hovering", hoveringAnim);
    
    // dashing 状态 - 猛扑动画
    ecs::AnimationStateData dashingAnim;
    dashingAnim.animationSetId = "vulture_fly";
    dashingAnim.frameSequence = {1, 2, 3, 4, 5};
    dashingAnim.frameTime = 0.08f;  // 更快的动画
    dashingAnim.loop = true;
    dashingAnim.priority = 2;
    animState.addStateAnimation("dashing", dashingAnim);
    
    animState.enableStateDriven = true;
  }
  
  // 设置初始状态
  if (isFlying) {
    animState.defaultState = "hovering";
    animState.currentState = "hovering";
  } else {
    animState.defaultState = "idle";
    animState.currentState = "idle";
  }
  
  CCLOG("  Added AnimationStateComponent (states: idle, hovering, dashing)");
}

void VultureFactory::registerVultureAnimationResources(const MonsterConfig &config,
                                                        const std::vector<std::string> &framePaths) {
  // 注册 vulture_idle 资源（使用第6帧作为栖息形态）
  ecs::SpriteResourceDescriptor idleDesc;
  idleDesc.resourceId = "vulture_idle";
  idleDesc.framePaths = framePaths;
  idleDesc.anchorPoint = Vec2(0.5f, config.display.anchorY);
  if (framePaths.size() >= 6) {
    idleDesc.spritePath = framePaths[5];  // 第6帧（索引5）
  } else if (!framePaths.empty()) {
    idleDesc.spritePath = framePaths[0];
  }
  ecs::SpriteManager::getInstance().registerResource(idleDesc);
  
  // 注册 vulture_fly 资源（使用所有帧）
  ecs::SpriteResourceDescriptor flyDesc;
  flyDesc.resourceId = "vulture_fly";
  flyDesc.framePaths = framePaths;
  flyDesc.anchorPoint = Vec2(0.5f, config.display.anchorY);
  if (!framePaths.empty()) {
    flyDesc.spritePath = framePaths[0];
  }
  ecs::SpriteManager::getInstance().registerResource(flyDesc);
  
  CCLOG("  Registered animation resources: vulture_idle, vulture_fly");
}
