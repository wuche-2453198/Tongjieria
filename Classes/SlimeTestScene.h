#ifndef __SLIME_TEST_SCENE_H__
#define __SLIME_TEST_SCENE_H__

#include "cocos2d.h"
#include "ecs/ECS.h"
#include "ecs/AllComponents.h"  // 使用拆分后的组件
#include "ecs/AllSystems.h"     // 使用拆分后的系统
#include <entt/entt.hpp>
#include <map>

/**
 * @class SlimeTestScene
 * @brief 史莱姆测试场景 - 使用ECS组件创建各种史莱姆
 */
class SlimeTestScene : public cocos2d::Layer {
public:
  static cocos2d::Scene *createScene();
  virtual bool init();
  virtual void update(float delta) override;
  void menuBackCallback(cocos2d::Ref *pSender);

  CREATE_FUNC(SlimeTestScene);

private:
  // ==================== ECS系统（EnTT版本） ====================
  entt::registry _registry;             // EnTT实体注册表
  ecs::SystemManagerEntt _systemManager; // EnTT System管理器
  ecs::ProjectileCollisionSystemEntt* _projectileCollisionSystem; // 投射物碰撞系统

  // 虚拟玩家
  cocos2d::Sprite *_fakePlayer = nullptr;
  cocos2d::Label *_playerLabel = nullptr;
  bool _fakePlayerVisible = true;
  ecs::EntityId _fakePlayerEntity = ecs::INVALID_ENTITY;

  // 共享碰撞监听器
  cocos2d::EventListenerPhysicsContact *_sharedContactListener = nullptr;

  // 按键状态
  std::map<cocos2d::EventKeyboard::KeyCode, bool> _keysPressed;

  void setupKeyboardListener();
  void updateFakePlayerPosition(float delta);
  void toggleFakePlayer();
  void setupEcsSystems();
  void createPhysicsEnvironment();
  void createFakePlayerEntity();
  void createEcsSlime();
  void setupSharedContactListener();
};

#endif // __SLIME_TEST_SCENE_H__
