#ifndef __EATER_CRIMERA_TEST_SCENE_H__
#define __EATER_CRIMERA_TEST_SCENE_H__

#include "cocos2d.h"
#include "components/AllComponents.h"
#include "systems/AllSystems.h"
#include <entt/entt.hpp>
#include <map>

/**
 * @class EaterCrimeraTestScene
 * @brief 噬魂怪和猩红喀迈拉测试场景
 * 
 * 测试特性：
 * - 噬魂怪（3种尺寸）
 * - 猩红喀迈拉（3种尺寸）
 * - 绕圈飞行后周期性冲向玩家
 */
class EaterCrimeraTestScene : public cocos2d::Layer {
public:
  static cocos2d::Scene *createScene();
  virtual bool init();
  virtual void update(float delta) override;
  void menuBackCallback(cocos2d::Ref *pSender);

  CREATE_FUNC(EaterCrimeraTestScene);

private:
  // ==================== ECS系统（EnTT版本） ====================
  entt::registry _registry;             // EnTT实体注册表
  ecs::SystemManagerEntt _systemManager; // EnTT System管理器

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
  void createEatersAndCrimeras();
  void setupSharedContactListener();
};

#endif // __EATER_CRIMERA_TEST_SCENE_H__
