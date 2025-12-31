#include "SlimeFactory.h"
#include "components/AllComponents.h"
#include "systems/render/SpriteManager.h"

USING_NS_CC;

// ==================== 构造函数 ====================

SlimeFactory::SlimeFactory() {
  _monsterType = "Slime";
  
  // 加载 config/slimes 目录下的所有配置（排除 KingSlime）
  loadConfigs("config/slimes");
  
  CCLOG("SlimeFactory: Initialized with %zu slime configs", _configs.size());
}

// ==================== 创建方法 ====================

ecs::EntityId SlimeFactory::create(entt::registry &registry,
                                   const std::string &monsterId,
                                   float x, float y,
                                   cocos2d::Node *parentNode) {
  // 检查是否支持该怪物ID
  if (!supports(monsterId)) {
    CCLOG("SlimeFactory: Unsupported monster ID: %s", monsterId.c_str());
    return ecs::INVALID_ENTITY;
  }

  const MonsterConfig *configPtr = getConfig(monsterId);
  if (!configPtr) {
    CCLOG("SlimeFactory: Config not found for: %s", monsterId.c_str());
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

  // 5. 地面检测组件（史莱姆需要地面检测）
  registry.emplace<ecs::GroundDetectorComponent>(entity);

  // 6. 移动组件
  attachSlimeMovement(registry, entity, config);

  // 7. 特殊组件（缓降、投射物等）
  attachSpecialSlimeComponents(registry, entity, monsterId, config);

  ecs::EntityId entityId = entt::to_integral(entity);
  CCLOG("SlimeFactory: Created %s at (%.1f, %.1f) with EntityId %u",
        monsterId.c_str(), x, y, entityId);

  return entityId;
}

// ==================== 私有方法 ====================

void SlimeFactory::attachSlimeMovement(entt::registry &registry, entt::entity entity,
                                       const MonsterConfig &config) {
  // 史莱姆使用跳跃移动
  if (config.movement.type == "jump") {
    auto &jump = registry.emplace<ecs::JumpMovementComponent>(entity);
    jump.jumpCooldown = config.movement.jumpCooldown;
    jump.maxHorizontalImpulse = config.movement.horizontalImpulse;
    jump.maxVerticalImpulse = config.movement.verticalImpulse;
    jump.patrolImpulseRatio = config.movement.patrolImpulseRatio;
    jump.randomDirectionChangeChance = config.movement.directionChangeChance;
    jump.jumpTimer = config.movement.jumpCooldown;  // 初始化为冷却完成，准备跳跃
    jump.readyToJump = true;

    CCLOG("  Added JumpMovementComponent (cooldown=%.1f, hImpulse=%.1f, vImpulse=%.1f)",
          jump.jumpCooldown, jump.maxHorizontalImpulse, jump.maxVerticalImpulse);
  }
}

void SlimeFactory::attachSpecialSlimeComponents(entt::registry &registry, entt::entity entity,
                                                const std::string &monsterId,
                                                const MonsterConfig &config) {
  // 伞史莱姆：缓降能力
  if (monsterId.find("Umbrella") != std::string::npos || config.slowFall.enabled) {
    auto &slowFall = registry.emplace<ecs::SlowFallComponent>(entity);
    slowFall.maxFallSpeed = config.slowFall.maxFallSpeed;
    slowFall.fallDamping = config.slowFall.fallDamping;
    slowFall.horizontalDamping = config.slowFall.horizontalDamping;
    CCLOG("  Added SlowFallComponent (maxFallSpeed=%.1f, fallDamping=%.2f)",
          slowFall.maxFallSpeed, slowFall.fallDamping);
  }

  // 尖刺史莱姆：投射物攻击能力
  if (config.projectile.enabled || monsterId.find("Spiked") != std::string::npos) {
    auto &projectileAttack = registry.emplace<ecs::ProjectileAttackComponent>(entity);

    // 使用配置文件中的投射物参数
    projectileAttack.projectileSpritePath = config.projectile.spritePath;
    projectileAttack.projectileSpriteWidth = config.projectile.spriteWidth;
    projectileAttack.projectileSpriteHeight = config.projectile.spriteHeight;
    projectileAttack.projectileCount = config.projectile.count;
    projectileAttack.fireInterval = config.projectile.fireInterval;
    projectileAttack.fireRange = config.projectile.fireRange;
    projectileAttack.projectileSpeed = config.projectile.speed;
    projectileAttack.projectileDamage = config.projectile.damage;
    projectileAttack.projectileLifetime = config.projectile.lifetime;
    projectileAttack.horizontalSpread = config.projectile.horizontalSpread;
    projectileAttack.verticalImpulse = config.projectile.verticalImpulse;
    projectileAttack.useGravity = config.projectile.useGravity;

    // 使用配置文件中的减益效果参数
    projectileAttack.chillChance = config.debuff.chillChance;
    projectileAttack.chillDuration = config.debuff.chillDuration;
    projectileAttack.chillSpeedReduction = config.debuff.chillSpeedReduction;
    projectileAttack.freezeChance = config.debuff.freezeChance;
    projectileAttack.freezeDuration = config.debuff.freezeDuration;
    projectileAttack.poisonChance1 = config.debuff.poisonChance1;
    projectileAttack.poisonDuration1 = config.debuff.poisonDuration1;
    projectileAttack.poisonDamage1 = config.debuff.poisonDamage1;
    projectileAttack.poisonChance2 = config.debuff.poisonChance2;
    projectileAttack.poisonDuration2 = config.debuff.poisonDuration2;
    projectileAttack.poisonDamage2 = config.debuff.poisonDamage2;

    CCLOG("  Added ProjectileAttackComponent (sprite=%s, count=%d, damage=%.1f)",
          config.projectile.spritePath.c_str(), config.projectile.count,
          config.projectile.damage);
  }
}
