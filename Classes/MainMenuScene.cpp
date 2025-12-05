#include "MainMenuScene.h"
#include "audio/include/AudioEngine.h"

USING_NS_CC;

Scene* MainMenuScene::createScene()
{
    return MainMenuScene::create();
}

// 错误处理辅助函数
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
}

bool MainMenuScene::init()
{
    if (!Scene::init())
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 创建背景
    auto background = Sprite::create("bg/World refference/Forest/Forest_background_1.png");
    this->addChild(background, 0);

    if (background == nullptr)
    {
        problemLoading("bg/World refference/Forest/Forest_background_1.png");
        return false;
    }

    else
    {
        // 将图片放在屏幕中心
        background->setPosition(Vec2(visibleSize.width / 2 + origin.x,
            visibleSize.height / 2 + origin.y));

        // 根据屏幕大小调整图片缩放比例，使其填满屏幕
        float scaleX = visibleSize.width / background->getContentSize().width;
        float scaleY = visibleSize.height / background->getContentSize().height;
        float scale = MAX(scaleX, scaleY);
        background->setScale(scale);
    }

    // “Start Game” 按钮
    auto startGameItem = MenuItemLabel::create(Label::createWithTTF("New Game", "fonts/Marker Felt.ttf", 36),
        CC_CALLBACK_1(MainMenuScene::menuNewGameCallback, this));
    if (startGameItem)
    {
        startGameItem->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height / 2));
    }
    // 创建logo精灵
    auto logo = Sprite::create("bg/Tongjierialogo.png");

    // 设置位置到屏幕中心
    logo->setPosition(Vec2(origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height / 4 * 3 ));

    // 添加到当前层
    this->addChild(logo, 1);
    // “Load Game” 按钮
    auto loadItem = MenuItemLabel::create(Label::createWithTTF("Load Game", "fonts/Marker Felt.ttf", 36),
        CC_CALLBACK_1(MainMenuScene::menuLoadCallback, this));
    if (loadItem)
    {
        loadItem->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height / 2 - startGameItem->getContentSize().height - 10));
    }

    // “Exit” 按钮
    auto closeItem = MenuItemLabel::create(Label::createWithTTF("Exit", "fonts/Marker Felt.ttf", 36),
        CC_CALLBACK_1(MainMenuScene::menuCloseCallback, this));
    if (closeItem)
    {
        closeItem->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height / 2 - loadItem->getContentSize().height - startGameItem->getContentSize().height - 20));
    }

    // 创建菜单
    auto menu = Menu::create(startGameItem, loadItem, closeItem, nullptr);
    menu->setPosition(Vec2::ZERO); // 菜单的定位会通过其子项的setPosition来控制
    this->addChild(menu, 1); // Z-order 1，在背景之上

    return true;
}
void MainMenuScene::menuNewGameCallback(Ref* pSender)
{
    // 切换到游戏场景
    auto gameScene = Scene::create();
    Director::getInstance()->replaceScene(TransitionFade::create(1.0, gameScene));
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    // 创建背景
    auto bg = Sprite::create("");//加图片
    bg->setPosition(Director::getInstance()->getVisibleSize() / 2);
    auto size = bg->getContentSize();
    bg->setScaleX(visibleSize.width / size.width);
    bg->setScaleY(visibleSize.height / size.height);
    gameScene->addChild(bg, -1);
    log("New Game button pressed!");

}
void MainMenuScene::menuLoadCallback(Ref* pSender)
{
    auto gameScene = Scene::create();
    Director::getInstance()->replaceScene(TransitionFade::create(1.0, gameScene));

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    //背景
    auto bg = Sprite::create("bg/World refference/Forest/Forest_background_1.png");
    bg->setPosition(visibleSize / 2);
    auto size = bg->getContentSize();
    bg->setScale(visibleSize.width / size.width, visibleSize.height / size.height);
    gameScene->addChild(bg, -1);  // 背景永远最底层


    //存档按钮
    for (int i = 0; i < 8; i++)
    {
        // ---- 存档背景图 ----
        auto loadoption = Sprite::create("bg/loadoption.png");

        if (!loadoption)
        {
            problemLoading("bg/loadoption.png");
            return;
        }

        float scaleX = visibleSize.width / loadoption->getContentSize().width;
        float scaleY = visibleSize.height / loadoption->getContentSize().height;
        loadoption->setScale(MAX(scaleX, scaleY) / 5);

        loadoption->setPosition(Vec2(
            visibleSize.width / 3 * (1 + i % 2) + origin.x ,
            visibleSize.height / 4 * (i / 2) +origin.y + 80));

        gameScene->addChild(loadoption, 1); 


        // 存档信息文字
        auto loadLabel = Label::createWithTTF("Save Slot", "fonts/Marker Felt.ttf", 24);//把这里改成具体进入存档，现在只是个标签
        loadLabel->setPosition(loadoption->getPosition());
        gameScene->addChild(loadLabel, 2);  
    }
    //退出按钮
    auto backLabel = Label::createWithTTF("Back", "fonts/Marker Felt.ttf", 36);
    backLabel->setPosition(Vec2(100, visibleSize.height - 50));
    gameScene->addChild(backLabel, 10);

    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = [=](Touch* t, Event* e) {
        if (backLabel->getBoundingBox().containsPoint(t->getLocation()))
        {
            Director::getInstance()->replaceScene(
                TransitionFade::create(0.5f, MainMenuScene::createScene())
            );
            return true;
        }
        return false;
        };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, backLabel);
}


void MainMenuScene::menuCloseCallback(Ref* pSender)
{
    // 关闭游戏
    Director::getInstance()->end();
}
