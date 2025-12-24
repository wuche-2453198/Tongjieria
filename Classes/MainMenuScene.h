#ifndef __MAIN_MENU_SCENE_H__
#define __MAIN_MENU_SCENE_H__

#include "cocos2d.h"

class MainMenuScene : public cocos2d::Scene {
public:
  static cocos2d::Scene *createScene();

  virtual bool init();

  // 关闭按钮回调
  void menuCloseCallback(cocos2d::Ref *pSender);

  // 物品/背包测试场景按钮回调
  void menuItemsTestCallback(cocos2d::Ref *pSender);

  // 玩家测试场景按钮回调
  void menuPlayerTestCallback(cocos2d::Ref *pSender);

  // implement the "static create()" method manually
  CREATE_FUNC(MainMenuScene);
};

#endif // __MAIN_MENU_SCENE_H__
