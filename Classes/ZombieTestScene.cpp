#include "ZombieTestScene.h"
#include "MainMenuScene.h"
#include "MonsterFactory.h"
#include "ecs/SpriteComponent.h"

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
  factory.loadConfigsFromDir("config/zombies");

  if (_useEnttSystems) {
    // ==================== 使用EnTT版本Systems ====================
    CCLOG("========== Setting up EnTT Systems ==========");
    
    _systemManager.setRegistry(&_registry);
    
    // 按优先级顺序添加Systems
    _systemManager.addSystem<ecs::AggroSystemEntt>();
    _systemManager.addSystem<ecs::MonsterGroundDetectorSystemEntt>();
    _systemManager.addSystem<ecs::WalkMovementSystemEntt>();
    _systemManager.addSystem<ecs::MonsterSyncSystemEntt>();
    _systemManager.addSystem<ecs::MonsterAnimationSystemEntt>();
    _systemManager.addSystem<ecs::HealthSystemEntt>();
    _systemManager.addSystem<ecs::CombatSystemEntt>();
    _systemManager.addSystem<ecs::LifetimeSystemEntt>();
    
    CCLOG("EnTT Systems initialized: %zu systems", _systemManager.getSystemCount());
    CCLOG("==========================================");
  } else {
    // ==================== 使用旧版ECS ====================
    _world.addSystem<ecs::AggroSystem>();
    _world.addSystem<ecs::MonsterGroundDetectorSystem>();
    _world.addSystem<ecs::WalkMovementSystem>();
    _world.addSystem<ecs::MonsterSyncSystem>();
    _world.addSystem<ecs::MonsterAnimationSystem>();
    _world.addSystem<ecs::HealthSystem>();
    _world.addSystem<ecs::CombatSystem>();
    _world.addSystem<ecs::LifetimeSystem>();
    CCLOG("Legacy ECS Systems initialized");
  }
}

void ZombieTestScene::createPhysicsEnvironment()
{
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();
  // 墙壁使用0摩擦力，避免贴墙时产生粘滞和嵌入
  PhysicsMaterial wallMaterial(1.0f, 0.0f, 0.0f);

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

  // 地面（墓地风格）
  PhysicsMaterial groundMaterial(1.0f, 0.0f, 1.0f);
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

  if (_useEnttSystems) {
    // EnTT版本：创建玩家实体
    auto playerEntity = _registry.create();
    
    // 添加组件
    auto& transform = _registry.emplace<ecs::TransformComponent>(playerEntity);
    transform.position.x = _fakePlayer->getPositionX();
    transform.position.y = _fakePlayer->getPositionY();
    
    _registry.emplace<ecs::PlayerTag>(playerEntity);
    
    // 注册到NodeEntityMap（使用EntityId兼容）
    _fakePlayerEntity = entt::to_integral(playerEntity);
    ecs::NodeEntityMap::getInstance().registerNode(_fakePlayer, _fakePlayerEntity);
    
    CCLOG("EnTT: Created player entity %u", _fakePlayerEntity);
  } else {
    // 旧版ECS：创建玩家实体
    _fakePlayerEntity = _world.createEntity("Player");
    _world.addComponent<ecs::TransformComponent>(_fakePlayerEntity, _fakePlayer->getPositionX(), _fakePlayer->getPositionY());
    _world.addComponent<ecs::PlayerTag>(_fakePlayerEntity);
    ecs::NodeEntityMap::getInstance().registerNode(_fakePlayer, _fakePlayerEntity);
  }
}

void ZombieTestScene::createEcsZombie()
{
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();
  auto &factory = MonsterFactory::getInstance();
  
  float groundTop = origin.y + 50.0f;
  float spacing = visibleSize.width / 5.0f;
  
  // 生成混合的僵尸：普通僵尸和31px小僵尸
  for (int i = 0; i < 6; i++) {
    float x = origin.x + spacing * (i + 0.5f);
    float y = groundTop + 100.0f;
    
    // 交替生成普通僵尸和小型僵尸
    const char* zombieType = (i % 2 == 0) ? "Zombie" : "31px-Zombie";
    
    if (_useEnttSystems) {
      // EnTT版本：使用createMonsterEntt
      factory.createMonsterEntt(_registry, zombieType, x, y, this);
    } else {
      // 旧版：使用createMonster
      factory.createMonster(_world, zombieType, x, y, this);
    }
  }
}

void ZombieTestScene::setupSharedContactListener()
{
  _sharedContactListener = EventListenerPhysicsContact::create();

  _sharedContactListener->onContactBegin = [this](PhysicsContact &contact) {
    auto bodyA = contact.getShapeA()->getBody();
    auto bodyB = contact.getShapeB()->getBody();

    // 地面检测 - 检查碰撞法线，只有向上的接触才算地面
    PhysicsBody *dynamicBody = nullptr;
    bool dynamicIsA = false;
    if (bodyA->isDynamic() && !bodyB->isDynamic()) {
      dynamicBody = bodyA;
      dynamicIsA = true;
    } else if (bodyB->isDynamic() && !bodyA->isDynamic()) {
      dynamicBody = bodyB;
      dynamicIsA = false;
    } else {
      return true;
    }

    Node *dynamicNode = dynamicBody->getNode();
    if (!dynamicNode) return true;
    
    // 获取接触法线（指向动态物体的方向）
    cocos2d::Vec2 normal = contact.getContactData()->normal;
    if (!dynamicIsA) normal = -normal;
    
    // 地面法线实际是向下的(0, -1)，墙壁法线是水平的
    // 检查normal.y < -0.3，即向下的法线才算地面
    bool isGroundContact = (normal.y < -0.3f);
    
    if (isGroundContact) {
      if (dynamicNode == _fakePlayer) {
        _playerOnGround = true;
        _playerGroundContactCount++;
      }

      ecs::EntityId entity = ecs::NodeEntityMap::getInstance().findEntity(dynamicNode);
      if (entity != ecs::INVALID_ENTITY) {
        auto *ground = _world.getComponent<ecs::GroundDetectorComponent>(entity);
        if (ground) {
          ground->isOnGround = true;
          ground->groundContactCount++;
        }
      }
    }
    return true;
  };

  _sharedContactListener->onContactSeparate = [this](PhysicsContact &contact) {
    auto bodyA = contact.getShapeA()->getBody();
    auto bodyB = contact.getShapeB()->getBody();
    PhysicsBody *dynamicBody = nullptr;
    bool dynamicIsA = false;
    if (bodyA->isDynamic() && !bodyB->isDynamic()) {
      dynamicBody = bodyA;
      dynamicIsA = true;
    } else if (bodyB->isDynamic() && !bodyA->isDynamic()) {
      dynamicBody = bodyB;
      dynamicIsA = false;
    } else {
      return;
    }

    Node *dynamicNode = dynamicBody->getNode();
    if (!dynamicNode) return;
    
    // 检查法线，只有地面接触分离时才减少计数
    // 这样可以避免墙角bug（墙壁接触没有增加计数，分离时也不应减少）
    cocos2d::Vec2 normal = contact.getContactData()->normal;
    if (!dynamicIsA) normal = -normal;
    bool wasGroundContact = (normal.y < -0.3f);
    
    if (!wasGroundContact) {
      // 这不是地面接触，直接返回，不减少计数
      return;
    }

    if (dynamicNode == _fakePlayer) {
      _playerGroundContactCount--;
      if (_playerGroundContactCount <= 0) {
        _playerOnGround = false;
        _playerGroundContactCount = 0;
      }
    }

    ecs::EntityId entity = ecs::NodeEntityMap::getInstance().findEntity(dynamicNode);
    if (entity != ecs::INVALID_ENTITY) {
      auto *ground = _world.getComponent<ecs::GroundDetectorComponent>(entity);
      if (ground) {
        ground->groundContactCount--;
        if (ground->groundContactCount <= 0) {
          ground->isOnGround = false;
          ground->groundContactCount = 0;
        }
      }
    }
  };

  // 预处理回调：防止撞墙时产生向上的滑动
  _sharedContactListener->onContactPreSolve = [this](PhysicsContact &contact, PhysicsContactPreSolve &solve) {
    auto bodyA = contact.getShapeA()->getBody();
    auto bodyB = contact.getShapeB()->getBody();
    
    PhysicsBody *dynamicBody = nullptr;
    bool dynamicIsA = false;
    if (bodyA->isDynamic() && !bodyB->isDynamic()) {
      dynamicBody = bodyA;
      dynamicIsA = true;
    } else if (bodyB->isDynamic() && !bodyA->isDynamic()) {
      dynamicBody = bodyB;
      dynamicIsA = false;
    } else {
      return true;
    }

    // 获取接触法线
    cocos2d::Vec2 normal = contact.getContactData()->normal;
    if (!dynamicIsA) normal = -normal;
    
    // 如果是侧面碰撞（墙壁），阻止垂直方向的反弹
    if (std::abs(normal.x) > 0.7f && std::abs(normal.y) < 0.3f) {
      // 这是侧面碰撞，设置弹性为0防止滑上去
      solve.setRestitution(0.0f);
      solve.setFriction(0.0f);
    }
    return true;
  };

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
  
  // 根据标志选择使用哪个ECS系统
  if (_useEnttSystems) {
    _systemManager.update(delta);  // EnTT版本
  } else {
    _world.update(delta);          // 旧版
  }
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
  // 射线预测，防止玩家物理体穿入静态障碍
  ecs::performRaycastCorrection(body, delta);
  
  if (_playerLabel) {
    _playerLabel->setPosition(Vec2(_fakePlayer->getPositionX(), _fakePlayer->getPositionY() + 40));
  }
  
  if (_fakePlayerEntity != ecs::INVALID_ENTITY) {
    auto *transform = _world.getComponent<ecs::TransformComponent>(_fakePlayerEntity);
    if (transform) transform->position = _fakePlayer->getPosition();
  }
}

void ZombieTestScene::toggleFakePlayer()
{
  _fakePlayerVisible = !_fakePlayerVisible;
  _fakePlayer->setVisible(_fakePlayerVisible);
  if (_playerLabel) _playerLabel->setVisible(_fakePlayerVisible);
}
