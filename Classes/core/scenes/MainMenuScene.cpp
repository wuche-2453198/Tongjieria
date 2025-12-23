#include "MainMenuScene.h"
#include "test/SlimeTestScene.h"
#include "test/ZombieTestScene.h"
#include "test/DemonEyeTestScene.h"
#include "test/KingSlimeTestScene.h"
#include "test/EaterCrimeraTestScene.h"
#include "test/DesertTestScene.h"
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

  // 添加史莱姆测试按钮
  auto slimeTestLabel =
      Label::createWithTTF("Slime Test", "fonts/Marker Felt.ttf", 28);
  slimeTestLabel->setColor(Color3B(100, 200, 100));
  auto slimeTestItem = MenuItemLabel::create(
      slimeTestLabel, CC_CALLBACK_1(MainMenuScene::menuSlimeTestCallback, this));

  if (slimeTestItem != nullptr) {
    slimeTestItem->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                    origin.y + visibleSize.height / 2 + 40 + 40));
  }

  // 添加僵尸测试按钮
  auto zombieTestLabel =
      Label::createWithTTF("Zombie Test", "fonts/Marker Felt.ttf", 28);
  zombieTestLabel->setColor(Color3B(200, 100, 100));
  auto zombieTestItem = MenuItemLabel::create(
      zombieTestLabel, CC_CALLBACK_1(MainMenuScene::menuZombieTestCallback, this));

  if (zombieTestItem != nullptr) {
    zombieTestItem->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                     origin.y + visibleSize.height / 2));
  }

  // 添加恶魔眼测试按钮
  auto demonEyeTestLabel =
      Label::createWithTTF("Demon Eye Test", "fonts/Marker Felt.ttf", 28);
  demonEyeTestLabel->setColor(Color3B(150, 100, 200));
  auto demonEyeTestItem = MenuItemLabel::create(
      demonEyeTestLabel, CC_CALLBACK_1(MainMenuScene::menuDemonEyeTestCallback, this));

  if (demonEyeTestItem != nullptr) {
    demonEyeTestItem->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                       origin.y + visibleSize.height / 2 - 40));
  }

  // 添加史莱姆王测试按钮
  auto kingSlimeTestLabel =
      Label::createWithTTF("King Slime Arena", "fonts/Marker Felt.ttf", 28);
  kingSlimeTestLabel->setColor(Color3B(220, 180, 100));
  auto kingSlimeTestItem = MenuItemLabel::create(
      kingSlimeTestLabel, CC_CALLBACK_1(MainMenuScene::menuKingSlimeTestCallback, this));

  if (kingSlimeTestItem != nullptr) {
    kingSlimeTestItem->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                        origin.y + visibleSize.height / 2 - 80));
  }

  // 添加噬魂怪和猩红喀迈拉测试按钮
  auto eaterCrimeraTestLabel =
      Label::createWithTTF("Eater & Crimera Test", "fonts/Marker Felt.ttf", 28);
  eaterCrimeraTestLabel->setColor(Color3B(180, 100, 150));
  auto eaterCrimeraTestItem = MenuItemLabel::create(
      eaterCrimeraTestLabel, CC_CALLBACK_1(MainMenuScene::menuEaterCrimeraTestCallback, this));

  if (eaterCrimeraTestItem != nullptr) {
    eaterCrimeraTestItem->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                           origin.y + visibleSize.height / 2 - 120));
  }

  // 添加沙漠测试按钮
  auto desertTestLabel =
      Label::createWithTTF("Desert Test", "fonts/Marker Felt.ttf", 28);
  desertTestLabel->setColor(Color3B(218, 165, 32)); // 金黄色沙漠色调
  auto desertTestItem = MenuItemLabel::create(
      desertTestLabel, CC_CALLBACK_1(MainMenuScene::menuDesertTestCallback, this));

  if (desertTestItem != nullptr) {
    desertTestItem->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                     origin.y + visibleSize.height / 2 - 160));
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

    auto menu = Menu::create(slimeTestItem, zombieTestItem, demonEyeTestItem, kingSlimeTestItem, eaterCrimeraTestItem, desertTestItem, closeItem, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);
  }

  return true;
}

void MainMenuScene::menuCloseCallback(Ref *pSender) {
  // 关闭游戏
  Director::getInstance()->end();
}

void MainMenuScene::menuSlimeTestCallback(Ref *pSender) {
  // 进入史莱姆测试场景
  auto slimeTestScene = SlimeTestScene::createScene();
  Director::getInstance()->replaceScene(
      TransitionFade::create(0.5f, slimeTestScene));
}

void MainMenuScene::menuZombieTestCallback(Ref *pSender) {
  // 进入僵尸测试场景
  auto zombieTestScene = ZombieTestScene::createScene();
  Director::getInstance()->replaceScene(
      TransitionFade::create(0.5f, zombieTestScene));
}

void MainMenuScene::menuDemonEyeTestCallback(Ref *pSender) {
  // 进入恶魔眼测试场景
  auto demonEyeTestScene = DemonEyeTestScene::createScene();
  Director::getInstance()->replaceScene(
      TransitionFade::create(0.5f, demonEyeTestScene));
}

void MainMenuScene::menuKingSlimeTestCallback(Ref *pSender) {
  // 进入史莱姆王测试场景
  auto kingSlimeTestScene = KingSlimeTestScene::createScene();
  Director::getInstance()->replaceScene(
      TransitionFade::create(0.5f, kingSlimeTestScene));
}

void MainMenuScene::menuEaterCrimeraTestCallback(Ref *pSender) {
  // 进入噬魂怪和猩红喀迈拉测试场景
  auto eaterCrimeraTestScene = EaterCrimeraTestScene::createScene();
  Director::getInstance()->replaceScene(
      TransitionFade::create(0.5f, eaterCrimeraTestScene));
}

void MainMenuScene::menuDesertTestCallback(Ref *pSender) {
  // 进入沙漠测试场景
  auto desertTestScene = DesertTestScene::createScene();
  Director::getInstance()->replaceScene(
      TransitionFade::create(0.5f, desertTestScene));
}
