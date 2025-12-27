#include "DemonTestScene.h"
#include "core/scenes/MainMenuScene.h"
#include "core/factory/monster/MonsterMasterFactory.h"
#include "systems/physics/PhysicsContactHandler.h"

USING_NS_CC;

Scene *DemonTestScene::createScene() {
  auto scene = Scene::createWithPhysics();
  auto physicsWorld = scene->getPhysicsWorld();
  physicsWorld->setGravity(Vec2(0, -980));
  physicsWorld->setSpeed(1.0f);
  physicsWorld->setSubsteps(8);

  auto layer = DemonTestScene::create();
  scene->addChild(layer);
  return scene;
}

bool DemonTestScene::init() {
  if (!Layer::init()) {
    return false;
  }

  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  auto background = LayerColor::create(Color4B(30, 10, 20, 255));
  this->addChild(background, 0);

  auto titleLabel = Label::createWithTTF("Demon Test Scene", "fonts/Marker Felt.ttf", 32);
  if (titleLabel != nullptr) {
    titleLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                 origin.y + visibleSize.height - titleLabel->getContentSize().height - 20));
    this->addChild(titleLabel, 1);
  }

  auto infoLabel = Label::createWithTTF(
      "Demon: chase + contact damage + scythe volley (WASD/Arrows move, R toggle)",
      "fonts/Marker Felt.ttf", 16);
  if (infoLabel != nullptr) {
    infoLabel->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height - 60));
    infoLabel->setColor(Color3B(220, 220, 240));
    this->addChild(infoLabel, 1);
  }

  auto backLabel = Label::createWithTTF("Back to Menu", "fonts/Marker Felt.ttf", 24);
  auto backItem = MenuItemLabel::create(backLabel, CC_CALLBACK_1(DemonTestScene::menuBackCallback, this));
  if (backItem != nullptr) {
    backItem->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + 50));
    auto menu = Menu::create(backItem, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);
  }

  createPhysicsEnvironment();
  setupEcsSystems();
  setupSharedContactListener();
  createFakePlayerEntity();
  createDemons();
  setupKeyboardListener();
  this->scheduleUpdate();

  return true;
}

void DemonTestScene::setupEcsSystems() {
  auto &factory = MonsterMasterFactory::getInstance();
  (void)factory;

  _systemManager.setRegistry(&_registry);

  _systemManager.addSystem<ecs::AggroSystemEntt>();
  auto *demonAI = _systemManager.addSystem<ecs::DemonAISystemEntt>();
  if (demonAI) {
    demonAI->setSceneContext(this);
  }

  _systemManager.addSystem<ecs::ProjectileSystemEntt>();
  _projectileCollisionSystem = _systemManager.addSystem<ecs::ProjectileCollisionSystemEntt>();

  _systemManager.addSystem<ecs::RenderSystem>();
  _systemManager.addSystem<ecs::AnimationSystem>();
  _systemManager.addSystem<ecs::PhysicsSyncSystemEntt>();

  _systemManager.addSystem<ecs::HealthSystemEntt>();
  _systemManager.addSystem<ecs::CombatSystemEntt>();
  _systemManager.addSystem<ecs::LifetimeSystemEntt>();

  ecs::SpriteDestructionObserver::registerToRegistry(_registry);
}

void DemonTestScene::createPhysicsEnvironment() {
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  PhysicsMaterial wallMaterial(1.0f, 0.3f, 0.0f);

  auto leftWall = Sprite::create();
  leftWall->setTextureRect(Rect(0, 0, 10, visibleSize.height));
  leftWall->setColor(Color3B(70, 40, 50));
  leftWall->setPosition(Vec2(origin.x + 5, origin.y + visibleSize.height / 2));
  this->addChild(leftWall, 0);
  auto leftWallBody = PhysicsBody::createBox(leftWall->getContentSize(), wallMaterial);
  leftWallBody->setDynamic(false);
  leftWallBody->setCategoryBitmask(0x0001);
  leftWallBody->setContactTestBitmask(0xFFFFFFFF);
  leftWallBody->setCollisionBitmask(0xFFFFFFFF);
  leftWallBody->setGroup(0);
  leftWall->setPhysicsBody(leftWallBody);

  auto rightWall = Sprite::create();
  rightWall->setTextureRect(Rect(0, 0, 10, visibleSize.height));
  rightWall->setColor(Color3B(70, 40, 50));
  rightWall->setPosition(Vec2(origin.x + visibleSize.width - 5, origin.y + visibleSize.height / 2));
  this->addChild(rightWall, 0);
  auto rightWallBody = PhysicsBody::createBox(rightWall->getContentSize(), wallMaterial);
  rightWallBody->setDynamic(false);
  rightWallBody->setCategoryBitmask(0x0001);
  rightWallBody->setContactTestBitmask(0xFFFFFFFF);
  rightWallBody->setCollisionBitmask(0xFFFFFFFF);
  rightWallBody->setGroup(0);
  rightWall->setPhysicsBody(rightWallBody);

  auto topWall = Sprite::create();
  topWall->setTextureRect(Rect(0, 0, visibleSize.width, 10));
  topWall->setColor(Color3B(70, 40, 50));
  topWall->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height - 5));
  this->addChild(topWall, 0);
  auto topWallBody = PhysicsBody::createBox(topWall->getContentSize(), wallMaterial);
  topWallBody->setDynamic(false);
  topWallBody->setCategoryBitmask(0x0001);
  topWallBody->setContactTestBitmask(0xFFFFFFFF);
  topWallBody->setCollisionBitmask(0xFFFFFFFF);
  topWallBody->setGroup(0);
  topWall->setPhysicsBody(topWallBody);

  PhysicsMaterial groundMaterial(1.0f, 0.0f, 2.0f);
  auto ground = Sprite::create();
  ground->setTextureRect(Rect(0, 0, visibleSize.width, 50));
  ground->setColor(Color3B(90, 50, 60));
  ground->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + 25));
  this->addChild(ground, 0);
  auto groundBody = PhysicsBody::createBox(ground->getContentSize(), groundMaterial);
  groundBody->setDynamic(false);
  groundBody->setCategoryBitmask(0x0001);
  groundBody->setContactTestBitmask(0xFFFFFFFF);
  groundBody->setCollisionBitmask(0xFFFFFFFF);
  groundBody->setGroup(0);
  ground->setPhysicsBody(groundBody);

  auto obstacle1 = Sprite::create();
  obstacle1->setTextureRect(Rect(0, 0, 120, 60));
  obstacle1->setColor(Color3B(90, 60, 70));
  obstacle1->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2));
  this->addChild(obstacle1, 0);
  auto obstacle1Body = PhysicsBody::createBox(obstacle1->getContentSize(), wallMaterial);
  obstacle1Body->setDynamic(false);
  obstacle1Body->setCategoryBitmask(0x0001);
  obstacle1Body->setContactTestBitmask(0xFFFFFFFF);
  obstacle1Body->setCollisionBitmask(0xFFFFFFFF);
  obstacle1Body->setGroup(0);
  obstacle1->setPhysicsBody(obstacle1Body);
}

void DemonTestScene::createFakePlayerEntity() {
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  _fakePlayer = Sprite::create();
  _fakePlayer->setTextureRect(Rect(0, 0, 30, 50));
  _fakePlayer->setColor(Color3B(120, 210, 255));
  _fakePlayer->setPosition(Vec2(visibleSize.width / 2 + origin.x - 200, origin.y + 160));
  this->addChild(_fakePlayer, 1);
  _fakePlayerVisible = true;

  PhysicsMaterial playerMaterial(1.0f, 0.0f, 0.0f);
  auto playerBody = PhysicsBody::createBox(Size(26, 46), playerMaterial, Vec2(0, 0));
  playerBody->setDynamic(true);
  playerBody->setMass(1.0f);
  playerBody->setRotationEnable(false);
  playerBody->setGravityEnable(false);
  playerBody->setVelocityLimit(600.0f);
  playerBody->setCategoryBitmask(0x0004);
  playerBody->setContactTestBitmask(0xFFFFFFFF);
  playerBody->setCollisionBitmask(0xFFFFFFFF);
  playerBody->setGroup(0);
  _fakePlayer->setPhysicsBody(playerBody);

  _playerLabel = Label::createWithTTF("Player (WASD/Arrows:Move R:Toggle)", "fonts/Marker Felt.ttf", 14);
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

void DemonTestScene::createDemons() {
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  auto &factory = MonsterMasterFactory::getInstance();

  float x = origin.x + visibleSize.width / 2 + 120.0f;
  float y = origin.y + visibleSize.height / 2 + 120.0f;

  factory.createMonster(_registry, "Demon", x, y, this);
}

void DemonTestScene::setupSharedContactListener() {
  _sharedContactListener = ecs::PhysicsContactHandler::createContactListener(
      _registry,
      [this](PhysicsContact &contact, const ecs::PhysicsContactHandler::ContactInfo &info) -> bool {
        (void)info;

        auto bodyA = contact.getShapeA()->getBody();
        auto bodyB = contact.getShapeB()->getBody();
        Node *nodeA = bodyA ? bodyA->getNode() : nullptr;
        Node *nodeB = bodyB ? bodyB->getNode() : nullptr;

        ecs::EntityId entityA = nodeA ? ecs::NodeEntityMap::getInstance().findEntity(nodeA) : ecs::INVALID_ENTITY;
        ecs::EntityId entityB = nodeB ? ecs::NodeEntityMap::getInstance().findEntity(nodeB) : ecs::INVALID_ENTITY;

        ecs::ProjectileComponent *projA = nullptr;
        ecs::ProjectileComponent *projB = nullptr;

        if (entityA != ecs::INVALID_ENTITY) {
          auto entA = static_cast<entt::entity>(entityA);
          if (_registry.valid(entA)) {
            projA = _registry.try_get<ecs::ProjectileComponent>(entA);
          }
        }
        if (entityB != ecs::INVALID_ENTITY) {
          auto entB = static_cast<entt::entity>(entityB);
          if (_registry.valid(entB)) {
            projB = _registry.try_get<ecs::ProjectileComponent>(entB);
          }
        }

        if (projA || projB) {
          ecs::ProjectileComponent *proj = projA ? projA : projB;
          ecs::EntityId otherEntity = projA ? entityB : entityA;
          PhysicsBody *otherBody = projA ? bodyB : bodyA;
          entt::entity projEntity = projA ? static_cast<entt::entity>(entityA) : static_cast<entt::entity>(entityB);

          if (_projectileCollisionSystem) {
            _projectileCollisionSystem->handleProjectileCollision(proj, projEntity, otherEntity, otherBody);
          }
        }

        return true;
      },
      nullptr);

  _eventDispatcher->addEventListenerWithSceneGraphPriority(_sharedContactListener, this);
}

void DemonTestScene::menuBackCallback(Ref *pSender) {
  auto mainMenuScene = MainMenuScene::createScene();
  Director::getInstance()->replaceScene(TransitionFade::create(0.5f, mainMenuScene));
}

void DemonTestScene::setupKeyboardListener() {
  auto keyboardListener = EventListenerKeyboard::create();
  keyboardListener->onKeyPressed = [this](EventKeyboard::KeyCode keyCode, Event *event) {
    (void)event;
    _keysPressed[keyCode] = true;
    if (keyCode == EventKeyboard::KeyCode::KEY_R) {
      toggleFakePlayer();
    }
  };
  keyboardListener->onKeyReleased = [this](EventKeyboard::KeyCode keyCode, Event *event) {
    (void)event;
    _keysPressed[keyCode] = false;
  };
  _eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListener, this);
}

void DemonTestScene::update(float delta) {
  Layer::update(delta);
  updateFakePlayerPosition(delta);
  _systemManager.update(delta);
}

void DemonTestScene::updateFakePlayerPosition(float delta) {
  (void)delta;

  if (!_fakePlayerVisible || !_fakePlayer) {
    return;
  }

  auto *body = _fakePlayer->getPhysicsBody();
  if (!body) {
    return;
  }

  float moveSpeed = 280.0f;
  Vec2 velocity(0, 0);

  if (_keysPressed[EventKeyboard::KeyCode::KEY_W] || _keysPressed[EventKeyboard::KeyCode::KEY_UP_ARROW]) {
    velocity.y += moveSpeed;
  }
  if (_keysPressed[EventKeyboard::KeyCode::KEY_S] || _keysPressed[EventKeyboard::KeyCode::KEY_DOWN_ARROW]) {
    velocity.y -= moveSpeed;
  }
  if (_keysPressed[EventKeyboard::KeyCode::KEY_A] || _keysPressed[EventKeyboard::KeyCode::KEY_LEFT_ARROW]) {
    velocity.x -= moveSpeed;
  }
  if (_keysPressed[EventKeyboard::KeyCode::KEY_D] || _keysPressed[EventKeyboard::KeyCode::KEY_RIGHT_ARROW]) {
    velocity.x += moveSpeed;
  }

  body->setVelocity(velocity);

  if (_playerLabel) {
    _playerLabel->setPosition(Vec2(_fakePlayer->getPositionX(), _fakePlayer->getPositionY() + 40));
  }

  if (_fakePlayerEntity != ecs::INVALID_ENTITY) {
    auto playerEntity = static_cast<entt::entity>(_fakePlayerEntity);
    if (_registry.valid(playerEntity)) {
      auto *transform = _registry.try_get<ecs::TransformComponent>(playerEntity);
      if (transform) {
        transform->position = _fakePlayer->getPosition();
      }
    }
  }
}

void DemonTestScene::toggleFakePlayer() {
  _fakePlayerVisible = !_fakePlayerVisible;
  if (_fakePlayer) {
    _fakePlayer->setVisible(_fakePlayerVisible);
    if (auto *body = _fakePlayer->getPhysicsBody()) {
      body->setEnabled(_fakePlayerVisible);
      body->setVelocity(Vec2::ZERO);
    }
  }
  if (_playerLabel) {
    _playerLabel->setVisible(_fakePlayerVisible);
  }
}
