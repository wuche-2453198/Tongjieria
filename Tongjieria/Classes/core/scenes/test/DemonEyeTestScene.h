#ifndef __DEMONEYE_TEST_SCENE_H__
#define __DEMONEYE_TEST_SCENE_H__

#include "cocos2d.h"
#include "components/AllComponents.h"
#include "systems/AllSystems.h"
#include <entt/entt.hpp>
#include <map>

/**
 * @class DemonEyeTestScene
 * @brief 恶魔眼测试场景 - 测试飞行追踪类怪物AI
 * 
 * 恶魔眼特性：
 * - 飞行追踪玩家（无重力）
 * - 缓慢转向
 * - 撞墙弧形回弹
 * - 被击退时弧形轨迹回弹
 */
class DemonEyeTestScene : public cocos2d::Layer {
public:
  static cocos2d::Scene *createScene();
  virtual bool init();
  virtual void update(float delta) override;
  void menuBackCallback(cocos2d::Ref *pSender);

  CREATE_FUNC(DemonEyeTestScene);

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
  void createDemonEyes();
  void setupSharedContactListener();
};

#endif // __DEMONEYE_TEST_SCENE_H__
