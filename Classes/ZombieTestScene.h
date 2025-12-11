#ifndef __ZOMBIE_TEST_SCENE_H__
#define __ZOMBIE_TEST_SCENE_H__

#include "cocos2d.h"
#include "ecs/ECS.h"
#include "ecs/SystemsEntt.h"
#include <entt/entt.hpp>
#include <map>

/**
 * @class ZombieTestScene
 * @brief 僵尸测试场景 - 使用ECS组件创建僵尸
 */
class ZombieTestScene : public cocos2d::Layer {
public:
  static cocos2d::Scene *createScene();
  virtual bool init();
  virtual void update(float delta) override;
  void menuBackCallback(cocos2d::Ref *pSender);

  CREATE_FUNC(ZombieTestScene);

private:
  // ==================== ECS系统（双版本支持） ====================
  ecs::World _world;                    // 旧版ECS（保留兼容）
  entt::registry _registry;             // EnTT版本
  ecs::SystemManagerEntt _systemManager; // EnTT System管理器
  bool _useEnttSystems = true;          // 是否使用EnTT版Systems（默认true）

  // 虚拟玩家
  cocos2d::Sprite *_fakePlayer = nullptr;
  cocos2d::Label *_playerLabel = nullptr;
  bool _fakePlayerVisible = true;
  bool _playerOnGround = false;       // 玩家是否在地面上
  int _playerGroundContactCount = 0;  // 玩家地面接触计数
  float _playerMoveSpeed = 150.0f;    // 水平移动速度
  float _playerJumpForce = 400.0f;    // 跳跃力度
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
  void createEcsZombie();
  void setupSharedContactListener();
};

#endif // __ZOMBIE_TEST_SCENE_H__
