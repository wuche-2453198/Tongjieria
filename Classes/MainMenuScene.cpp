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


    // 添加标题
    auto titleLabel = Label::createWithTTF("Terraria Clone - Main Menu", "fonts/Marker Felt.ttf", 32);
    if (titleLabel != nullptr)
    {
        titleLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                     origin.y + visibleSize.height - titleLabel->getContentSize().height - 20));
        this->addChild(titleLabel, 1);
    }

    // 添加提示文本
    auto hintLabel = Label::createWithTTF("Main Menu (To be implemented)", "fonts/Marker Felt.ttf", 24);
    if (hintLabel != nullptr)
    {
        hintLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                    origin.y + visibleSize.height / 2));
        this->addChild(hintLabel, 1);
    }

    // 添加关闭按钮
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(MainMenuScene::menuCloseCallback, this));

    if (closeItem != nullptr)
    {
        float x = origin.x + visibleSize.width - closeItem->getContentSize().width / 2;
        float y = origin.y + closeItem->getContentSize().height / 2;
        closeItem->setPosition(Vec2(x, y));

        auto menu = Menu::create(closeItem, nullptr);
        menu->setPosition(Vec2::ZERO);
        this->addChild(menu, 1);
    }

    return true;
}

void MainMenuScene::menuCloseCallback(Ref* pSender)
{
    // 关闭游戏
    Director::getInstance()->end();
}
