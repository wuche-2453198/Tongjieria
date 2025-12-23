#include "EaterCrimeraTestScene.h"
#include "MainMenuScene.h"
#include "MonsterFactory.h"
#include "ecs/systems/PhysicsContactHandler.h"

USING_NS_CC;

Scene *EaterCrimeraTestScene::createScene()
{
  auto scene = Scene::createWithPhysics();
  auto physicsWorld = scene->getPhysicsWorld();
  physicsWorld->setGravity(Vec2(0, -980));

  // 调试物理体
  //  physicsWorld->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);

  physicsWorld->setSpeed(1.0f);
  physicsWorld->setSubsteps(8);

  auto layer = EaterCrimeraTestScene::create();
  scene->addChild(layer);
  return scene;
}

bool EaterCrimeraTestScene::init()
{
  if (!Layer::init())
  {
    return false;
  }

  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 创建背景（深紫色/红色混合，腐化氛围）
  auto background = LayerColor::create(Color4B(40, 20, 40, 255));
  this->addChild(background, 0);

  // 添加标题
  auto titleLabel = Label::createWithTTF("Eater of Souls & Crimera Test Scene",
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
      "Flying enemies: Circle around player, then charge periodically",
      "fonts/Marker Felt.ttf", 16);
  if (infoLabel != nullptr)
  {
    infoLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                origin.y + visibleSize.height - 60));
    infoLabel->setColor(Color3B(200, 180, 255));
    this->addChild(infoLabel, 1);
  }

  // 添加返回按钮
  auto backLabel =
      Label::createWithTTF("Back to Menu", "fonts/Marker Felt.ttf", 24);
  auto backItem = MenuItemLabel::create(
      backLabel, CC_CALLBACK_1(EaterCrimeraTestScene::menuBackCallback, this));

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
  createEatersAndCrimeras();
  setupKeyboardListener();
  this->scheduleUpdate();

  CCLOG("EaterCrimeraTestScene: Initialized");
  return true;
}

void EaterCrimeraTestScene::setupEcsSystems()
{
  auto &factory = MonsterFactory::getInstance();
  factory.clearConfigs();  // 清空之前场景的配置
  // 加载噬魂怪和猩红喀迈拉配置
  factory.loadConfigsFromDir("config/eaters");

  CCLOG("========== Setting up EnTT Systems for Eaters & Crimeras ==========");

  _systemManager.setRegistry(&_registry);

  // 按优先级顺序添加Systems
  _systemManager.addSystem<ecs::AggroSystemEntt>();  // 必须：追踪玩家
  _systemManager.addSystem<ecs::EaterOfSoulsAISystemEntt>();
  
  // 新架构：使用RenderSystem和AnimationSystem
  _systemManager.addSystem<ecs::RenderSystem>();
  _systemManager.addSystem<ecs::AnimationSystem>();
  _systemManager.addSystem<ecs::MonsterSyncSystemEntt>();  // 同步物理位置
  
  // 基础系统
  _systemManager.addSystem<ecs::HealthSystemEntt>();
  _systemManager.addSystem<ecs::CombatSystemEntt>();
  _systemManager.addSystem<ecs::LifetimeSystemEntt>();
  
  // 注册Sprite销毁监听器
  ecs::SpriteDestructionObserver::registerToRegistry(_registry);

  CCLOG("EnTT Systems initialized: %zu systems", _systemManager.getSystemCount());
  CCLOG("==========================================");
}

void EaterCrimeraTestScene::createPhysicsEnvironment()
{
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 统一墙壁材质：适度弹性支持各类怪物
  PhysicsMaterial wallMaterial(1.0f, 0.3f, 0.0f);

  // 左边界
  auto leftWall = Sprite::create();
  leftWall->setTextureRect(Rect(0, 0, 10, visibleSize.height));
  leftWall->setColor(Color3B(60, 40, 60));
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
  rightWall->setColor(Color3B(60, 40, 60));
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
  topWall->setColor(Color3B(60, 40, 60));
  topWall->setPosition(Vec2(origin.x + visibleSize.width / 2,
                            origin.y + visibleSize.height - 5));
  this->addChild(topWall, 0);
  auto topWallBody = PhysicsBody::createBox(topWall->getContentSize(), wallMaterial);
  topWallBody->setDynamic(false);
  topWallBody->setCategoryBitmask(0x0001);
  topWallBody->setContactTestBitmask(0xFFFFFFFF);
  topWallBody->setCollisionBitmask(0xFFFFFFFF);
  topWall->setPhysicsBody(topWallBody);

  // 统一地面材质：中等摩擦力+零弹性
  PhysicsMaterial groundMaterial(1.0f, 0.0f, 2.0f);
  auto ground = Sprite::create();
  ground->setTextureRect(Rect(0, 0, visibleSize.width, 50));
  ground->setColor(Color3B(70, 50, 70));
  ground->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + 25));
  this->addChild(ground, 0);
  auto groundBody = PhysicsBody::createBox(ground->getContentSize(), groundMaterial);
  groundBody->setDynamic(false);
  groundBody->setCategoryBitmask(0x0001);
  groundBody->setContactTestBitmask(0xFFFFFFFF);
  groundBody->setCollisionBitmask(0xFFFFFFFF);
  groundBody->setGroup(0);
  ground->setPhysicsBody(groundBody);

  // 中间障碍物（让飞行怪物有东西可以绕）
  auto obstacle1 = Sprite::create();
  obstacle1->setTextureRect(Rect(0, 0, 80, 80));
  obstacle1->setColor(Color3B(90, 60, 90));
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
  obstacle2->setColor(Color3B(90, 60, 90));
  obstacle2->setPosition(Vec2(origin.x + visibleSize.width * 2 / 3,
                              origin.y + visibleSize.height / 3));
  this->addChild(obstacle2, 0);
  auto obstacle2Body = PhysicsBody::createBox(obstacle2->getContentSize(), wallMaterial);
  obstacle2Body->setDynamic(false);
  obstacle2Body->setCategoryBitmask(0x0001);
  obstacle2Body->setContactTestBitmask(0xFFFFFFFF);
  obstacle2Body->setCollisionBitmask(0xFFFFFFFF);
  obstacle2->setPhysicsBody(obstacle2Body);

  CCLOG("EaterCrimeraTestScene: Physics environment created");
}

void EaterCrimeraTestScene::createFakePlayerEntity()
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

  CCLOG("EaterCrimeraTestScene: Fake player created (EnTT entity %u)", _fakePlayerEntity);
}

void EaterCrimeraTestScene::createEatersAndCrimeras()
{
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  CCLOG("EaterCrimeraTestScene: Creating Eaters and Crimeras via MonsterFactory...");

  auto &factory = MonsterFactory::getInstance();
  
  // 怪物类型ID列表（3个噬魂怪 + 3个猩红喀迈拉）
  const char* monsterTypes[] = {
    "EaterOfSouls_Small",
    "EaterOfSouls_Medium",
    "EaterOfSouls_Large",
    "Crimera_Small",
    "Crimera_Medium",
    "Crimera_Large"
  };
  const int monsterCount = 6;

  for (int i = 0; i < monsterCount; i++)
  {
    // 分布位置（左侧3个噬魂怪，右侧3个猩红喀迈拉）
    float xOffset = (i < 3) ? 0.0f : visibleSize.width / 2;
    float yBase = origin.y + visibleSize.height / 2;
    
    float x = origin.x + 100.0f + xOffset + (rand() % 200);
    float y = yBase + ((i % 3) - 1) * 150.0f + (rand() % 100 - 50);

    // 使用MonsterFactory创建怪物
    ecs::EntityId entityId = factory.createMonster(_registry, monsterTypes[i], x, y, this);
    
    if (entityId != ecs::INVALID_ENTITY) {
      CCLOG("EaterCrimeraTestScene: Created %s at (%.1f, %.1f), entity=%u",
            monsterTypes[i], x, y, entityId);
    } else {
      CCLOG("EaterCrimeraTestScene: Failed to create %s", monsterTypes[i]);
    }
  }

  CCLOG("EaterCrimeraTestScene: Created %d monsters", monsterCount);
}

void EaterCrimeraTestScene::setupSharedContactListener()
{
  // 碰撞监听器：处理飞行怪物撞墙回弹
  _sharedContactListener = ecs::PhysicsContactHandler::createContactListener(
      _registry,
      // 碰撞开始处理
      [this](PhysicsContact &contact, const ecs::PhysicsContactHandler::ContactInfo &info) -> bool
      {
        // 物理引擎会通过restitution自动处理反弹
        return true;
      },
      nullptr // 无分离处理
  );

  _eventDispatcher->addEventListenerWithSceneGraphPriority(_sharedContactListener, this);
  CCLOG("EaterCrimeraTestScene: Contact listener initialized");
}

void EaterCrimeraTestScene::menuBackCallback(Ref *pSender)
{
  auto mainMenuScene = MainMenuScene::createScene();
  Director::getInstance()->replaceScene(TransitionFade::create(0.5f, mainMenuScene));
}

void EaterCrimeraTestScene::setupKeyboardListener()
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

void EaterCrimeraTestScene::update(float delta)
{
  Layer::update(delta);
  updateFakePlayerPosition(delta);
  _systemManager.update(delta);
}

void EaterCrimeraTestScene::updateFakePlayerPosition(float delta)
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

void EaterCrimeraTestScene::toggleFakePlayer()
{
  _fakePlayerVisible = !_fakePlayerVisible;
  _fakePlayer->setVisible(_fakePlayerVisible);
  if (_playerLabel)
    _playerLabel->setVisible(_fakePlayerVisible);
  CCLOG("EaterCrimeraTestScene: Fake Player %s", _fakePlayerVisible ? "Visible" : "Hidden");
}
