#include "KingSlimeFactory.h"
#include "components/AllComponents.h"
#include "systems/render/SpriteManager.h"

USING_NS_CC;

// ==================== 构造函数 ====================

KingSlimeFactory::KingSlimeFactory() {
  _monsterType = "KingSlime";
  
  // 加载 config/bosses/KingSlime.json 配置
  loadSingleConfig("config/bosses/KingSlime.json");
  
  CCLOG("KingSlimeFactory: Initialized with %zu boss configs", _configs.size());
}

// ==================== 创建方法 ====================

ecs::EntityId KingSlimeFactory::create(entt::registry &registry,
                                       const std::string &monsterId,
                                       float x, float y,
                                       cocos2d::Node *parentNode) {
  // 检查是否支持该怪物ID
  if (!supports(monsterId)) {
    CCLOG("KingSlimeFactory: Unsupported monster ID: %s", monsterId.c_str());
    return ecs::INVALID_ENTITY;
  }

  const MonsterConfig *configPtr = getConfig(monsterId);
  if (!configPtr) {
    CCLOG("KingSlimeFactory: Config not found for: %s", monsterId.c_str());
    return ecs::INVALID_ENTITY;
  }

  const MonsterConfig &config = *configPtr;

  // 创建实体
  auto entity = registry.create();

  // ==================== 按优化顺序附加组件 ====================
  
  // 1. 核心组件（高频访问）
  attachCoreComponents(registry, entity, config, x, y);

  // 2. 物理组件（Boss需要更高的速度限制）
  attachPhysicsComponents(registry, entity, config);
  
  // 修改物理组件的速度限制为Boss专用值
  auto &physics = registry.get<ecs::PhysicsBodyComponent>(entity);
  physics.velocityLimit = 1200.0f;  // Boss专用更高速度限制
  CCLOG("  Set velocityLimit to 1200.0f for KingSlime Boss");

  // 3. 渲染组件
  attachRenderComponents(registry, entity, config, parentNode);

  // 4. 战斗组件
  attachCombatComponents(registry, entity, config);

  // 5. 地面检测组件（史莱姆王需要地面检测）
  registry.emplace<ecs::GroundDetectorComponent>(entity);

  // 6. 移动组件
  attachKingSlimeMovement(registry, entity, config);

  // 7. Boss组件（传送、生成小史莱姆、缩放阶段变化）
  attachKingSlimeComponents(registry, entity, config);

  // 更新渲染组件的缩放（根据KingSlimeComponent的初始缩放）
  auto &render = registry.get<ecs::RenderComponent>(entity);
  auto &kingSlime = registry.get<ecs::KingSlimeComponent>(entity);
  render.scale = kingSlime.currentScale;

  ecs::EntityId entityId = entt::to_integral(entity);
  CCLOG("KingSlimeFactory: Created %s at (%.1f, %.1f) with EntityId %u",
        monsterId.c_str(), x, y, entityId);

  return entityId;
}

// ==================== 私有方法 ====================

void KingSlimeFactory::attachKingSlimeMovement(entt::registry &registry, entt::entity entity,
                                               const MonsterConfig &config) {
  // 史莱姆王使用跳跃移动
  if (config.movement.type == "jump") {
    auto &jump = registry.emplace<ecs::JumpMovementComponent>(entity);
    jump.jumpCooldown = config.movement.jumpCooldown;
    jump.maxHorizontalImpulse = config.movement.horizontalImpulse;
    jump.maxVerticalImpulse = config.movement.verticalImpulse;
    jump.patrolImpulseRatio = config.movement.patrolImpulseRatio;
    jump.randomDirectionChangeChance = config.movement.directionChangeChance;
    jump.jumpTimer = config.movement.jumpCooldown;  // 初始化为冷却完成，准备跳跃
    jump.readyToJump = true;

    CCLOG("  Added JumpMovementComponent for KingSlime (cooldown=%.1f, hImpulse=%.1f, vImpulse=%.1f)",
          jump.jumpCooldown, jump.maxHorizontalImpulse, jump.maxVerticalImpulse);
  }
}

void KingSlimeFactory::attachKingSlimeComponents(entt::registry &registry, entt::entity entity,
                                                 const MonsterConfig &config) {
  auto &kingSlime = registry.emplace<ecs::KingSlimeComponent>(entity);
  
  // 从JSON配置中读取KingSlime特殊参数
  kingSlime.baseScale = config.kingSlime.baseScale;
  kingSlime.minScale = config.kingSlime.minScale;
  kingSlime.smallJumpsPerCycle = config.kingSlime.smallJumpsPerCycle;
  kingSlime.bigJumpMultiplier = config.kingSlime.bigJumpMultiplier;
  kingSlime.teleportInterval = config.kingSlime.teleportInterval;
  kingSlime.teleportRange = config.kingSlime.teleportRange;
  kingSlime.totalSlimesToSpawn = config.kingSlime.totalSlimesToSpawn;
  kingSlime.spawnHealthInterval = config.kingSlime.spawnHealthInterval;
  kingSlime.maxSpawnPerInterval = config.kingSlime.maxSpawnPerInterval;
  
  // 从JSON读取跳跃参数（用于大跳计算）
  kingSlime.baseHorizontalImpulse = config.movement.horizontalImpulse;
  kingSlime.baseVerticalImpulse = config.movement.verticalImpulse;
  
  // 初始化缩放：根据满血状态（healthPercent = 1.0）计算
  // 这样确保满血时的缩放是正确的
  kingSlime.updateStage(1.0f);  // 满血时调用updateStage
  
  CCLOG("  Added KingSlimeComponent (baseScale=%.1f, minScale=%.1f, bigJumpMult=%.1f)",
        kingSlime.baseScale, kingSlime.minScale, kingSlime.bigJumpMultiplier);
  CCLOG("    Teleport: interval=%.1f, range=%.1f",
        kingSlime.teleportInterval, kingSlime.teleportRange);
  CCLOG("    Spawning: total=%d slimes, healthInterval=%.3f, maxPerInterval=%d",
        kingSlime.totalSlimesToSpawn, kingSlime.spawnHealthInterval, kingSlime.maxSpawnPerInterval);
}
