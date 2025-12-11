#include "SlimeTestScene.h"
#include "MainMenuScene.h"
#include "MonsterFactory.h"
#include "ecs/SpriteComponent.h"

USING_NS_CC;

Scene *SlimeTestScene::createScene()
{
  auto scene = Scene::createWithPhysics();
  auto physicsWorld = scene->getPhysicsWorld();
  physicsWorld->setGravity(Vec2(0, -980));
  physicsWorld->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);

  // 提高物理引擎精度，减少穿透问题
  physicsWorld->setSpeed(1.0f);
  physicsWorld->setSubsteps(8); // 增加子步数提高碰撞精度

  auto layer = SlimeTestScene::create();
  scene->addChild(layer);
  return scene;
}

bool SlimeTestScene::init()
{
  if (!Layer::init())
  {
    return false;
  }

  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 创建背景
  auto background = LayerColor::create(Color4B(20, 40, 20, 255));
  this->addChild(background, 0);

  // 添加标题
  auto titleLabel = Label::createWithTTF("Slime Test Scene",
                                         "fonts/Marker Felt.ttf", 32);
  if (titleLabel != nullptr)
  {
    titleLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                 origin.y + visibleSize.height -
                                     titleLabel->getContentSize().height - 20));
    this->addChild(titleLabel, 1);
  }

  // 添加返回按钮
  auto backLabel =
      Label::createWithTTF("Back to Menu", "fonts/Marker Felt.ttf", 24);
  auto backItem = MenuItemLabel::create(
      backLabel, CC_CALLBACK_1(SlimeTestScene::menuBackCallback, this));

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
  createEcsSlime();
  setupKeyboardListener();
  this->scheduleUpdate();

  CCLOG("SlimeTestScene: Initialized");
  return true;
}

void SlimeTestScene::setupEcsSystems()
{
  auto &factory = MonsterFactory::getInstance();
  factory.loadConfigsFromDir("config/slimes");

  // 史莱姆专用系统
  _world.addSystem<ecs::AggroSystem>();
  _world.addSystem<ecs::GroundDetectorSystem>();
  _world.addSystem<ecs::SlowFallSystem>();
  _world.addSystem<ecs::JumpMovementSystem>();
  _world.addSystem<ecs::ProjectileAttackSystem>();
  _world.addSystem<ecs::ProjectileSystem>();
  _world.addSystem<ecs::DebuffSystem>();
  _world.addSystem<ecs::SlimeSyncSystem>();
  _world.addSystem<ecs::SlimeRenderSystem>();
  _world.addSystem<ecs::HealthSystem>();
  _world.addSystem<ecs::CombatSystem>();
  _world.addSystem<ecs::LifetimeSystem>();

  CCLOG("SlimeTestScene: ECS Systems initialized");
}

void SlimeTestScene::createPhysicsEnvironment()
{
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();
  // 墙壁使用0摩擦力，避免贴墙时产生粘滞和嵌入
  PhysicsMaterial wallMaterial(1.0f, 0.0f, 0.0f);

  // 左边界 - 明确设置碰撞掩码
  auto leftWall = Sprite::create();
  leftWall->setTextureRect(Rect(0, 0, 10, visibleSize.height));
  leftWall->setColor(Color3B(80, 80, 80));
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
  rightWall->setColor(Color3B(80, 80, 80));
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
  topWall->setColor(Color3B(80, 80, 80));
  topWall->setPosition(Vec2(origin.x + visibleSize.width / 2,
                            origin.y + visibleSize.height - 5));
  this->addChild(topWall, 0);
  auto topWallBody = PhysicsBody::createBox(topWall->getContentSize(), wallMaterial);
  topWallBody->setDynamic(false);
  topWallBody->setContactTestBitmask(0xFFFFFFFF);
  topWall->setPhysicsBody(topWallBody);

  // 地面 - 确保与所有动态物体碰撞
  PhysicsMaterial groundMaterial(1.0f, 0.0f, 1.0f);
  auto ground = Sprite::create();
  ground->setTextureRect(Rect(0, 0, visibleSize.width, 50));
  ground->setColor(Color3B(139, 90, 43));
  ground->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + 25));
  this->addChild(ground, 0);
  auto groundBody = PhysicsBody::createBox(ground->getContentSize(), groundMaterial);
  groundBody->setDynamic(false);
  groundBody->setCategoryBitmask(0x0001);     // 地形类别
  groundBody->setContactTestBitmask(0xFFFFFFFF);
  groundBody->setCollisionBitmask(0xFFFFFFFF); // 与所有物体碰撞
  groundBody->setGroup(0); // 使用默认组，确保与所有物体碰撞
  ground->setPhysicsBody(groundBody);
  CCLOG("SlimeTestScene: Ground created at (%.1f, %.1f), body=%p", 
        ground->getPositionX(), ground->getPositionY(), groundBody);

  // 中间平台
  auto platform1 = Sprite::create();
  platform1->setTextureRect(Rect(0, 0, 300, 30));
  platform1->setColor(Color3B(100, 150, 100));
  platform1->setPosition(Vec2(origin.x + visibleSize.width / 2 - 200,
                              origin.y + visibleSize.height / 3));
  this->addChild(platform1, 0);
  auto platform1Body = PhysicsBody::createBox(platform1->getContentSize(), groundMaterial);
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
  auto platform2Body = PhysicsBody::createBox(platform2->getContentSize(), groundMaterial);
  platform2Body->setDynamic(false);
  platform2Body->setContactTestBitmask(0xFFFFFFFF);
  platform2->setPhysicsBody(platform2Body);
}

void SlimeTestScene::createFakePlayerEntity()
{
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  _fakePlayer = Sprite::create();
  _fakePlayer->setTextureRect(Rect(0, 0, 30, 50));
  _fakePlayer->setColor(Color3B(255, 100, 100));
  _fakePlayer->setPosition(
      Vec2(visibleSize.width / 2 + origin.x - 150, origin.y + 100));
  this->addChild(_fakePlayer, 1);
  _fakePlayerVisible = true;

  _playerLabel = Label::createWithTTF("Fake Player (WASD:Move R:Toggle)",
                                      "fonts/Marker Felt.ttf", 14);
  if (_playerLabel != nullptr)
  {
    _playerLabel->setPosition(
        Vec2(_fakePlayer->getPositionX(), _fakePlayer->getPositionY() + 40));
    this->addChild(_playerLabel, 1);
  }

  _fakePlayerEntity = _world.createEntity("Player");
  _world.addComponent<ecs::TransformComponent>(_fakePlayerEntity,
                                               _fakePlayer->getPositionX(),
                                               _fakePlayer->getPositionY());
  _world.addComponent<ecs::PlayerTag>(_fakePlayerEntity);

  CCLOG("SlimeTestScene: Fake player created");
}

void SlimeTestScene::createEcsSlime()
{
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();
  auto &factory = MonsterFactory::getInstance();

  CCLOG("SlimeTestScene: visibleSize=(%.1f, %.1f), origin=(%.1f, %.1f)",
        visibleSize.width, visibleSize.height, origin.x, origin.y);

  const char *slimeTypes[] = {
      "GreenSlime", "BlueSlime", "RedSlime",
      "YellowSlime", "PurpleSlime", "PinkSlime", "IceSlime",
      "SpikedIceSlime", "SpikedJungleSlime", "UmbrellaSlime", "MotherSlime", "BabySlime"};
  int slimeCount = sizeof(slimeTypes) / sizeof(slimeTypes[0]);

  float groundTop = origin.y + 50.0f;
  CCLOG("SlimeTestScene: groundTop=%.1f, will spawn %d slimes", groundTop, slimeCount);

  for (int i = 0; i < slimeCount; i++)
  {
    float X = origin.x + 100.0f + i * (visibleSize.width - 200.0f) / (slimeCount - 1);
    float Y = groundTop + 100.0f;
    CCLOG("SlimeTestScene: Spawning %s at (%.1f, %.1f) [i=%d]", slimeTypes[i], X, Y, i);
    factory.createMonster(_world, slimeTypes[i], X, Y, this);
  }
  CCLOG("SlimeTestScene: All slimes spawned");
}

void SlimeTestScene::setupSharedContactListener()
{
  _sharedContactListener = EventListenerPhysicsContact::create();

  _sharedContactListener->onContactBegin = [this](PhysicsContact &contact)
  {
    auto bodyA = contact.getShapeA()->getBody();
    auto bodyB = contact.getShapeB()->getBody();
    
    Node *nodeA = bodyA->getNode();
    Node *nodeB = bodyB->getNode();
    
    ecs::EntityId entityA = nodeA ? ecs::NodeEntityMap::getInstance().findEntity(nodeA) : ecs::INVALID_ENTITY;
    ecs::EntityId entityB = nodeB ? ecs::NodeEntityMap::getInstance().findEntity(nodeB) : ecs::INVALID_ENTITY;
    
    // 投射物碰撞处理
    ecs::ProjectileComponent *projA = (entityA != ecs::INVALID_ENTITY) ? 
        _world.getComponent<ecs::ProjectileComponent>(entityA) : nullptr;
    ecs::ProjectileComponent *projB = (entityB != ecs::INVALID_ENTITY) ? 
        _world.getComponent<ecs::ProjectileComponent>(entityB) : nullptr;
    
    if (projA || projB) {
      ecs::ProjectileComponent *proj = projA ? projA : projB;
      ecs::EntityId otherEntity = projA ? entityB : entityA;
      PhysicsBody *otherBody = projA ? bodyB : bodyA;
      
      if (proj->hasHit) return true;
      if (otherEntity == proj->owner) return true;
      
      if (!otherBody->isDynamic()) {
        proj->hasHit = true;
        return true;
      }
      
      if (otherEntity != ecs::INVALID_ENTITY) {
        auto *playerTag = _world.getComponent<ecs::PlayerTag>(otherEntity);
        if (playerTag) {
          auto *health = _world.getComponent<ecs::HealthComponent>(otherEntity);
          if (health) health->takeDamage(proj->damage);
          
          auto *debuff = _world.getComponent<ecs::DebuffComponent>(otherEntity);
          if (!debuff) debuff = &_world.addComponent<ecs::DebuffComponent>(otherEntity);
          
          if (proj->chillChance > 0 && (float)rand() / RAND_MAX < proj->chillChance) {
            debuff->applyChillDebuff(proj->chillDuration, proj->chillSpeedReduction);
          }
          if (proj->freezeChance > 0 && (float)rand() / RAND_MAX < proj->freezeChance) {
            debuff->applyFreezeDebuff(proj->freezeDuration);
          }
          if (proj->poisonChance1 > 0 && (float)rand() / RAND_MAX < proj->poisonChance1) {
            debuff->applyPoisonDebuff(proj->poisonDuration1, proj->poisonDamage1);
          } else if (proj->poisonChance2 > 0 && (float)rand() / RAND_MAX < proj->poisonChance2) {
            debuff->applyPoisonDebuff(proj->poisonDuration2, proj->poisonDamage2);
          }
          
          proj->hasHit = true;
          return true;
        }
      }
      return true;
    }

    // 地面检测 - 检查碰撞法线，只有向上的接触才算地面
    PhysicsBody *dynamicBody = nullptr;
    PhysicsBody *staticBody = nullptr;
    bool dynamicIsA = false;
    if (bodyA->isDynamic() && !bodyB->isDynamic()) {
      dynamicBody = bodyA;
      staticBody = bodyB;
      dynamicIsA = true;
    } else if (bodyB->isDynamic() && !bodyA->isDynamic()) {
      dynamicBody = bodyB;
      staticBody = bodyA;
      dynamicIsA = false;
    } else {
      return true;
    }

    Node *dynamicNode = dynamicBody->getNode();
    Node *staticNode = staticBody->getNode();
    if (!dynamicNode) return true;

    // 获取接触法线（指向动态物体的方向）
    cocos2d::Vec2 normal = contact.getContactData()->normal;
    if (!dynamicIsA) normal = -normal;
    
    // 地面法线实际是向下的(0, -1)，墙壁法线是水平的
    // 检查normal.y < -0.3，即向下的法线才算地面
    bool isGroundContact = (normal.y < -0.3f);

    if (isGroundContact) {
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

  _sharedContactListener->onContactSeparate = [this](PhysicsContact &contact)
  {
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
  CCLOG("SlimeTestScene: Contact listener initialized");
}

void SlimeTestScene::menuBackCallback(Ref *pSender)
{
  auto mainMenuScene = MainMenuScene::createScene();
  Director::getInstance()->replaceScene(TransitionFade::create(0.5f, mainMenuScene));
}

void SlimeTestScene::setupKeyboardListener()
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

void SlimeTestScene::update(float delta)
{
  Layer::update(delta);
  updateFakePlayerPosition(delta);
  _world.update(delta);
  
  // 调试：追踪史莱姆位置（每60帧输出一次）
  static int frameCount = 0;
  if (++frameCount >= 60) {
    frameCount = 0;
    _world.forEach<ecs::SlimeSpriteComponent, ecs::TransformComponent>(
        [](ecs::EntityId entity, ecs::SlimeSpriteComponent &sprite, ecs::TransformComponent &transform) {
          if (sprite.sprite) {
            auto pos = sprite.sprite->getPosition();
            auto *body = sprite.sprite->getPhysicsBody();
            cocos2d::Vec2 vel(0, 0);
            if (body) vel = body->getVelocity();
            CCLOG("Slime %u: pos=(%.1f, %.1f) vel=(%.1f, %.1f) visible=%d", 
                  entity, pos.x, pos.y, vel.x, vel.y, sprite.sprite->isVisible());
          }
        });
  }
}

void SlimeTestScene::updateFakePlayerPosition(float delta)
{
  if (!_fakePlayerVisible || _fakePlayer == nullptr) return;

  float moveSpeed = 200.0f;
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
    newPos.y = std::max(origin.y + 20.0f, std::min(newPos.y, origin.y + visibleSize.height - 20.0f));
    _fakePlayer->setPosition(newPos);
    if (_playerLabel) _playerLabel->setPosition(Vec2(_fakePlayer->getPositionX(), _fakePlayer->getPositionY() + 40));
    if (_fakePlayerEntity != ecs::INVALID_ENTITY) {
      auto *transform = _world.getComponent<ecs::TransformComponent>(_fakePlayerEntity);
      if (transform) transform->position = newPos;
    }
  }
}

void SlimeTestScene::toggleFakePlayer()
{
  _fakePlayerVisible = !_fakePlayerVisible;
  _fakePlayer->setVisible(_fakePlayerVisible);
  if (_playerLabel) _playerLabel->setVisible(_fakePlayerVisible);
  CCLOG("SlimeTestScene: Fake Player %s", _fakePlayerVisible ? "Visible" : "Hidden");
}
