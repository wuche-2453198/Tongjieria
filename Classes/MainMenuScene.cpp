#include "MainMenuScene.h"
#include "ItemsTestScene.h"
#include "PlayerTestScene.h"
#include "IntegrationTestScene.h"
#include "WorldTest.h"
#include "core/world.h"
#include "audio/include/AudioEngine.h"

USING_NS_CC;

Scene *MainMenuScene::createScene() { return MainMenuScene::create(); }

// 错误处理辅助函数
static void problemLoading(const char *filename) {
  printf("Error while loading: %s\n", filename);
}

bool MainMenuScene::init() {
  if (!Scene::init()) {
    return false;
  }

  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 创建背景
  auto background =
      Sprite::create("bg/World refference/Forest/Forest_background_1.png");
  this->addChild(background, 0);

  if (background == nullptr) {
    problemLoading("bg/World refference/Forest/Forest_background_1.png");
    return false;
  }

  else {
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
  auto titleLabel =
      Label::createWithTTF("Terraria", "fonts/Marker Felt.ttf", 32);
  if (titleLabel != nullptr) {
    titleLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                 origin.y + visibleSize.height -
                                     titleLabel->getContentSize().height - 20));
    this->addChild(titleLabel, 1);
  }

  // 添加玩家测试按钮
  auto playerTestLabel =
      Label::createWithTTF("Player Test", "fonts/Marker Felt.ttf", 28);
  auto playerTestItem = MenuItemLabel::create(
      playerTestLabel, CC_CALLBACK_1(MainMenuScene::menuPlayerTestCallback, this));
  if (playerTestItem != nullptr) {
    playerTestItem->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                     origin.y + visibleSize.height / 2 + 50));
  }

  // 添加物品/背包测试按钮
  auto itemsTestLabel =
      Label::createWithTTF("Items Test", "fonts/Marker Felt.ttf", 28);
  auto itemsTestItem = MenuItemLabel::create(
      itemsTestLabel, CC_CALLBACK_1(MainMenuScene::menuItemsTestCallback, this));
  if (itemsTestItem != nullptr) {
    itemsTestItem->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                    origin.y + visibleSize.height / 2 + 10));
  }

  // 添加整合测试按钮
  auto integrationTestLabel =
      Label::createWithTTF("Integration Test", "fonts/Marker Felt.ttf", 28);
  auto integrationTestItem = MenuItemLabel::create(
      integrationTestLabel, CC_CALLBACK_1(MainMenuScene::menuIntegrationTestCallback, this));
  if (integrationTestItem != nullptr) {
    integrationTestItem->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                          origin.y + visibleSize.height / 2 - 30));
  }

  // 添加方块调试按钮
  auto blockDebugLabel =
      Label::createWithTTF("Block Debug", "fonts/Marker Felt.ttf", 28);
  auto blockDebugItem = MenuItemLabel::create(
      blockDebugLabel, CC_CALLBACK_1(MainMenuScene::menuBlockDebugCallback, this));
  if (blockDebugItem != nullptr) {
    blockDebugItem->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                     origin.y + visibleSize.height / 2 - 70));
  }

  // 添加方块测试按钮
  auto worldTestLabel =
      Label::createWithTTF("World Test", "fonts/Marker Felt.ttf", 28);
  auto worldTestItem = MenuItemLabel::create(
      worldTestLabel, CC_CALLBACK_1(MainMenuScene::menuWorldTestCallback, this));
  if (worldTestItem != nullptr) {
    worldTestItem->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                    origin.y + visibleSize.height / 2 - 110));
  }

  // 添加关闭按钮
  auto closeItem = MenuItemImage::create(
      "CloseNormal.png", "CloseSelected.png",
      CC_CALLBACK_1(MainMenuScene::menuCloseCallback, this));

  if (closeItem != nullptr) {
    float x =
        origin.x + visibleSize.width - closeItem->getContentSize().width / 2;
    float y = origin.y + closeItem->getContentSize().height / 2;
    closeItem->setPosition(Vec2(x, y));

    auto menu = Menu::create(playerTestItem, itemsTestItem, integrationTestItem, blockDebugItem, worldTestItem, closeItem, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);
  }

  return true;
}

void MainMenuScene::menuCloseCallback(Ref *pSender) {
  // 关闭游戏
  Director::getInstance()->end();
}



void MainMenuScene::menuPlayerTestCallback(Ref *pSender) {
  // 进入玩家测试场景
  auto playerScene = PlayerTestScene::createScene();
  Director::getInstance()->replaceScene(
      TransitionFade::create(0.5f, playerScene));
}

void MainMenuScene::menuItemsTestCallback(Ref *pSender) {
  // 进入物品/背包测试场景
  auto itemsScene = ItemsTestScene::createScene();
  Director::getInstance()->replaceScene(
      TransitionFade::create(0.5f, itemsScene));
}

void MainMenuScene::menuIntegrationTestCallback(Ref *pSender) {
  // 进入Items与Player整合测试场景
  auto integrationScene = IntegrationTestScene::createScene();
  Director::getInstance()->replaceScene(
      TransitionFade::create(0.5f, integrationScene));
}

void MainMenuScene::menuBlockDebugCallback(Ref *pSender) {
  // 进入方块调试场景
  auto blockScene = World::createScene();
  Director::getInstance()->replaceScene(
      TransitionFade::create(0.5f, blockScene));
}

void MainMenuScene::menuWorldTestCallback(Ref *pSender) {
  // 进入方块测试场景
  auto worldTestScene = WorldTest::createScene();
  Director::getInstance()->replaceScene(
      TransitionFade::create(0.5f, worldTestScene));
}
