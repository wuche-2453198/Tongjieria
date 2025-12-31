#include "SplashScene.h"
#include "MainMenuScene.h"
#include "audio/include/AudioEngine.h"
#include <utils/audio/AudioManager.h>


USING_NS_CC;

Scene* SplashScene::createScene()
{
    return SplashScene::create();
}

// 错误处理辅助函数
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
}

bool SplashScene::init()
{
    if (!Scene::init())
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 开始播放背景音乐（循环播放）
    AudioManager::playBGM("music/Scott Lloyd Shelly - Title Screen.mp3");

    // 创建黑色背景
    auto blackBg = LayerColor::create(Color4B(0, 0, 0, 255));
    this->addChild(blackBg, 0);

    // 加载启动图片
    _splashSprite = Sprite::create("bg/Startbg.png");
    if (_splashSprite == nullptr)
    {
        problemLoading("'bg/Startbg.png'");
        // 如果加载失败，直接跳转到主菜单
        this->switchToMainMenu();
        return false;
    }
    else
    {
        // 将图片放在屏幕中心
        _splashSprite->setPosition(Vec2(visibleSize.width / 2 + origin.x, 
                                        visibleSize.height / 2 + origin.y));
        
        // 根据屏幕大小调整图片缩放比例，使其填满屏幕
        float scaleX = visibleSize.width / _splashSprite->getContentSize().width;
        float scaleY = visibleSize.height / _splashSprite->getContentSize().height;
        float scale = MAX(scaleX, scaleY);
        _splashSprite->setScale(scale);
        
        // 初始时设置为完全透明
        _splashSprite->setOpacity(0);
        
        this->addChild(_splashSprite, 1);
    }

    // 开始淡入淡出动画
    this->fadeInAndOut();

    return true;
}

void SplashScene::fadeInAndOut()
{
    // 创建淡入动画（2秒）
    auto fadeIn = FadeIn::create(2.0f);
    
    // 创建停留动画（4秒）
    auto delay = DelayTime::create(4.0f);
    
    // 创建淡出动画（1秒）
    auto fadeOut = FadeOut::create(1.0f);
    
    // 创建切换场景的回调
    auto callback = CallFunc::create(CC_CALLBACK_0(SplashScene::switchToMainMenu, this));
    
    // 组合动画序列：淡入 -> 停留 -> 淡出 -> 切换场景
    auto sequence = Sequence::create(fadeIn, delay, fadeOut, callback, nullptr);
    
    // 执行动画
    _splashSprite->runAction(sequence);
}

void SplashScene::switchToMainMenu()
{
    // 切换到主菜单场景
    auto mainMenuScene = MainMenuScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, mainMenuScene));
}
