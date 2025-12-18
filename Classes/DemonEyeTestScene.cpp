#include "DemonEyeTestScene.h"
#include "MainMenuScene.h"
#include "MonsterFactory.h"
#include "ecs/SpriteComponent.h"
#include "ecs/PhysicsContactHandler.h"

USING_NS_CC;

Scene *DemonEyeTestScene::createScene()
{
  auto scene = Scene::createWithPhysics();
  auto physicsWorld = scene->getPhysicsWorld();
  physicsWorld->setGravity(Vec2(0, -980));

  // 调试物理体
  //  physicsWorld->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);

  physicsWorld->setSpeed(1.0f);
  physicsWorld->setSubsteps(8);

  auto layer = DemonEyeTestScene::create();
  scene->addChild(layer);
  return scene;
}

bool DemonEyeTestScene::init()
{
  if (!Layer::init())
  {
    return false;
  }

  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 创建背景（深紫色，夜晚氛围）
  auto background = LayerColor::create(Color4B(30, 20, 50, 255));
  this->addChild(background, 0);

  // 添加标题
  auto titleLabel = Label::createWithTTF("Demon Eye Test Scene",
                                         "fonts/Marker Felt.ttf", 32);
  if (titleLabel != nullptr)
  {
    titleLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                 origin.y + visibleSize.height -
                                     titleLabel->getContentSize().height - 20));
    this->addChild(titleLabel, 1);
  }

  // 添加说明
  auto infoLabel = Label::createWithTTF(
      "Demon Eyes: Fly, chase player, slow turning, arc bounce on walls",
      "fonts/Marker Felt.ttf", 16);
  if (infoLabel != nullptr)
  {
    infoLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                origin.y + visibleSize.height - 60));
    infoLabel->setColor(Color3B(200, 200, 255));
    this->addChild(infoLabel, 1);
  }

  // 添加返回按钮
  auto backLabel =
      Label::createWithTTF("Back to Menu", "fonts/Marker Felt.ttf", 24);
  auto backItem = MenuItemLabel::create(
      backLabel, CC_CALLBACK_1(DemonEyeTestScene::menuBackCallback, this));

  if (backItem != nullptr)
  {
    backItem->setPosition(
        Vec2(origin.x + visibleSize.width / 2, origin.y + 50));

    auto menu = Menu::create(backItem, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);
  }

  createPhysicsEnvironment();
  setupEcsSystems();
  setupSharedContactListener();
  createFakePlayerEntity();
  createDemonEyes();
  setupKeyboardListener();
  this->scheduleUpdate();

  CCLOG("DemonEyeTestScene: Initialized");
  return true;
}

void DemonEyeTestScene::setupEcsSystems()
{
  auto &factory = MonsterFactory::getInstance();
  factory.clearConfigs();  // 清空之前场景的配置
  // 加载恶魔眼配置
  factory.loadConfigsFromDir("config/eyes");

  CCLOG("========== Setting up EnTT Systems for DemonEyes ==========");

  _systemManager.setRegistry(&_registry);

  // 按优先级顺序添加Systems
  _systemManager.addSystem<ecs::AggroSystemEntt>();
  _systemManager.addSystem<ecs::DemonEyeAISystemEntt>();       // 恶魔眼AI（已包含位置同步）
  _systemManager.addSystem<ecs::MonsterAnimationSystemEntt>(); // 怪物动画
  _systemManager.addSystem<ecs::HealthSystemEntt>();
  _systemManager.addSystem<ecs::CombatSystemEntt>();
  _systemManager.addSystem<ecs::LifetimeSystemEntt>();

  CCLOG("EnTT Systems initialized: %zu systems", _systemManager.getSystemCount());
  CCLOG("==========================================");
}

void DemonEyeTestScene::createPhysicsEnvironment()
{
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 墙壁材质：零摩擦、高弹性（让恶魔眼能弹开）
  PhysicsMaterial wallMaterial(1.0f, 0.9f, 0.0f);

  // 左边界
  auto leftWall = Sprite::create();
  leftWall->setTextureRect(Rect(0, 0, 10, visibleSize.height));
  leftWall->setColor(Color3B(60, 50, 80));
  leftWall->setPosition(Vec2(origin.x + 5, origin.y + visibleSize.height / 2));
  this->addChild(leftWall, 0);
  auto leftWallBody = PhysicsBody::createBox(leftWall->getContentSize(), wallMaterial);
  leftWallBody->setDynamic(false);
  leftWallBody->setCategoryBitmask(0x0001);
  leftWallBody->setContactTestBitmask(0xFFFFFFFF);
  leftWallBody->setCollisionBitmask(0xFFFFFFFF);
  leftWallBody->setGroup(0);
  leftWall->setPhysicsBody(leftWallBody);

  // 右边界
  auto rightWall = Sprite::create();
  rightWall->setTextureRect(Rect(0, 0, 10, visibleSize.height));
  rightWall->setColor(Color3B(60, 50, 80));
  rightWall->setPosition(Vec2(origin.x + visibleSize.width - 5,
                              origin.y + visibleSize.height / 2));
  this->addChild(rightWall, 0);
  auto rightWallBody = PhysicsBody::createBox(rightWall->getContentSize(), wallMaterial);
  rightWallBody->setDynamic(false);
  rightWallBody->setCategoryBitmask(0x0001);
  rightWallBody->setContactTestBitmask(0xFFFFFFFF);
  rightWallBody->setCollisionBitmask(0xFFFFFFFF);
  rightWall->setPhysicsBody(rightWallBody);

  // 上边界
  auto topWall = Sprite::create();
  topWall->setTextureRect(Rect(0, 0, visibleSize.width, 10));
  topWall->setColor(Color3B(60, 50, 80));
  topWall->setPosition(Vec2(origin.x + visibleSize.width / 2,
                            origin.y + visibleSize.height - 5));
  this->addChild(topWall, 0);
  auto topWallBody = PhysicsBody::createBox(topWall->getContentSize(), wallMaterial);
  topWallBody->setDynamic(false);
  topWallBody->setCategoryBitmask(0x0001);
  topWallBody->setContactTestBitmask(0xFFFFFFFF);
  topWallBody->setCollisionBitmask(0xFFFFFFFF);
  topWall->setPhysicsBody(topWallBody);

  // 地面
  PhysicsMaterial groundMaterial(1.0f, 0.5f, 0.0f);
  auto ground = Sprite::create();
  ground->setTextureRect(Rect(0, 0, visibleSize.width, 50));
  ground->setColor(Color3B(80, 60, 100));
  ground->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + 25));
  this->addChild(ground, 0);
  auto groundBody = PhysicsBody::createBox(ground->getContentSize(), groundMaterial);
  groundBody->setDynamic(false);
  groundBody->setCategoryBitmask(0x0001);
  groundBody->setContactTestBitmask(0xFFFFFFFF);
  groundBody->setCollisionBitmask(0xFFFFFFFF);
  groundBody->setGroup(0);
  ground->setPhysicsBody(groundBody);

  // 中间障碍物（让恶魔眼有东西可以绕）
  auto obstacle1 = Sprite::create();
  obstacle1->setTextureRect(Rect(0, 0, 80, 80));
  obstacle1->setColor(Color3B(100, 80, 120));
  obstacle1->setPosition(Vec2(origin.x + visibleSize.width / 3,
                              origin.y + visibleSize.height / 2));
  this->addChild(obstacle1, 0);
  auto obstacle1Body = PhysicsBody::createBox(obstacle1->getContentSize(), wallMaterial);
  obstacle1Body->setDynamic(false);
  obstacle1Body->setCategoryBitmask(0x0001);
  obstacle1Body->setContactTestBitmask(0xFFFFFFFF);
  obstacle1Body->setCollisionBitmask(0xFFFFFFFF);
  obstacle1->setPhysicsBody(obstacle1Body);

  auto obstacle2 = Sprite::create();
  obstacle2->setTextureRect(Rect(0, 0, 60, 120));
  obstacle2->setColor(Color3B(100, 80, 120));
  obstacle2->setPosition(Vec2(origin.x + visibleSize.width * 2 / 3,
                              origin.y + visibleSize.height / 3));
  this->addChild(obstacle2, 0);
  auto obstacle2Body = PhysicsBody::createBox(obstacle2->getContentSize(), wallMaterial);
  obstacle2Body->setDynamic(false);
  obstacle2Body->setCategoryBitmask(0x0001);
  obstacle2Body->setContactTestBitmask(0xFFFFFFFF);
  obstacle2Body->setCollisionBitmask(0xFFFFFFFF);
  obstacle2->setPhysicsBody(obstacle2Body);

  CCLOG("DemonEyeTestScene: Physics environment created");
}

void DemonEyeTestScene::createFakePlayerEntity()
{
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  _fakePlayer = Sprite::create();
  _fakePlayer->setTextureRect(Rect(0, 0, 30, 50));
  _fakePlayer->setColor(Color3B(255, 200, 100));
  _fakePlayer->setPosition(
      Vec2(visibleSize.width / 2 + origin.x, origin.y + 150));
  this->addChild(_fakePlayer, 1);
  _fakePlayerVisible = true;

  _playerLabel = Label::createWithTTF("Player (WASD:Move R:Toggle)",
                                      "fonts/Marker Felt.ttf", 14);
  if (_playerLabel != nullptr)
  {
    _playerLabel->setPosition(
        Vec2(_fakePlayer->getPositionX(), _fakePlayer->getPositionY() + 40));
    this->addChild(_playerLabel, 1);
  }

  // 创建玩家实体
  auto playerEntity = _registry.create();

  auto &transform = _registry.emplace<ecs::TransformComponent>(playerEntity);
  transform.position.x = _fakePlayer->getPositionX();
  transform.position.y = _fakePlayer->getPositionY();

  _registry.emplace<ecs::PlayerTag>(playerEntity);

  _fakePlayerEntity = entt::to_integral(playerEntity);

  ecs::NodeEntityMap::getInstance().registerNode(_fakePlayer, _fakePlayerEntity);

  CCLOG("DemonEyeTestScene: Fake player created (EnTT entity %u)", _fakePlayerEntity);
}

void DemonEyeTestScene::createDemonEyes()
{
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  CCLOG("DemonEyeTestScene: Creating Demon Eyes via MonsterFactory...");

  auto &factory = MonsterFactory::getInstance();
  
  // 恶魔眼类型ID列表
  const char* eyeTypes[] = {
    "DemonEye",
    "PurpleEye",
    "GreenEye",
    "CataractEye",
    "DilatedEye"
  };
  const int eyeTypeCount = 5;

  for (int i = 0; i < eyeTypeCount; i++)
  {
    // 随机位置（在屏幕上半部分）
    float x = origin.x + 100.0f + (rand() % (int)(visibleSize.width - 200.0f));
    float y = origin.y + visibleSize.height / 2 + (rand() % (int)(visibleSize.height / 3));

    // 使用MonsterFactory创建DemonEye
    ecs::EntityId entityId = factory.createMonster(_registry, eyeTypes[i], x, y, this);
    
    if (entityId != ecs::INVALID_ENTITY) {
      CCLOG("DemonEyeTestScene: Created %s at (%.1f, %.1f), entity=%u",
            eyeTypes[i], x, y, entityId);
    } else {
      CCLOG("DemonEyeTestScene: Failed to create %s", eyeTypes[i]);
    }
  }

  CCLOG("DemonEyeTestScene: Created %d Demon Eyes", eyeTypeCount);
}

void DemonEyeTestScene::setupSharedContactListener()
{
  // 碰撞监听器：处理恶魔眼撞墙弧形回弹
  _sharedContactListener = ecs::PhysicsContactHandler::createContactListener(
      _registry,
      // 碰撞开始处理
      [this](PhysicsContact &contact, const ecs::PhysicsContactHandler::ContactInfo &info) -> bool
      {
        auto bodyA = contact.getShapeA()->getBody();
        auto bodyB = contact.getShapeB()->getBody();
        Node *nodeA = bodyA->getNode();
        Node *nodeB = bodyB->getNode();

        ecs::EntityId entityA = nodeA ? ecs::NodeEntityMap::getInstance().findEntity(nodeA) : ecs::INVALID_ENTITY;
        ecs::EntityId entityB = nodeB ? ecs::NodeEntityMap::getInstance().findEntity(nodeB) : ecs::INVALID_ENTITY;

        // 检测恶魔眼与墙壁碰撞
        ecs::DemonEyeMovementComponent *demonA = nullptr;
        ecs::DemonEyeMovementComponent *demonB = nullptr;

        if (entityA != ecs::INVALID_ENTITY)
        {
          auto entA = static_cast<entt::entity>(entityA);
          if (_registry.valid(entA))
          {
            demonA = _registry.try_get<ecs::DemonEyeMovementComponent>(entA);
          }
        }
        if (entityB != ecs::INVALID_ENTITY)
        {
          auto entB = static_cast<entt::entity>(entityB);
          if (_registry.valid(entB))
          {
            demonB = _registry.try_get<ecs::DemonEyeMovementComponent>(entB);
          }
        }

        // 恶魔眼碰撞处理 - 物理引擎原生反弹
        // 物理引擎会通过restitution自动处理反弹

        return true;
      },
      nullptr // 无分离处理
  );

  _eventDispatcher->addEventListenerWithSceneGraphPriority(_sharedContactListener, this);
  CCLOG("DemonEyeTestScene: Contact listener initialized");
}

void DemonEyeTestScene::menuBackCallback(Ref *pSender)
{
  auto mainMenuScene = MainMenuScene::createScene();
  Director::getInstance()->replaceScene(TransitionFade::create(0.5f, mainMenuScene));
}

void DemonEyeTestScene::setupKeyboardListener()
{
  auto keyboardListener = EventListenerKeyboard::create();
  keyboardListener->onKeyPressed = [this](EventKeyboard::KeyCode keyCode, Event *event)
  {
    _keysPressed[keyCode] = true;
    if (keyCode == EventKeyboard::KeyCode::KEY_R)
      toggleFakePlayer();
  };
  keyboardListener->onKeyReleased = [this](EventKeyboard::KeyCode keyCode, Event *event)
  {
    _keysPressed[keyCode] = false;
  };
  _eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListener, this);
}

void DemonEyeTestScene::update(float delta)
{
  Layer::update(delta);
  updateFakePlayerPosition(delta);
  _systemManager.update(delta);
}

void DemonEyeTestScene::updateFakePlayerPosition(float delta)
{
  if (!_fakePlayerVisible || _fakePlayer == nullptr)
    return;

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

  if (movement.x != 0 || movement.y != 0)
  {
    Vec2 newPos = _fakePlayer->getPosition() + movement;
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    newPos.x = std::max(origin.x + 20.0f, std::min(newPos.x, origin.x + visibleSize.width - 20.0f));
    newPos.y = std::max(origin.y + 60.0f, std::min(newPos.y, origin.y + visibleSize.height - 20.0f));
    _fakePlayer->setPosition(newPos);
    if (_playerLabel)
      _playerLabel->setPosition(Vec2(_fakePlayer->getPositionX(), _fakePlayer->getPositionY() + 40));

    // 同步玩家位置到EnTT registry
    if (_fakePlayerEntity != ecs::INVALID_ENTITY)
    {
      auto playerEntity = static_cast<entt::entity>(_fakePlayerEntity);
      if (_registry.valid(playerEntity))
      {
        auto *transform = _registry.try_get<ecs::TransformComponent>(playerEntity);
        if (transform)
        {
          transform->position = newPos;
        }
      }
    }
  }
}

void DemonEyeTestScene::toggleFakePlayer()
{
  _fakePlayerVisible = !_fakePlayerVisible;
  _fakePlayer->setVisible(_fakePlayerVisible);
  if (_playerLabel)
    _playerLabel->setVisible(_fakePlayerVisible);
  CCLOG("DemonEyeTestScene: Fake Player %s", _fakePlayerVisible ? "Visible" : "Hidden");
}
