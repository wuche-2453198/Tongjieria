#include "ZombieTestScene.h"
#include "MainMenuScene.h"
#include "MonsterFactory.h"
#include "ecs/systems/PhysicsContactHandler.h"

USING_NS_CC;

Scene *ZombieTestScene::createScene()
{
  auto scene = Scene::createWithPhysics();
  auto physicsWorld = scene->getPhysicsWorld();
  physicsWorld->setGravity(Vec2(0, -980));
  //  physicsWorld->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);

  // 提高物理引擎精度，减少穿透问题
  physicsWorld->setSpeed(1.0f);
  physicsWorld->setSubsteps(8); // 增加子步数提高碰撞精度

  auto layer = ZombieTestScene::create();
  scene->addChild(layer);
  return scene;
}

bool ZombieTestScene::init()
{
  if (!Layer::init())
  {
    return false;
  }

  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 创建深色背景（僵尸主题）
  auto background = LayerColor::create(Color4B(30, 20, 40, 255));
  this->addChild(background, 0);

  // 添加标题
  auto titleLabel = Label::createWithTTF("Zombie Test Scene",
                                         "fonts/Marker Felt.ttf", 32);
  if (titleLabel != nullptr)
  {
    titleLabel->setColor(Color3B(200, 100, 100));
    titleLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                 origin.y + visibleSize.height -
                                     titleLabel->getContentSize().height - 20));
    this->addChild(titleLabel, 1);
  }

  // 添加返回按钮
  auto backLabel =
      Label::createWithTTF("Back to Menu", "fonts/Marker Felt.ttf", 24);
  auto backItem = MenuItemLabel::create(
      backLabel, CC_CALLBACK_1(ZombieTestScene::menuBackCallback, this));

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
  createEcsZombie();
  setupKeyboardListener();
  this->scheduleUpdate();

  CCLOG("ZombieTestScene: Initialized");
  return true;
}

void ZombieTestScene::setupEcsSystems()
{
  auto &factory = MonsterFactory::getInstance();
  factory.clearConfigs();  // 清空之前场景的配置
  factory.loadConfigsFromDir("config/zombies");

  // ==================== 使用EnTT版本Systems（僵尸专用）====================
  CCLOG("========== Setting up EnTT Systems for Zombies ==========");
  
  _systemManager.setRegistry(&_registry);
  
  // 按优先级顺序添加Systems（仅僵尸相关）
  _systemManager.addSystem<ecs::AggroSystemEntt>();
  _systemManager.addSystem<ecs::MonsterGroundDetectorSystemEntt>();
  _systemManager.addSystem<ecs::WarriorAISystemEntt>();
  _systemManager.addSystem<ecs::HealthSystemEntt>();
  _systemManager.addSystem<ecs::CombatSystemEntt>();
  _systemManager.addSystem<ecs::LifetimeSystemEntt>();
  
  // 新解耦渲染系统
  _systemManager.addSystem<ecs::RenderSystem>();
  _systemManager.addSystem<ecs::AnimationSystem>();
  
  // 注册Sprite销毁监听器
  ecs::SpriteDestructionObserver::registerToRegistry(_registry);
  
  CCLOG("EnTT Systems initialized: %zu systems", _systemManager.getSystemCount());
  CCLOG("==========================================");
}

void ZombieTestScene::createPhysicsEnvironment()
{
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();
  // 统一墙壁材质：适度弹性支持各类怪物
  PhysicsMaterial wallMaterial(1.0f, 0.3f, 0.0f);

  // 左边界
  auto leftWall = Sprite::create();
  leftWall->setTextureRect(Rect(0, 0, 10, visibleSize.height));
  leftWall->setColor(Color3B(60, 60, 80));
  leftWall->setPosition(Vec2(origin.x + 5, origin.y + visibleSize.height / 2));
  this->addChild(leftWall, 0);
  auto leftWallBody = PhysicsBody::createBox(leftWall->getContentSize(), wallMaterial);
  leftWallBody->setDynamic(false);
  leftWallBody->setContactTestBitmask(0xFFFFFFFF);
  leftWall->setPhysicsBody(leftWallBody);

  // 右边界
  auto rightWall = Sprite::create();
  rightWall->setTextureRect(Rect(0, 0, 10, visibleSize.height));
  rightWall->setColor(Color3B(60, 60, 80));
  rightWall->setPosition(Vec2(origin.x + visibleSize.width - 5,
                              origin.y + visibleSize.height / 2));
  this->addChild(rightWall, 0);
  auto rightWallBody = PhysicsBody::createBox(rightWall->getContentSize(), wallMaterial);
  rightWallBody->setDynamic(false);
  rightWallBody->setContactTestBitmask(0xFFFFFFFF);
  rightWall->setPhysicsBody(rightWallBody);

  // 上边界
  auto topWall = Sprite::create();
  topWall->setTextureRect(Rect(0, 0, visibleSize.width, 10));
  topWall->setColor(Color3B(60, 60, 80));
  topWall->setPosition(Vec2(origin.x + visibleSize.width / 2,
                            origin.y + visibleSize.height - 5));
  this->addChild(topWall, 0);
  auto topWallBody = PhysicsBody::createBox(topWall->getContentSize(), wallMaterial);
  topWallBody->setDynamic(false);
  topWallBody->setContactTestBitmask(0xFFFFFFFF);
  topWall->setPhysicsBody(topWallBody);

  // 统一地面材质：中等摩擦力+零弹性
  PhysicsMaterial groundMaterial(1.0f, 0.0f, 2.0f);
  auto ground = Sprite::create();
  ground->setTextureRect(Rect(0, 0, visibleSize.width, 50));
  ground->setColor(Color3B(80, 60, 40));
  ground->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + 25));
  this->addChild(ground, 0);
  auto groundBody = PhysicsBody::createBox(ground->getContentSize(), groundMaterial);
  groundBody->setDynamic(false);
  groundBody->setContactTestBitmask(0xFFFFFFFF);
  ground->setPhysicsBody(groundBody);

  // 左侧平台
  auto platform1 = Sprite::create();
  platform1->setTextureRect(Rect(0, 0, 200, 30));
  platform1->setColor(Color3B(100, 80, 60));
  platform1->setPosition(Vec2(origin.x + 150,
                              origin.y + visibleSize.height / 3));
  this->addChild(platform1, 0);
  auto platform1Body = PhysicsBody::createBox(platform1->getContentSize(), groundMaterial);
  platform1Body->setDynamic(false);
  platform1Body->setContactTestBitmask(0xFFFFFFFF);
  platform1->setPhysicsBody(platform1Body);

  // 中间平台
  auto platform2 = Sprite::create();
  platform2->setTextureRect(Rect(0, 0, 250, 30));
  platform2->setColor(Color3B(100, 80, 60));
  platform2->setPosition(Vec2(origin.x + visibleSize.width / 2,
                              origin.y + visibleSize.height / 2));
  this->addChild(platform2, 0);
  auto platform2Body = PhysicsBody::createBox(platform2->getContentSize(), groundMaterial);
  platform2Body->setDynamic(false);
  platform2Body->setContactTestBitmask(0xFFFFFFFF);
  platform2->setPhysicsBody(platform2Body);

  // 右侧平台
  auto platform3 = Sprite::create();
  platform3->setTextureRect(Rect(0, 0, 200, 30));
  platform3->setColor(Color3B(100, 80, 60));
  platform3->setPosition(Vec2(origin.x + visibleSize.width - 150,
                              origin.y + visibleSize.height / 3));
  this->addChild(platform3, 0);
  auto platform3Body = PhysicsBody::createBox(platform3->getContentSize(), groundMaterial);
  platform3Body->setDynamic(false);
  platform3Body->setContactTestBitmask(0xFFFFFFFF);
  platform3->setPhysicsBody(platform3Body);

  // 障碍物（用于测试僵尸跳跃）
  auto obstacle = Sprite::create();
  obstacle->setTextureRect(Rect(0, 0, 40, 60));
  obstacle->setColor(Color3B(80, 80, 100));
  obstacle->setPosition(Vec2(origin.x + visibleSize.width / 2 - 100, origin.y + 80));
  this->addChild(obstacle, 0);
  auto obstacleBody = PhysicsBody::createBox(obstacle->getContentSize(), groundMaterial);
  obstacleBody->setDynamic(false);
  obstacleBody->setContactTestBitmask(0xFFFFFFFF);
  obstacle->setPhysicsBody(obstacleBody);
  
  // 低处小平台（用于跳到高处平台）- 移动到靠近中间并加高
  auto stepPlatform1 = Sprite::create();
  stepPlatform1->setTextureRect(Rect(0, 0, 100, 20));
  stepPlatform1->setColor(Color3B(120, 100, 80));
  stepPlatform1->setPosition(Vec2(origin.x + visibleSize.width / 3, origin.y + 150));
  this->addChild(stepPlatform1, 0);
  auto stepPlatform1Body = PhysicsBody::createBox(stepPlatform1->getContentSize(), groundMaterial);
  stepPlatform1Body->setDynamic(false);
  stepPlatform1Body->setContactTestBitmask(0xFFFFFFFF);
  stepPlatform1->setPhysicsBody(stepPlatform1Body);
  
  auto stepPlatform2 = Sprite::create();
  stepPlatform2->setTextureRect(Rect(0, 0, 100, 20));
  stepPlatform2->setColor(Color3B(120, 100, 80));
  stepPlatform2->setPosition(Vec2(origin.x + visibleSize.width * 2 / 3, origin.y + 150));
  this->addChild(stepPlatform2, 0);
  auto stepPlatform2Body = PhysicsBody::createBox(stepPlatform2->getContentSize(), groundMaterial);
  stepPlatform2Body->setDynamic(false);
  stepPlatform2Body->setContactTestBitmask(0xFFFFFFFF);
  stepPlatform2->setPhysicsBody(stepPlatform2Body);
}

void ZombieTestScene::createFakePlayerEntity()
{
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  _fakePlayer = Sprite::create();
  _fakePlayer->setTextureRect(Rect(0, 0, 30, 50));
  _fakePlayer->setColor(Color3B(100, 200, 255));
  _fakePlayer->setPosition(Vec2(visibleSize.width / 2 + origin.x + 200, origin.y + 150));
  this->addChild(_fakePlayer, 1);
  _fakePlayerVisible = true;

  // 玩家使用0摩擦力，避免贴墙时产生粘滞
  PhysicsMaterial playerMaterial(1.0f, 0.0f, 0.0f);
  auto playerBody = PhysicsBody::createBox(Size(26, 46), playerMaterial, Vec2(0, 0));
  playerBody->setDynamic(true);
  playerBody->setMass(1.0f);
  playerBody->setRotationEnable(false);
  playerBody->setVelocityLimit(500.0f);
  playerBody->setCategoryBitmask(0x0004);
  playerBody->setContactTestBitmask(0xFFFFFFFF);
  playerBody->setCollisionBitmask(0xFFFFFFFF);
  _fakePlayer->setPhysicsBody(playerBody);

  _playerLabel = Label::createWithTTF("Player (AD:Move Space:Jump)", "fonts/Marker Felt.ttf", 14);
  if (_playerLabel) {
    _playerLabel->setPosition(Vec2(_fakePlayer->getPositionX(), _fakePlayer->getPositionY() + 40));
    this->addChild(_playerLabel, 1);
  }

  // 创建玩家实体
  auto playerEntity = _registry.create();
  
  // 添加组件
  auto& transform = _registry.emplace<ecs::TransformComponent>(playerEntity);
  transform.position.x = _fakePlayer->getPositionX();
  transform.position.y = _fakePlayer->getPositionY();
  
  _registry.emplace<ecs::PlayerTag>(playerEntity);
  
  // 注册到NodeEntityMap
  _fakePlayerEntity = entt::to_integral(playerEntity);
  ecs::NodeEntityMap::getInstance().registerNode(_fakePlayer, _fakePlayerEntity);
  
  CCLOG("EnTT: Created player entity %u", _fakePlayerEntity);
}

void ZombieTestScene::createEcsZombie()
{
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();
  auto &factory = MonsterFactory::getInstance();
  
  float groundTop = origin.y + 50.0f;
  
  // 所有僵尸类型
  const char* zombieTypes[] = {
    "Zombie",
    "31px-Zombie",
    "Bigger-Zombie",
    "BaldZombie",
    "29px-BaldZombie",
    "Bigger-BaldZombie",
    "PincushionZombie",
    "32px-PincushionZombie",
    "Bigger-PincushionZombie"
  };
  int numTypes = sizeof(zombieTypes) / sizeof(zombieTypes[0]);
  
  float spacing = visibleSize.width / (numTypes + 1);
  
  // 生成所有类型的僵尸
  for (int i = 0; i < numTypes; i++) {
    float x = origin.x + spacing * (i + 1);
    float y = groundTop + 100.0f;
    factory.createMonster(_registry, zombieTypes[i], x, y, this);
  }
}

void ZombieTestScene::setupSharedContactListener()
{
  // 使用PhysicsContactHandler创建碰撞监听器，自定义玩家地面检测逻辑
  _sharedContactListener = ecs::PhysicsContactHandler::createContactListener(
    _registry,
    // 自定义碰撞开始处理：玩家地面检测
    [this](PhysicsContact& contact, const ecs::PhysicsContactHandler::ContactInfo& info) -> bool {
      // 处理玩家地面检测（不在NodeEntityMap中）
      if (info.isValid && info.isGroundContact && info.dynamicNode == _fakePlayer) {
        _playerOnGround = true;
        _playerGroundContactCount++;
      }
      return true;  // 继续执行默认地面检测
    },
    // 自定义碰撞分离处理：玩家地面检测
    [this](PhysicsContact& contact, const ecs::PhysicsContactHandler::ContactInfo& info) {
      // 处理玩家地面检测（不在NodeEntityMap中）
      if (info.isValid && info.isGroundContact && info.dynamicNode == _fakePlayer) {
        _playerGroundContactCount--;
        if (_playerGroundContactCount <= 0) {
          _playerOnGround = false;
          _playerGroundContactCount = 0;
        }
      }
    }
  );

  _eventDispatcher->addEventListenerWithSceneGraphPriority(_sharedContactListener, this);
}

void ZombieTestScene::menuBackCallback(Ref *pSender)
{
  auto mainMenuScene = MainMenuScene::createScene();
  Director::getInstance()->replaceScene(TransitionFade::create(0.5f, mainMenuScene));
}

void ZombieTestScene::setupKeyboardListener()
{
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

void ZombieTestScene::update(float delta)
{
  Layer::update(delta);
  updateFakePlayerPosition(delta);
  _systemManager.update(delta);
}

void ZombieTestScene::updateFakePlayerPosition(float delta)
{
  if (!_fakePlayerVisible || !_fakePlayer) return;
  
  auto body = _fakePlayer->getPhysicsBody();
  if (!body) return;
  
  Vec2 velocity = body->getVelocity();
  
  float targetVelX = 0.0f;
  if (_keysPressed[EventKeyboard::KeyCode::KEY_A] || _keysPressed[EventKeyboard::KeyCode::KEY_LEFT_ARROW])
    targetVelX = -_playerMoveSpeed;
  if (_keysPressed[EventKeyboard::KeyCode::KEY_D] || _keysPressed[EventKeyboard::KeyCode::KEY_RIGHT_ARROW])
    targetVelX = _playerMoveSpeed;
  
  velocity.x = targetVelX;
  
  // 玩家跳跃 - 通过法线检测已经确保只有真正在地面上才会设置_playerOnGround
  if (_playerOnGround && 
      (_keysPressed[EventKeyboard::KeyCode::KEY_SPACE] || 
       _keysPressed[EventKeyboard::KeyCode::KEY_W] ||
       _keysPressed[EventKeyboard::KeyCode::KEY_UP_ARROW])) {
    velocity.y = _playerJumpForce;
    _playerOnGround = false;
  }
  
  body->setVelocity(velocity);
  
  if (_playerLabel) {
    _playerLabel->setPosition(Vec2(_fakePlayer->getPositionX(), _fakePlayer->getPositionY() + 40));
  }
  
  // 同步玩家位置到EnTT registry
  if (_fakePlayerEntity != ecs::INVALID_ENTITY) {
    auto playerEntity = static_cast<entt::entity>(_fakePlayerEntity);
    if (_registry.valid(playerEntity)) {
      auto* transform = _registry.try_get<ecs::TransformComponent>(playerEntity);
      if (transform) {
        transform->position = _fakePlayer->getPosition();
      }
    }
  }
}

void ZombieTestScene::toggleFakePlayer()
{
  _fakePlayerVisible = !_fakePlayerVisible;
  _fakePlayer->setVisible(_fakePlayerVisible);
  if (_playerLabel) _playerLabel->setVisible(_fakePlayerVisible);
}
