#ifndef __BAT_FACTORY_H__
#define __BAT_FACTORY_H__

#include "BaseMonsterFactory.h"

class BatFactory : public BaseMonsterFactory {
public:
  BatFactory();

  ecs::EntityId create(entt::registry &registry,
                       const std::string &monsterId,
                       float x, float y,
                       cocos2d::Node *parentNode) override;

private:
  void attachBatComponents(entt::registry &registry, entt::entity entity,
                           const MonsterConfig &config);
};

#endif // __BAT_FACTORY_H__
