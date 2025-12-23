#ifndef __MAIN_MENU_SCENE_H__
#define __MAIN_MENU_SCENE_H__

#include "cocos2d.h"

class MainMenuScene : public cocos2d::Scene {
public:
  static cocos2d::Scene *createScene();

  virtual bool init();

  // 关闭按钮回调
  void menuCloseCallback(cocos2d::Ref *pSender);

  // 史莱姆测试场景按钮回调
  void menuSlimeTestCallback(cocos2d::Ref *pSender);

  // 僵尸测试场景按钮回调
  void menuZombieTestCallback(cocos2d::Ref *pSender);

  // 恶魔眼测试场景按钮回调
  void menuDemonEyeTestCallback(cocos2d::Ref *pSender);

  // 史莱姆王测试场景按钮回调
  void menuKingSlimeTestCallback(cocos2d::Ref *pSender);

  // 噬魂怪和猩红喀迈拉测试场景按钮回调
  void menuEaterCrimeraTestCallback(cocos2d::Ref *pSender);

  // 沙漠测试场景按钮回调
  void menuDesertTestCallback(cocos2d::Ref *pSender);

  // implement the "static create()" method manually
  CREATE_FUNC(MainMenuScene);
};

#endif // __MAIN_MENU_SCENE_H__
