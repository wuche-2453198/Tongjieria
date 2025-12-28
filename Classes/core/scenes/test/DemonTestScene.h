#ifndef __DEMON_TEST_SCENE_H__
#define __DEMON_TEST_SCENE_H__

#include "cocos2d.h"
#include "components/AllComponents.h"
#include "systems/AllSystems.h"
#include <entt/entt.hpp>
#include <map>

class DemonTestScene : public cocos2d::Layer {
public:
  static cocos2d::Scene *createScene();
  virtual bool init();
  virtual void update(float delta) override;
  void menuBackCallback(cocos2d::Ref *pSender);

  CREATE_FUNC(DemonTestScene);

private:
  entt::registry _registry;
  ecs::SystemManagerEntt _systemManager;
  ecs::ProjectileCollisionSystemEntt *_projectileCollisionSystem = nullptr;

  cocos2d::Sprite *_fakePlayer = nullptr;
  cocos2d::Label *_playerLabel = nullptr;
  bool _fakePlayerVisible = true;
  ecs::EntityId _fakePlayerEntity = ecs::INVALID_ENTITY;

  cocos2d::EventListenerPhysicsContact *_sharedContactListener = nullptr;

  std::map<cocos2d::EventKeyboard::KeyCode, bool> _keysPressed;

  void setupKeyboardListener();
  void updateFakePlayerPosition(float delta);
  void toggleFakePlayer();
  void setupEcsSystems();
  void createPhysicsEnvironment();
  void createFakePlayerEntity();
  void createDemons();
  void setupSharedContactListener();
};

#endif
