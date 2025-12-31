#ifndef __DEMON_FACTORY_H__
#define __DEMON_FACTORY_H__

#include "BaseMonsterFactory.h"

class DemonFactory : public BaseMonsterFactory {
public:
  DemonFactory();

  ecs::EntityId create(entt::registry &registry,
                       const std::string &monsterId,
                       float x, float y,
                       cocos2d::Node *parentNode) override;

private:
  void attachDemonComponents(entt::registry &registry, entt::entity entity,
                             const MonsterConfig &config);

  void configurePhysicsForFlying(entt::registry &registry, entt::entity entity,
                                 const MonsterConfig &config);
};

#endif
