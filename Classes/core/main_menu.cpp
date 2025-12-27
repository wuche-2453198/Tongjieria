#include "main_menu.h"

cocos2d::Scene* MainMenu::createScene()
{
    auto main_menu = create();
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 60; j++)
        {
            cocos2d::Sprite* block = cocos2d::Sprite::create("TestBlock.bmp");
            cocos2d::Texture2D* texture = block->getTexture();
            texture->setAliasTexParameters();
            block->setContentSize(cocos2d::Size(10, 10));
            block->setPosition(10 * j, 10 * i);
            main_menu->addChild(block);
        }
    }
    return main_menu;
}

void MainMenu::Init()
{
    auto* camera = getCameras()[0];
    camera->setPosition(0, 0);
}
