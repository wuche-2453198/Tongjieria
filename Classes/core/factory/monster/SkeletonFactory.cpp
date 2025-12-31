#include "SkeletonFactory.h"
#include "components/AllComponents.h"
#include "systems/render/SpriteManager.h"

USING_NS_CC;

SkeletonFactory::SkeletonFactory() {
  _monsterType = "Skeleton";

  const char *files[] = {
      "config/skeletons/Skeleton.json",
      "config/skeletons/Angry_Bones_1.json",
      "config/skeletons/Angry_Bones_2.json",
      "config/skeletons/Angry_Bones_3.json",
      "config/skeletons/Angry_Bones_4.json",
      "config/skeletons/27px-Angry_Bones_1.json",
  };

  int loaded = 0;
  for (const char *f : files) {
    if (loadSingleConfig(f)) {
      loaded++;
    }
  }

  CCLOG("SkeletonFactory: Initialized with %d skeleton configs", loaded);
}

ecs::EntityId SkeletonFactory::create(entt::registry &registry,
                                      const std::string &monsterId,
                                      float x, float y,
                                      cocos2d::Node *parentNode) {
  if (!supports(monsterId)) {
    CCLOG("SkeletonFactory: Unsupported monster ID: %s", monsterId.c_str());
    return ecs::INVALID_ENTITY;
  }

  const MonsterConfig *configPtr = getConfig(monsterId);
  if (!configPtr) {
    CCLOG("SkeletonFactory: Config not found for: %s", monsterId.c_str());
    return ecs::INVALID_ENTITY;
  }

  const MonsterConfig &config = *configPtr;

  auto entity = registry.create();

  attachCoreComponents(registry, entity, config, x, y);
  attachPhysicsComponents(registry, entity, config);
  attachRenderComponents(registry, entity, config, parentNode);
  attachCombatComponents(registry, entity, config);

  registry.emplace<ecs::GroundDetectorComponent>(entity);
  attachSkeletonComponents(registry, entity, config, monsterId);

  ecs::EntityId entityId = entt::to_integral(entity);
  CCLOG("SkeletonFactory: Created %s at (%.1f, %.1f) with EntityId %u",
        monsterId.c_str(), x, y, entityId);

  return entityId;
}

void SkeletonFactory::attachSkeletonComponents(entt::registry &registry,
                                              entt::entity entity,
                                              const MonsterConfig &config,
                                              const std::string &monsterId) {
  if (config.movement.type == "walk") {
    auto &walk = registry.emplace<ecs::WarriorMovementComponent>(entity);
    walk.walkSpeed = config.movement.walkSpeed;
    walk.jumpForce = config.movement.jumpForce;
    walk.obstacleJumpEnabled = config.movement.obstacleJumpEnabled;
    walk.targetJumpEnabled = config.movement.targetJumpEnabled;
    walk.targetJumpReactionTime = config.movement.targetJumpReactionTime;
    walk.patrolDirectionChangeInterval = config.movement.patrolDirectionChangeInterval;
    walk.initialized = true;
  }

  if (monsterId.rfind("Angry_Bones_", 0) == 0 || monsterId == "27px-Angry_Bones_1") {
    registry.emplace<ecs::PounceAttackComponent>(entity);
  }
}
