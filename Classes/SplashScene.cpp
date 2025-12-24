#include "SplashScene.h"
#include "MainMenuScene.h"
#include "audio/include/AudioEngine.h"
#include "systems/items/ItemManager.h"
#include "ui/items/InventoryLayer.h"                                                                                                              
#include "systems/items/Inventory.h" 
#include "test.h" 
USING_NS_CC;

Scene* SplashScene::createScene()
{
    return SplashScene::create();
}


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

    
    AudioEngine::play2d("music/Scott Lloyd Shelly - Title Screen.mp3", true, 0.2f);

    
    auto blackBg = LayerColor::create(Color4B(0, 0, 0, 255));
    this->addChild(blackBg, 0);

    _splashSprite = Sprite::create("bg/Startbg.png");
    if (_splashSprite == nullptr)
    {
        problemLoading("'bg/Startbg.png'");
        
        this->switchToMainMenu();
        return false;
    }
    else
    {
       
        _splashSprite->setPosition(Vec2(visibleSize.width / 2 + origin.x, 
                                        visibleSize.height / 2 + origin.y));
        
        
        float scaleX = visibleSize.width / _splashSprite->getContentSize().width;
        float scaleY = visibleSize.height / _splashSprite->getContentSize().height;
        float scale = MAX(scaleX, scaleY);
        _splashSprite->setScale(scale);
        
        
        _splashSprite->setOpacity(0);
        
        this->addChild(_splashSprite, 1);
    }

      this->fadeInAndOut();
      
   
    DebugItemManager();
    DebugInventoryBasic();
    DebugInventoryMoveSwap();
    DebugInventoryEvent();
    return true;
}

void SplashScene::fadeInAndOut()
{
    
    auto fadeIn = FadeIn::create(2.0f);
    
   
    auto delay = DelayTime::create(4.0f);
    
    
    auto fadeOut = FadeOut::create(1.0f);
    
    
    auto callback = CallFunc::create(CC_CALLBACK_0(SplashScene::switchToMainMenu, this));
    
    
    auto sequence = Sequence::create(fadeIn, delay, fadeOut, callback, nullptr);
    
    
    _splashSprite->runAction(sequence);
}

void SplashScene::switchToMainMenu()
{
    
    auto mainMenuScene = MainMenuScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, mainMenuScene));
}
