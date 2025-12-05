#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include "cocos2d.h"
#include <map>

class GreenSlime;  // 前向声明
class BlueSlime;   // 前向声明
class YellowSlime; // 前向声明

/**
 * @class GameScene
 * @brief 游戏主场景 - 用于测试和游戏逻辑
 */
class GameScene : public cocos2d::Layer {
public:
  /**
   * 创建场景
   */
  static cocos2d::Scene *createScene();

  /**
   * 初始化场景
   */
  virtual bool init();

  /**
   * 每帧更新
   */
  virtual void update(float delta) override;

  /**
   * 返回主菜单回调
   */
  void menuBackCallback(cocos2d::Ref *pSender);

  // implement the "static create()" method manually
  CREATE_FUNC(GameScene);

private:
  cocos2d::Sprite *_fakePlayer; // 虚拟玩家精灵
  cocos2d::Label *_playerLabel; // 虚拟玩家标签
  GreenSlime *_greenSlime;      // 绿史莱姆引用
  BlueSlime *_blueSlime;        // 蓝史莱姆引用
  YellowSlime *_yellowSlime;    // 黄史莱姆引用
  bool _fakePlayerVisible;      // 虚拟玩家是否可见
  std::map<cocos2d::EventKeyboard::KeyCode, bool> _keysPressed; // 按键状态

  /**
   * 设置键盘监听
   */
  void setupKeyboardListener();

  /**
   * 更新虚拟玩家位置
   */
  void updateFakePlayerPosition(float delta);

  /**
   * 切换虚拟玩家显示/隐藏
   */
  void toggleFakePlayer();
};

#endif // __GAME_SCENE_H__
