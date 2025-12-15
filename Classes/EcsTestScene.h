#ifndef __ECS_TEST_SCENE_H__
#define __ECS_TEST_SCENE_H__

#include "cocos2d.h"
#include "ecs/ECS.h"
#include <map>

/**
 * @class EcsTestScene
 * @brief ECS框架测试场景 - 使用ECS组件创建史莱姆
 */
class EcsTestScene : public cocos2d::Layer {
public:
  /**
   * @brief 创建场景（带物理引擎）
   */
  static cocos2d::Scene *createScene();

  /**
   * @brief 初始化场景
   */
  virtual bool init();

  /**
   * @brief 每帧更新
   */
  virtual void update(float delta) override;

  /**
   * @brief 返回主菜单回调
   */
  void menuBackCallback(cocos2d::Ref *pSender);

  CREATE_FUNC(EcsTestScene);

private:
  // ECS世界
  ecs::World _world;

  // 虚拟玩家
  cocos2d::Sprite *_fakePlayer = nullptr;
  cocos2d::Label *_playerLabel = nullptr;
  bool _fakePlayerVisible = true;
  ecs::EntityId _fakePlayerEntity = ecs::INVALID_ENTITY;

  // 共享碰撞监听器
  cocos2d::EventListenerPhysicsContact *_sharedContactListener = nullptr;

  // 按键状态
  std::map<cocos2d::EventKeyboard::KeyCode, bool> _keysPressed;

  /**
   * @brief 设置键盘监听
   */
  void setupKeyboardListener();

  /**
   * @brief 更新虚拟玩家位置
   */
  void updateFakePlayerPosition(float delta);

  /**
   * @brief 切换虚拟玩家显示/隐藏
   */
  void toggleFakePlayer();

  /**
   * @brief 初始化ECS系统
   */
  void setupEcsSystems();

  /**
   * @brief 创建场景边界和平台
   */
  void createPhysicsEnvironment();

  /**
   * @brief 创建虚拟玩家实体
   */
  void createFakePlayerEntity();

  /**
   * @brief 创建ECS史莱姆
   */
  void createEcsSlime();

  /**
   * @brief 初始化共享碰撞监听器
   */
  void setupSharedContactListener();
};

#endif // __ECS_TEST_SCENE_H__
