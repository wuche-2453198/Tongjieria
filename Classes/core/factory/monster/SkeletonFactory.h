#ifndef __SKELETON_FACTORY_H__
#define __SKELETON_FACTORY_H__

#include "BaseMonsterFactory.h"

class SkeletonFactory : public BaseMonsterFactory {
public:
  SkeletonFactory();

  ecs::EntityId create(entt::registry &registry,
                       const std::string &monsterId,
                       float x, float y,
                       cocos2d::Node *parentNode) override;

private:
  void attachSkeletonComponents(entt::registry &registry, entt::entity entity,
                                const MonsterConfig &config,
                                const std::string &monsterId);
};

#endif // __SKELETON_FACTORY_H__
