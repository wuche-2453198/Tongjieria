#include "BatTestScene.h"
#include "core/scenes/MainMenuScene.h"
#include "core/factory/monster/MonsterMasterFactory.h"
#include "systems/physics/PhysicsContactHandler.h"
#include "platform/CCFileUtils.h"
#include <algorithm>
#include <cstdlib>
#include <string>

USING_NS_CC;

Scene *BatTestScene::createScene() {
  auto scene = Scene::createWithPhysics();
  auto physicsWorld = scene->getPhysicsWorld();
  physicsWorld->setGravity(Vec2(0, -980));
  physicsWorld->setSpeed(1.0f);
  physicsWorld->setSubsteps(8);

  auto layer = BatTestScene::create();
  scene->addChild(layer);
  return scene;
}

bool BatTestScene::init() {
  if (!Layer::init()) {
    return false;
  }

  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  auto background = LayerColor::create(Color4B(20, 20, 30, 255));
  this->addChild(background, 0);

  auto titleLabel = Label::createWithTTF("Bat Test Scene", "fonts/Marker Felt.ttf", 32);
  if (titleLabel != nullptr) {
    titleLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                 origin.y + visibleSize.height - titleLabel->getContentSize().height - 20));
    this->addChild(titleLabel, 1);
  }

  auto infoLabel = Label::createWithTTF(
      "Bats: intermittent chase + random wandering (WASD move player, R toggle)",
      "fonts/Marker Felt.ttf", 16);
  if (infoLabel != nullptr) {
    infoLabel->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height - 60));
    infoLabel->setColor(Color3B(200, 200, 255));
    this->addChild(infoLabel, 1);
  }

  auto backLabel = Label::createWithTTF("Back to Menu", "fonts/Marker Felt.ttf", 24);
  auto backItem = MenuItemLabel::create(backLabel, CC_CALLBACK_1(BatTestScene::menuBackCallback, this));
  if (backItem != nullptr) {
    backItem->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + 50));
    auto menu = Menu::create(backItem, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);
  }

  createPhysicsEnvironment();
  setupEcsSystems();
  createFakePlayerEntity();
  createBats();
  setupKeyboardListener();
  this->scheduleUpdate();

  return true;
}

void BatTestScene::setupEcsSystems() {
  auto &factory = MonsterMasterFactory::getInstance();
  (void)factory;

  _systemManager.setRegistry(&_registry);

  _systemManager.addSystem<ecs::AggroSystemEntt>();
  _systemManager.addSystem<ecs::BatAISystemEntt>();

  _systemManager.addSystem<ecs::RenderSystem>();
  _systemManager.addSystem<ecs::AnimationSystem>();
  _systemManager.addSystem<ecs::PhysicsSyncSystemEntt>();

  _systemManager.addSystem<ecs::HealthSystemEntt>();
  _systemManager.addSystem<ecs::CombatSystemEntt>();
  _systemManager.addSystem<ecs::LifetimeSystemEntt>();

  ecs::SpriteDestructionObserver::registerToRegistry(_registry);
}

void BatTestScene::createPhysicsEnvironment() {
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  PhysicsMaterial wallMaterial(1.0f, 0.6f, 0.0f);

  auto leftWall = Sprite::create();
  leftWall->setTextureRect(Rect(0, 0, 10, visibleSize.height));
  leftWall->setColor(Color3B(40, 40, 60));
  leftWall->setPosition(Vec2(origin.x + 5, origin.y + visibleSize.height / 2));
  this->addChild(leftWall, 0);
  auto leftWallBody = PhysicsBody::createBox(leftWall->getContentSize(), wallMaterial);
  leftWallBody->setDynamic(false);
  leftWallBody->setCategoryBitmask(0x0001);
  leftWallBody->setContactTestBitmask(0xFFFFFFFF);
  leftWallBody->setCollisionBitmask(0xFFFFFFFF);
  leftWall->setPhysicsBody(leftWallBody);

  auto rightWall = Sprite::create();
  rightWall->setTextureRect(Rect(0, 0, 10, visibleSize.height));
  rightWall->setColor(Color3B(40, 40, 60));
  rightWall->setPosition(Vec2(origin.x + visibleSize.width - 5, origin.y + visibleSize.height / 2));
  this->addChild(rightWall, 0);
  auto rightWallBody = PhysicsBody::createBox(rightWall->getContentSize(), wallMaterial);
  rightWallBody->setDynamic(false);
  rightWallBody->setCategoryBitmask(0x0001);
  rightWallBody->setContactTestBitmask(0xFFFFFFFF);
  rightWallBody->setCollisionBitmask(0xFFFFFFFF);
  rightWall->setPhysicsBody(rightWallBody);

  auto topWall = Sprite::create();
  topWall->setTextureRect(Rect(0, 0, visibleSize.width, 10));
  topWall->setColor(Color3B(40, 40, 60));
  topWall->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height - 5));
  this->addChild(topWall, 0);
  auto topWallBody = PhysicsBody::createBox(topWall->getContentSize(), wallMaterial);
  topWallBody->setDynamic(false);
  topWallBody->setCategoryBitmask(0x0001);
  topWallBody->setContactTestBitmask(0xFFFFFFFF);
  topWallBody->setCollisionBitmask(0xFFFFFFFF);
  topWall->setPhysicsBody(topWallBody);

  PhysicsMaterial groundMaterial(1.0f, 0.0f, 2.0f);
  auto ground = Sprite::create();
  ground->setTextureRect(Rect(0, 0, visibleSize.width, 50));
  ground->setColor(Color3B(50, 50, 70));
  ground->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + 25));
  this->addChild(ground, 0);
  auto groundBody = PhysicsBody::createBox(ground->getContentSize(), groundMaterial);
  groundBody->setDynamic(false);
  groundBody->setCategoryBitmask(0x0001);
  groundBody->setContactTestBitmask(0xFFFFFFFF);
  groundBody->setCollisionBitmask(0xFFFFFFFF);
  ground->setPhysicsBody(groundBody);
}

void BatTestScene::createFakePlayerEntity() {
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  _fakePlayer = Sprite::create();
  _fakePlayer->setTextureRect(Rect(0, 0, 30, 50));
  _fakePlayer->setColor(Color3B(255, 200, 100));
  _fakePlayer->setPosition(Vec2(visibleSize.width / 2 + origin.x, origin.y + 150));
  this->addChild(_fakePlayer, 1);
  _fakePlayerVisible = true;

  _playerLabel = Label::createWithTTF("Player (WASD:Move R:Toggle)", "fonts/Marker Felt.ttf", 14);
  if (_playerLabel != nullptr) {
    _playerLabel->setPosition(Vec2(_fakePlayer->getPositionX(), _fakePlayer->getPositionY() + 40));
    this->addChild(_playerLabel, 1);
  }

  auto playerEntity = _registry.create();
  auto &transform = _registry.emplace<ecs::TransformComponent>(playerEntity);
  transform.position.x = _fakePlayer->getPositionX();
  transform.position.y = _fakePlayer->getPositionY();

  auto &health = _registry.emplace<ecs::HealthComponent>(playerEntity);
  health.maxHealth = 1000.0f;
  health.currentHealth = 1000.0f;
  health.invincibleTime = 0.3f;

  _registry.emplace<ecs::PlayerTag>(playerEntity);

  _fakePlayerEntity = entt::to_integral(playerEntity);
  ecs::NodeEntityMap::getInstance().registerNode(_fakePlayer, _fakePlayerEntity);
}

void BatTestScene::createBats() {
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  auto &factory = MonsterMasterFactory::getInstance();

  const char *batTypes[] = {
      "Cave_Bat",
      "Jungle_Bat",
      "Hell_Bat",
      "Ice_Bat",
      "Spore_Bat",
  };
  const int batTypeCount = 5;

  bool hasSporeFrames = FileUtils::getInstance()->isFileExist("Picture/Minor_monster/Spore_Bat/Spore_Bat1.png");

  for (int i = 0; i < batTypeCount; i++) {
    const char *id = batTypes[i];
    if (std::string(id) == "Spore_Bat" && !hasSporeFrames) {
      continue;
    }

    float x = origin.x + 100.0f + (rand() % (int)(visibleSize.width - 200.0f));
    float y = origin.y + visibleSize.height / 2 + (rand() % (int)(visibleSize.height / 3));

    factory.createMonster(_registry, id, x, y, this);
  }

  if (!hasSporeFrames) {
    auto warnLabel = Label::createWithTTF(
        "Spore_Bat frames missing (expected Spore_Bat1-4.png). Run gif_to_png.py in Resources/picture/Minor_monster/Spore_Bat",
        "fonts/Marker Felt.ttf", 14);
    if (warnLabel) {
      warnLabel->setColor(Color3B(255, 200, 120));
      warnLabel->setDimensions(visibleSize.width - 40.0f, 0.0f);
      warnLabel->setAlignment(TextHAlignment::CENTER);
      warnLabel->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + 90));
      this->addChild(warnLabel, 1);
    }
  }
}

void BatTestScene::setupKeyboardListener() {
  auto keyboardListener = EventListenerKeyboard::create();
  keyboardListener->onKeyPressed = [this](EventKeyboard::KeyCode keyCode, Event *event) {
    _keysPressed[keyCode] = true;
    if (keyCode == EventKeyboard::KeyCode::KEY_R) toggleFakePlayer();
  };
  keyboardListener->onKeyReleased = [this](EventKeyboard::KeyCode keyCode, Event *event) {
    _keysPressed[keyCode] = false;
  };
  _eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListener, this);
}

void BatTestScene::update(float delta) {
  Layer::update(delta);
  updateFakePlayerPosition(delta);
  _systemManager.update(delta);
}

void BatTestScene::updateFakePlayerPosition(float delta) {
  if (!_fakePlayerVisible || _fakePlayer == nullptr) return;

  float moveSpeed = 250.0f;
  Vec2 movement(0, 0);

  if (_keysPressed[EventKeyboard::KeyCode::KEY_W] || _keysPressed[EventKeyboard::KeyCode::KEY_UP_ARROW])
    movement.y += moveSpeed * delta;
  if (_keysPressed[EventKeyboard::KeyCode::KEY_S] || _keysPressed[EventKeyboard::KeyCode::KEY_DOWN_ARROW])
    movement.y -= moveSpeed * delta;
  if (_keysPressed[EventKeyboard::KeyCode::KEY_A] || _keysPressed[EventKeyboard::KeyCode::KEY_LEFT_ARROW])
    movement.x -= moveSpeed * delta;
  if (_keysPressed[EventKeyboard::KeyCode::KEY_D] || _keysPressed[EventKeyboard::KeyCode::KEY_RIGHT_ARROW])
    movement.x += moveSpeed * delta;

  if (movement.x != 0 || movement.y != 0) {
    Vec2 newPos = _fakePlayer->getPosition() + movement;
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    newPos.x = std::max(origin.x + 20.0f, std::min(newPos.x, origin.x + visibleSize.width - 20.0f));
    newPos.y = std::max(origin.y + 60.0f, std::min(newPos.y, origin.y + visibleSize.height - 20.0f));
    _fakePlayer->setPosition(newPos);
    if (_playerLabel) _playerLabel->setPosition(Vec2(_fakePlayer->getPositionX(), _fakePlayer->getPositionY() + 40));

    if (_fakePlayerEntity != ecs::INVALID_ENTITY) {
      auto playerEntity = static_cast<entt::entity>(_fakePlayerEntity);
      if (_registry.valid(playerEntity)) {
        auto *transform = _registry.try_get<ecs::TransformComponent>(playerEntity);
        if (transform) {
          transform->position = newPos;
        }
      }
    }
  }
}

void BatTestScene::toggleFakePlayer() {
  _fakePlayerVisible = !_fakePlayerVisible;
  _fakePlayer->setVisible(_fakePlayerVisible);
  if (_playerLabel) _playerLabel->setVisible(_fakePlayerVisible);
}

void BatTestScene::menuBackCallback(Ref *pSender) {
  auto mainMenuScene = MainMenuScene::createScene();
  Director::getInstance()->replaceScene(TransitionFade::create(0.5f, mainMenuScene));
}
