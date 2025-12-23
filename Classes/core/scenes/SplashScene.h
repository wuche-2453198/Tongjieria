#ifndef __SPLASH_SCENE_H__
#define __SPLASH_SCENE_H__

#include "cocos2d.h"

class SplashScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init();
    
    // 实现淡入淡出动画
    void fadeInAndOut();
    
    // 切换到主菜单的回调函数
    void switchToMainMenu();
    
    // implement the "static create()" method manually
    CREATE_FUNC(SplashScene);

private:
    cocos2d::Sprite* _splashSprite;
};

#endif // __SPLASH_SCENE_H__
