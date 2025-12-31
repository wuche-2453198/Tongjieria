#ifndef __MAIN_MENU_SCENE_H__
#define __MAIN_MENU_SCENE_H__

#include "cocos2d.h"

class MainMenuScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init();
    CREATE_FUNC(MainMenuScene);

    void menuNewGameCallback(cocos2d::Ref* pSender);
    void menuLoadCallback(cocos2d::Ref* pSender);
    void menuSetGameCallback(cocos2d::Ref* pSender);
    void menuCloseCallback(cocos2d::Ref* pSender);

private:
    // 存档相关函数
    void createNewGame(int slotNumber);
    void loadGameFromSlot(int slotNumber);
};

#endif // __MAIN_MENU_SCENE_H__
