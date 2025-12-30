#include "DemonFactory.h"
#include "components/AllComponents.h"
#include "systems/render/SpriteManager.h"

USING_NS_CC;

DemonFactory::DemonFactory() {
  _monsterType = "Demon";
  loadSingleConfig("config/demons/Demon.json");
}

ecs::EntityId DemonFactory::create(entt::registry &registry,
                                  const std::string &monsterId,
                                  float x, float y,
                                  cocos2d::Node *parentNode) {
  if (!supports(monsterId)) {
    return ecs::INVALID_ENTITY;
  }

  const MonsterConfig *configPtr = getConfig(monsterId);
  if (!configPtr) {
    return ecs::INVALID_ENTITY;
  }

  const MonsterConfig &config = *configPtr;

  auto entity = registry.create();

  attachCoreComponents(registry, entity, config, x, y);
  configurePhysicsForFlying(registry, entity, config);
  attachRenderComponents(registry, entity, config, parentNode);
  attachCombatComponents(registry, entity, config);
  attachDemonComponents(registry, entity, config);

  return entt::to_integral(entity);
}

void DemonFactory::configurePhysicsForFlying(entt::registry &registry, entt::entity entity,
                                            const MonsterConfig &config) {
  attachPhysicsComponents(registry, entity, config);
}

void DemonFactory::attachDemonComponents(entt::registry &registry, entt::entity entity,
                                        const MonsterConfig &config) {
  if (config.movement.type == "fly") {
    auto &move = registry.emplace<ecs::DemonMovementComponent>(entity);
    move.flySpeed = config.movement.flySpeed;
    move.maxSpeed = config.movement.maxSpeed;
    move.acceleration = config.movement.acceleration;
    move.turnRate = config.movement.turnRate;
  }

  auto &attack = registry.emplace<ecs::DemonAttackComponent>(entity);
  attack.volleyCooldown = config.projectile.fireInterval;
  attack.cooldownTimer = 0.0f;
  attack.shotsPerVolley = config.projectile.count;
  attack.shotInterval = 0.2f;
  attack.inVolley = false;
  attack.shotsFired = 0;
  attack.shotTimer = 0.0f;
  attack.fireRange = config.projectile.fireRange;
  attack.projectileDamage = config.projectile.damage;
  attack.projectileLifetime = config.projectile.lifetime;
  attack.projectileMaxSpeed = config.projectile.speed;
  attack.projectileAcceleration = 1200.0f;
  attack.projectileDelay = 2.0f;
}
