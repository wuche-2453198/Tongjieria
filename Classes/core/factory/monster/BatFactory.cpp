#include "BatFactory.h"
#include "components/AllComponents.h"
#include "systems/render/SpriteManager.h"

USING_NS_CC;

BatFactory::BatFactory() {
  _monsterType = "Bat";

  const char *files[] = {
      "config/bats/Cave_Bat.json",
      "config/bats/Spore_Bat.json",
      "config/bats/Jungle_Bat.json",
      "config/bats/Hell_Bat.json",
      "config/bats/Ice_Bat.json",
  };

  int loaded = 0;
  for (const char *f : files) {
    if (loadSingleConfig(f)) {
      loaded++;
    }
  }

  CCLOG("BatFactory: Initialized with %d bat configs", loaded);
}

ecs::EntityId BatFactory::create(entt::registry &registry,
                                 const std::string &monsterId,
                                 float x, float y,
                                 cocos2d::Node *parentNode) {
  if (!supports(monsterId)) {
    CCLOG("BatFactory: Unsupported monster ID: %s", monsterId.c_str());
    return ecs::INVALID_ENTITY;
  }

  const MonsterConfig *configPtr = getConfig(monsterId);
  if (!configPtr) {
    CCLOG("BatFactory: Config not found for: %s", monsterId.c_str());
    return ecs::INVALID_ENTITY;
  }

  const MonsterConfig &config = *configPtr;

  auto entity = registry.create();

  attachCoreComponents(registry, entity, config, x, y);
  attachPhysicsComponents(registry, entity, config);
  attachRenderComponents(registry, entity, config, parentNode);
  attachCombatComponents(registry, entity, config);
  attachBatComponents(registry, entity, config);

  ecs::EntityId entityId = entt::to_integral(entity);
  CCLOG("BatFactory: Created %s at (%.1f, %.1f) with EntityId %u",
        monsterId.c_str(), x, y, entityId);

  return entityId;
}

void BatFactory::attachBatComponents(entt::registry &registry, entt::entity entity,
                                    const MonsterConfig &config) {
  auto &bat = registry.emplace<ecs::BatMovementComponent>(entity);

  if (config.movement.flySpeed > 0) {
    bat.flySpeed = config.movement.flySpeed;
  }
  if (config.movement.maxSpeed > 0) {
    bat.maxSpeed = config.movement.maxSpeed;
  }
  if (config.movement.acceleration > 0) {
    bat.acceleration = config.movement.acceleration;
  }
  if (config.movement.turnRate > 0) {
    bat.turnRate = config.movement.turnRate;
  }
  if (config.movement.wobbleAmplitude > 0) {
    bat.wobbleAmplitude = config.movement.wobbleAmplitude;
  }
  if (config.movement.wobbleFrequency > 0) {
    bat.wobbleFrequency = config.movement.wobbleFrequency;
  }
}
