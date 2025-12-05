#ifndef __MAIN_MENU_SCENE_H__
#define __MAIN_MENU_SCENE_H__

#include "cocos2d.h"

class MainMenuScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init();
    
    // 关闭按钮回调
    void menuCloseCallback(cocos2d::Ref* pSender);
    void menuNewGameCallback(cocos2d::Ref* pSender); // 新增开始游戏回调
    void menuLoadCallback(Ref* pSender);//新增读档游戏回调
    // implement the "static create()" method manually
    CREATE_FUNC(MainMenuScene);
};

#endif // __MAIN_MENU_SCENE_H__
