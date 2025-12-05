#include "GameScene.h"
#include "BlueSlime.h"
#include "GreenSlime.h"
#include "YellowSlime.h"

#include "MainMenuScene.h"
USING_NS_CC;

Scene *GameScene::createScene() {
  // 创建带物理引擎的场景
  auto scene = Scene::createWithPhysics();

  // 设置重力加速度
  auto physicsWorld = scene->getPhysicsWorld();
  physicsWorld->setGravity(Vec2(0, -980)); // 重力加速度 -980 (标准重力)

  // 调试线（可选）
  // physicsWorld->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);

  auto layer = GameScene::create();
  scene->addChild(layer);
  return scene;
}

bool GameScene::init() {
  if (!Layer::init()) {
    return false;
  }

  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 创建黑色背景
  auto background = LayerColor::create(Color4B(0, 0, 0, 255));
  this->addChild(background, 0);

  // 添加标题
  auto titleLabel = Label::createWithTTF("Game Scene - Test Area",
                                         "fonts/Marker Felt.ttf", 32);
  if (titleLabel != nullptr) {
    titleLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                 origin.y + visibleSize.height -
                                     titleLabel->getContentSize().height - 20));
    this->addChild(titleLabel, 1);
  }

  // 添加返回按钮
  auto backLabel =
      Label::createWithTTF("Back to Menu", "fonts/Marker Felt.ttf", 24);
  auto backItem = MenuItemLabel::create(
      backLabel, CC_CALLBACK_1(GameScene::menuBackCallback, this));

  if (backItem != nullptr) {
    backItem->setPosition(
        Vec2(origin.x + visibleSize.width / 2, origin.y + 50));

    auto menu = Menu::create(backItem, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);
  }

  // ==================== 创建场景边框 ====================
  // 左边界
  auto leftWall = Sprite::create();
  leftWall->setTextureRect(Rect(0, 0, 10, visibleSize.height));
  leftWall->setColor(Color3B(80, 80, 80)); // 灰色边框
  leftWall->setPosition(Vec2(origin.x + 5, origin.y + visibleSize.height / 2));
  this->addChild(leftWall, 0);

  PhysicsMaterial wallMaterial(1.0f, 0.0f, 1.0f);
  auto leftWallBody =
      PhysicsBody::createBox(leftWall->getContentSize(), wallMaterial);
  leftWallBody->setDynamic(false);
  leftWallBody->setContactTestBitmask(0xFFFFFFFF);
  leftWall->setPhysicsBody(leftWallBody);

  // 右边界
  auto rightWall = Sprite::create();
  rightWall->setTextureRect(Rect(0, 0, 10, visibleSize.height));
  rightWall->setColor(Color3B(80, 80, 80));
  rightWall->setPosition(Vec2(origin.x + visibleSize.width - 5,
                              origin.y + visibleSize.height / 2));
  this->addChild(rightWall, 0);

  auto rightWallBody =
      PhysicsBody::createBox(rightWall->getContentSize(), wallMaterial);
  rightWallBody->setDynamic(false);
  rightWallBody->setContactTestBitmask(0xFFFFFFFF);
  rightWall->setPhysicsBody(rightWallBody);

  // 上边界
  auto topWall = Sprite::create();
  topWall->setTextureRect(Rect(0, 0, visibleSize.width, 10));
  topWall->setColor(Color3B(80, 80, 80));
  topWall->setPosition(Vec2(origin.x + visibleSize.width / 2,
                            origin.y + visibleSize.height - 5));
  this->addChild(topWall, 0);

  auto topWallBody =
      PhysicsBody::createBox(topWall->getContentSize(), wallMaterial);
  topWallBody->setDynamic(false);
  topWallBody->setContactTestBitmask(0xFFFFFFFF);
  topWall->setPhysicsBody(topWallBody);
  // ========================================================

  // ==================== 创建地面平台 ====================
  // 地面平台 - 底部
  auto ground = Sprite::create();
  ground->setTextureRect(Rect(0, 0, visibleSize.width, 50)); // 全屏宽，高50像素
  ground->setColor(Color3B(139, 90, 43));                    // 棕色
  ground->setPosition(
      Vec2(origin.x + visibleSize.width / 2, origin.y + 25)); // 位于底部
  this->addChild(ground, 0);

  // 为地面添加物理体
  // PhysicsMaterial(密度, 弹性, 摩擦力)
  PhysicsMaterial groundMaterial(1.0f, 0.0f, 1.0f); // 弹性0, 高摩擦力
  auto groundBody =
      PhysicsBody::createBox(ground->getContentSize(), groundMaterial);
  groundBody->setDynamic(false); // 静态，不受重力影响
  groundBody->setContactTestBitmask(0xFFFFFFFF);
  ground->setPhysicsBody(groundBody);

  // 中间平台
  auto platform1 = Sprite::create();
  platform1->setTextureRect(Rect(0, 0, 300, 30)); // 宽300，高30
  platform1->setColor(Color3B(100, 150, 100));    // 绿色
  platform1->setPosition(Vec2(origin.x + visibleSize.width / 2 - 200,
                              origin.y + visibleSize.height / 3));
  this->addChild(platform1, 0);

  PhysicsMaterial platformMaterial(1.0f, 0.0f, 1.0f); // 弹性0, 高摩擦力
  auto platform1Body =
      PhysicsBody::createBox(platform1->getContentSize(), platformMaterial);
  platform1Body->setDynamic(false);
  platform1Body->setContactTestBitmask(0xFFFFFFFF);
  platform1->setPhysicsBody(platform1Body);

  // 右上平台
  auto platform2 = Sprite::create();
  platform2->setTextureRect(Rect(0, 0, 250, 30));
  platform2->setColor(Color3B(100, 150, 100));
  platform2->setPosition(Vec2(origin.x + visibleSize.width / 2 + 180,
                              origin.y + visibleSize.height / 2));
  this->addChild(platform2, 0);

  PhysicsMaterial platform2Material(1.0f, 0.0f, 1.0f); // 弹性0, 高摩擦力
  auto platform2Body =
      PhysicsBody::createBox(platform2->getContentSize(), platform2Material);
  platform2Body->setDynamic(false);
  platform2Body->setContactTestBitmask(0xFFFFFFFF);
  platform2->setPhysicsBody(platform2Body);

  // 创建天花板（防止史莱姆跳出屏幕）
  auto ceiling = Sprite::create();
  ceiling->setTextureRect(
      Rect(0, 0, visibleSize.width, 20)); // 全屏宽，高20像素
  ceiling->setColor(Color3B(80, 80, 80)); // 深灰色
  ceiling->setPosition(Vec2(origin.x + visibleSize.width / 2,
                            origin.y + visibleSize.height - 10)); // 位于顶部
  this->addChild(ceiling, 0);

  // 为天花板添加物理体
  PhysicsMaterial ceilingMaterial(1.0f, 0.0f, 1.0f); // 弹性0, 高摩擦力
  auto ceilingBody =
      PhysicsBody::createBox(ceiling->getContentSize(), ceilingMaterial);
  ceilingBody->setDynamic(false); // 静态，不受重力影响
  ceilingBody->setContactTestBitmask(0xFFFFFFFF);
  ceiling->setPhysicsBody(ceilingBody);
  // ========================================================


  // ==================== 测试史莱姆敌人 ====================
  // 创建一个虚拟玩家目标（用于测试AI）
  _fakePlayer = Sprite::create();
  _fakePlayer->setTextureRect(Rect(0, 0, 30, 50)); // 30x50像素
  _fakePlayer->setColor(Color3B(255, 100, 100));   // 红色，代表玩家
  _fakePlayer->setPosition(Vec2(visibleSize.width / 2 + origin.x - 150,
                                origin.y + 100)); // 在地面上
  this->addChild(_fakePlayer, 1);
  _fakePlayerVisible = true; // 初始可见

  // 给虚拟玩家添加标签
  _playerLabel = Label::createWithTTF("Fake Player (WASD:Move R:Toggle)",
                                      "fonts/Marker Felt.ttf", 14);
  if (_playerLabel != nullptr) {
    _playerLabel->setPosition(
        Vec2(_fakePlayer->getPositionX(), _fakePlayer->getPositionY() + 40));
    this->addChild(_playerLabel, 1);
  }


  // 创建一个蓝史莱姆在场景中
  _blueSlime = BlueSlime::create();
  if (_blueSlime != nullptr) {
    // 将蓝史莱姆放置在右侧
    _blueSlime->setPosition(Vec2(visibleSize.width / 2 + origin.x,
                                 visibleSize.height * 0.8f + origin.y));
    this->addChild(_blueSlime, 1);

    // 设置虚拟玩家为目标，史莱姆会自动索敌
    _blueSlime->setPlayerTarget(_fakePlayer);
    // 以PATROL状态开始，等待自动索敌
    _blueSlime->changeState(Enemy::State::PATROL);
  }


  // 创建一个绿史莱姆在场景中
  _greenSlime = GreenSlime::create();
  if (_greenSlime != nullptr) {
      // 将史莱姆放置在空中，让它掉落到平台上
      _greenSlime->setPosition(Vec2(visibleSize.width / 2 + origin.x + 100,
          visibleSize.height * 0.8f + origin.y));
      this->addChild(_greenSlime, 1);

      // 设置虚拟玩家为目标，史莱姆会自动索敌
      _greenSlime->setPlayerTarget(_fakePlayer);
      // 以PATROL状态开始，等待自动索敌
      _greenSlime->changeState(Enemy::State::PATROL);
  }




  // 创建一个黄史莱姆在场景中
  _yellowSlime = YellowSlime::create();
  if (_yellowSlime != nullptr) {
      // 将史莱姆放置在空中，让它掉落到平台上
      _yellowSlime->setPosition(Vec2(visibleSize.width / 2 + origin.x + 10,
          visibleSize.height * 0.8f + origin.y));
      this->addChild(_yellowSlime, 1);

      // 设置虚拟玩家为目标，史莱姆会自动索敌
      _yellowSlime->setPlayerTarget(_fakePlayer);
      // 以PATROL状态开始，等待自动索敌
      _yellowSlime->changeState(Enemy::State::PATROL);
  }


  // ========================================================

  // 设置键盘监听
  setupKeyboardListener();

  // 开启更新
  this->scheduleUpdate();

  return true;
}

void GameScene::menuBackCallback(Ref *pSender) {
  // 返回主菜单
  auto mainMenuScene = MainMenuScene::createScene();
  Director::getInstance()->replaceScene(
      TransitionFade::create(0.5f, mainMenuScene));
}

void GameScene::setupKeyboardListener() {
  // 创建键盘事件监听器
  auto keyboardListener = EventListenerKeyboard::create();

  // 按键按下
  keyboardListener->onKeyPressed = [this](EventKeyboard::KeyCode keyCode,
                                          Event *event) {
    _keysPressed[keyCode] = true;

    // R键切换虚拟玩家显示/隐藏
    if (keyCode == EventKeyboard::KeyCode::KEY_R) {
      toggleFakePlayer();
    }
  };

  // 按键释放
  keyboardListener->onKeyReleased = [this](EventKeyboard::KeyCode keyCode,
                                           Event *event) {
    _keysPressed[keyCode] = false;
  };

  // 添加监听器
  _eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListener,
                                                           this);
}

void GameScene::update(float delta) {
  Layer::update(delta);

  // 更新虚拟玩家位置
  updateFakePlayerPosition(delta);
}

void GameScene::updateFakePlayerPosition(float delta) {
  if (!_fakePlayerVisible || _fakePlayer == nullptr) {
    return;
  }

  float moveSpeed = 200.0f; // 移动速度（像素/秒）
  Vec2 movement(0, 0);

  // WASD控制
  if (_keysPressed[EventKeyboard::KeyCode::KEY_W] ||
      _keysPressed[EventKeyboard::KeyCode::KEY_UP_ARROW]) {
    movement.y += moveSpeed * delta;
  }
  if (_keysPressed[EventKeyboard::KeyCode::KEY_S] ||
      _keysPressed[EventKeyboard::KeyCode::KEY_DOWN_ARROW]) {
    movement.y -= moveSpeed * delta;
  }
  if (_keysPressed[EventKeyboard::KeyCode::KEY_A] ||
      _keysPressed[EventKeyboard::KeyCode::KEY_LEFT_ARROW]) {
    movement.x -= moveSpeed * delta;
  }
  if (_keysPressed[EventKeyboard::KeyCode::KEY_D] ||
      _keysPressed[EventKeyboard::KeyCode::KEY_RIGHT_ARROW]) {
    movement.x += moveSpeed * delta;
  }

  // 应用移动
  if (movement.x != 0 || movement.y != 0) {
    Vec2 newPos = _fakePlayer->getPosition() + movement;

    // 限制在屏幕范围内
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    newPos.x =
        std::max(origin.x + 20.0f,
                 std::min(newPos.x, origin.x + visibleSize.width - 20.0f));
    newPos.y =
        std::max(origin.y + 20.0f,
                 std::min(newPos.y, origin.y + visibleSize.height - 20.0f));

    _fakePlayer->setPosition(newPos);

    // 更新标签位置
    if (_playerLabel != nullptr) {
      _playerLabel->setPosition(
          Vec2(_fakePlayer->getPositionX(), _fakePlayer->getPositionY() + 40));
    }
  }
}

void GameScene::toggleFakePlayer() {
  _fakePlayerVisible = !_fakePlayerVisible;

  if (_fakePlayerVisible) {
    // 显示虚拟玩家
    _fakePlayer->setVisible(true);
    if (_playerLabel != nullptr) {
      _playerLabel->setVisible(true);
    }

    // 重新设置史莱姆的目标，让它们自动索敌
    if (_greenSlime != nullptr) {
      _greenSlime->setPlayerTarget(_fakePlayer);
      // 不需要手动设置状态，update中会自动索敌
    }
    if (_blueSlime != nullptr) {
      _blueSlime->setPlayerTarget(_fakePlayer);
    }

    CCLOG("Fake Player: Visible");
  } else {
    // 隐藏虚拟玩家
    _fakePlayer->setVisible(false);
    if (_playerLabel != nullptr) {
      _playerLabel->setVisible(false);
    }

    // 移除史莱姆的目标，让它回到空闲状态
    if (_greenSlime != nullptr) {
      _greenSlime->setPlayerTarget(nullptr);
      _greenSlime->changeState(Enemy::State::PATROL);
    }
    if (_blueSlime != nullptr) {
      _blueSlime->setPlayerTarget(nullptr);
      _blueSlime->changeState(Enemy::State::PATROL);
    }

    CCLOG("Fake Player: Hidden");
  }
}
