#include "EcsTestScene.h"
#include "MainMenuScene.h"
#include "MonsterFactory.h"
#include "ecs/SpriteComponent.h"
#include "base/ccRandom.h"

USING_NS_CC;

Scene *EcsTestScene::createScene()
{
  // 创建带物理引擎的场景
  auto scene = Scene::createWithPhysics();

  // 设置重力加速度
  auto physicsWorld = scene->getPhysicsWorld();
  physicsWorld->setGravity(Vec2(0, -980));

  // 调试线（可选）红框标注物理体
  physicsWorld->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);

  auto layer = EcsTestScene::create();
  scene->addChild(layer);
  return scene;
}

bool EcsTestScene::init()
{
  if (!Layer::init())
  {
    return false;
  }

  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 创建黑色背景
  auto background = LayerColor::create(Color4B(20, 20, 40, 255));
  this->addChild(background, 0);

  // 添加标题
  auto titleLabel = Label::createWithTTF("ECS Test Scene - Green Slime",
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
      backLabel, CC_CALLBACK_1(EcsTestScene::menuBackCallback, this));

  if (backItem != nullptr)
  {
    backItem->setPosition(
        Vec2(origin.x + visibleSize.width / 2, origin.y + 50));

    auto menu = Menu::create(backItem, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);
  }

  // 创建物理环境（边界、平台）
  createPhysicsEnvironment();

  // 初始化ECS系统
  setupEcsSystems();

  // 初始化共享碰撞监听器
  setupSharedContactListener();

  // 创建虚拟玩家实体
  createFakePlayerEntity();

  // 创建ECS史莱姆
  createEcsSlime();

  // 设置键盘监听
  setupKeyboardListener();

  // 开启更新
  this->scheduleUpdate();

  CCLOG("EcsTestScene: Initialized with ECS World");
  return true;
}

void EcsTestScene::setupEcsSystems()
{
  // 加载怪物配置 (从单独的配置文件加载)
  auto &factory = MonsterFactory::getInstance();
  if (!factory.isLoaded())
  {
    // 方式1: 从目录加载单独的配置文件 (推荐)
    factory.loadConfigsFromDir("config/slimes");

    // 方式2: 从单个合并文件加载 (兼容旧格式)
    // factory.loadConfig("config/monsters.json");
  }

  // 添加系统（按优先级自动排序）
  _world.addSystem<ecs::AggroSystem>();          // 仇恨检测
  _world.addSystem<ecs::GroundDetectorSystem>(); // 地面检测
  _world.addSystem<ecs::SlowFallSystem>();       // 缓降（伞史莱姆）
  _world.addSystem<ecs::JumpMovementSystem>();   // 跳跃移动
  _world.addSystem<ecs::ProjectileAttackSystem>(); // 投射物攻击（冰雪尖刺等）
  _world.addSystem<ecs::ProjectileSystem>();     // 投射物更新
  _world.addSystem<ecs::DebuffSystem>();         // 减益效果
  _world.addSystem<ecs::SlimeSyncSystem>();      // 位置同步
  _world.addSystem<ecs::SlimeRenderSystem>();    // 渲染
  _world.addSystem<ecs::HealthSystem>();         // 生命值
  _world.addSystem<ecs::CombatSystem>();         // 战斗
  _world.addSystem<ecs::LifetimeSystem>();       // 生命周期

  CCLOG("EcsTestScene: ECS Systems initialized");
}

void EcsTestScene::createPhysicsEnvironment()
{
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();
  PhysicsMaterial wallMaterial(1.0f, 0.0f, 1.0f);

  // ==================== 创建场景边框 ====================
  // 左边界
  auto leftWall = Sprite::create();
  leftWall->setTextureRect(Rect(0, 0, 10, visibleSize.height));
  leftWall->setColor(Color3B(80, 80, 80));
  leftWall->setPosition(Vec2(origin.x + 5, origin.y + visibleSize.height / 2));
  this->addChild(leftWall, 0);

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

  // 上边界/天花板
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

  // ==================== 创建地面平台 ====================
  PhysicsMaterial groundMaterial(1.0f, 0.0f, 1.0f);

  // 地面平台 - 底部
  auto ground = Sprite::create();
  ground->setTextureRect(Rect(0, 0, visibleSize.width, 50));
  ground->setColor(Color3B(139, 90, 43));
  ground->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + 25));
  this->addChild(ground, 0);

  auto groundBody =
      PhysicsBody::createBox(ground->getContentSize(), groundMaterial);
  groundBody->setDynamic(false);
  groundBody->setContactTestBitmask(0xFFFFFFFF);
  ground->setPhysicsBody(groundBody);

  // 中间平台
  auto platform1 = Sprite::create();
  platform1->setTextureRect(Rect(0, 0, 300, 30));
  platform1->setColor(Color3B(100, 150, 100));
  platform1->setPosition(Vec2(origin.x + visibleSize.width / 2 - 200,
                              origin.y + visibleSize.height / 3));
  this->addChild(platform1, 0);

  auto platform1Body =
      PhysicsBody::createBox(platform1->getContentSize(), groundMaterial);
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

  auto platform2Body =
      PhysicsBody::createBox(platform2->getContentSize(), groundMaterial);
  platform2Body->setDynamic(false);
  platform2Body->setContactTestBitmask(0xFFFFFFFF);
  platform2->setPhysicsBody(platform2Body);
}

void EcsTestScene::createFakePlayerEntity()
{
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 创建虚拟玩家精灵
  _fakePlayer = Sprite::create();
  _fakePlayer->setTextureRect(Rect(0, 0, 30, 50));
  _fakePlayer->setColor(Color3B(255, 100, 100));
  _fakePlayer->setPosition(
      Vec2(visibleSize.width / 2 + origin.x - 150, origin.y + 100));
  this->addChild(_fakePlayer, 1);
  _fakePlayerVisible = true;

  // 给虚拟玩家添加标签
  _playerLabel = Label::createWithTTF("Fake Player (WASD:Move R:Toggle)",
                                      "fonts/Marker Felt.ttf", 14);
  if (_playerLabel != nullptr)
  {
    _playerLabel->setPosition(
        Vec2(_fakePlayer->getPositionX(), _fakePlayer->getPositionY() + 40));
    this->addChild(_playerLabel, 1);
  }

  // 在ECS世界中创建玩家实体（用于索敌系统查找）
  _fakePlayerEntity = _world.createEntity("Player");
  _world.addComponent<ecs::TransformComponent>(_fakePlayerEntity,
                                               _fakePlayer->getPositionX(),
                                               _fakePlayer->getPositionY());
  _world.addComponent<ecs::PlayerTag>(_fakePlayerEntity);

  CCLOG("EcsTestScene: Fake player created at (%.1f, %.1f)",
        _fakePlayer->getPositionX(), _fakePlayer->getPositionY());
}

void EcsTestScene::createEcsSlime()
{
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();
  auto &factory = MonsterFactory::getInstance();

  // 每种史莱姆各生成一只
  const char *slimeTypes[] = {
      "GreenSlime", "BlueSlime", "RedSlime",
      "YellowSlime", "PurpleSlime", "IceSlime",
      "SpikedIceSlime", "SpikedJungleSlime", "UmbrellaSlime", "MotherSlime", "BabySlime"};
  int slimeCount = sizeof(slimeTypes) / sizeof(slimeTypes[0]);

  float groundTop = origin.y + 50.0f;

  for (int i = 0; i < slimeCount; i++)
  {
    // 水平均匀分布
    float X = origin.x + 100.0f + i * (visibleSize.width - 200.0f) / (slimeCount - 1);

    // 先创建怪物，让它自由落体到地面
    // 生成位置稍高于地面，物理引擎会让它落到正确位置
    float Y = groundTop + 100.0f;

    factory.createMonster(_world, slimeTypes[i], X, Y, this);
    CCLOG("Created %s at (%.1f, %.1f)", slimeTypes[i], X, Y);
  }
}
void EcsTestScene::setupSharedContactListener()
{
  _sharedContactListener = EventListenerPhysicsContact::create();

  _sharedContactListener->onContactBegin = [this](PhysicsContact &contact)
  {
    auto bodyA = contact.getShapeA()->getBody();
    auto bodyB = contact.getShapeB()->getBody();
    
    Node *nodeA = bodyA->getNode();
    Node *nodeB = bodyB->getNode();
    
    // 查找两个碰撞体对应的实体
    ecs::EntityId entityA = nodeA ? ecs::NodeEntityMap::getInstance().findEntity(nodeA) : ecs::INVALID_ENTITY;
    ecs::EntityId entityB = nodeB ? ecs::NodeEntityMap::getInstance().findEntity(nodeB) : ecs::INVALID_ENTITY;
    
    // 检查是否有投射物参与碰撞
    ecs::ProjectileComponent *projA = (entityA != ecs::INVALID_ENTITY) ? 
        _world.getComponent<ecs::ProjectileComponent>(entityA) : nullptr;
    ecs::ProjectileComponent *projB = (entityB != ecs::INVALID_ENTITY) ? 
        _world.getComponent<ecs::ProjectileComponent>(entityB) : nullptr;
    
    // 处理投射物碰撞
    if (projA || projB) {
      ecs::ProjectileComponent *proj = projA ? projA : projB;
      ecs::EntityId projEntity = projA ? entityA : entityB;
      ecs::EntityId otherEntity = projA ? entityB : entityA;
      PhysicsBody *otherBody = projA ? bodyB : bodyA;
      
      // 投射物已经命中过，跳过
      if (proj->hasHit) return true;
      
      // 忽略发射者自身的碰撞
      if (otherEntity == proj->owner) {
        return true; // 穿过发射者，不处理
      }
      
      // 撞到静态物体（地形）- 破碎消失
      if (!otherBody->isDynamic()) {
        proj->hasHit = true;
        CCLOG("Ice Spike hit terrain, destroying");
        return true;
      }
      
      // 撞到其他实体
      if (otherEntity != ecs::INVALID_ENTITY) {
        // 撞到玩家 - 造成伤害和减益
        auto *playerTag = _world.getComponent<ecs::PlayerTag>(otherEntity);
        if (playerTag) {
          // 对玩家造成伤害
          auto *health = _world.getComponent<ecs::HealthComponent>(otherEntity);
          if (health) {
            health->takeDamage(proj->damage);
            CCLOG("Ice Spike hit player for %.1f damage", proj->damage);
          }
          
          // 应用减益效果
          auto *debuff = _world.getComponent<ecs::DebuffComponent>(otherEntity);
          if (!debuff) {
            debuff = &_world.addComponent<ecs::DebuffComponent>(otherEntity);
          }
          
          // 冷冻减益
          if (proj->chillChance > 0 && (float)rand() / RAND_MAX < proj->chillChance) {
            debuff->applyChillDebuff(proj->chillDuration, proj->chillSpeedReduction);
            CCLOG("  Applied Chill debuff: %.1fs, %.0f%% slow", 
                  proj->chillDuration, proj->chillSpeedReduction * 100);
          }
          
          // 冰冻减益
          if (proj->freezeChance > 0 && (float)rand() / RAND_MAX < proj->freezeChance) {
            debuff->applyFreezeDebuff(proj->freezeDuration);
            CCLOG("  Applied Freeze debuff: %.1fs", proj->freezeDuration);
          }
          
          // 毒素减益1（长时间）
          if (proj->poisonChance1 > 0 && (float)rand() / RAND_MAX < proj->poisonChance1) {
            debuff->applyPoisonDebuff(proj->poisonDuration1, proj->poisonDamage1);
            CCLOG("  Applied Poison debuff (long): %.1fs, %.1f dps", 
                  proj->poisonDuration1, proj->poisonDamage1);
          }
          // 毒素减益2（短时间，如果没有应用长时间毒素）
          else if (proj->poisonChance2 > 0 && (float)rand() / RAND_MAX < proj->poisonChance2) {
            debuff->applyPoisonDebuff(proj->poisonDuration2, proj->poisonDamage2);
            CCLOG("  Applied Poison debuff (short): %.1fs, %.1f dps", 
                  proj->poisonDuration2, proj->poisonDamage2);
          }
          
          proj->hasHit = true;
          return true;
        }
        
        // 小怪（敌人）不会触发碰撞事件，因为物理体设置为不碰撞
        // 如果触发了碰撞且不是玩家，忽略
      }
      
      return true;
    }

    // 原有逻辑：处理史莱姆落地检测
    PhysicsBody *dynamicBody = nullptr;
    PhysicsBody *staticBody = nullptr;

    if (bodyA->isDynamic() && !bodyB->isDynamic())
    {
      dynamicBody = bodyA;
      staticBody = bodyB;
    }
    else if (bodyB->isDynamic() && !bodyA->isDynamic())
    {
      dynamicBody = bodyB;
      staticBody = bodyA;
    }
    else
    {
      return true; // 两个都是动态或都是静态，跳过
    }

    // 通过物理体的节点查找对应的实体
    Node *node = dynamicBody->getNode();
    if (!node)
      return true;

    // 使用 NodeEntityMap 快速查找实体 (O(1) 而不是 O(n))
    ecs::EntityId entity = ecs::NodeEntityMap::getInstance().findEntity(node);
    if (entity != ecs::INVALID_ENTITY)
    {
      auto *ground = _world.getComponent<ecs::GroundDetectorComponent>(entity);
      if (ground)
      {
        ground->isOnGround = true;
      }
    }

    return true;
  };

  _sharedContactListener->onContactSeparate = [this](PhysicsContact &contact)
  {
    auto bodyA = contact.getShapeA()->getBody();
    auto bodyB = contact.getShapeB()->getBody();

    PhysicsBody *dynamicBody = nullptr;

    if (bodyA->isDynamic() && !bodyB->isDynamic())
    {
      dynamicBody = bodyA;
    }
    else if (bodyB->isDynamic() && !bodyA->isDynamic())
    {
      dynamicBody = bodyB;
    }
    else
    {
      return;
    }

    Node *node = dynamicBody->getNode();
    if (!node)
      return;

    // 使用 NodeEntityMap 快速查找实体 (O(1) 而不是 O(n))
    ecs::EntityId entity = ecs::NodeEntityMap::getInstance().findEntity(node);
    if (entity != ecs::INVALID_ENTITY)
    {
      auto *ground = _world.getComponent<ecs::GroundDetectorComponent>(entity);
      if (ground)
      {
        ground->isOnGround = false;
      }
    }
  };

  // 添加到场景级别，只需一个监听器
  _eventDispatcher->addEventListenerWithSceneGraphPriority(
      _sharedContactListener, this);

  CCLOG("EcsTestScene: Shared contact listener initialized");
}

void EcsTestScene::menuBackCallback(Ref *pSender)
{
  // 返回主菜单
  auto mainMenuScene = MainMenuScene::createScene();
  Director::getInstance()->replaceScene(
      TransitionFade::create(0.5f, mainMenuScene));
}

void EcsTestScene::setupKeyboardListener()
{
  auto keyboardListener = EventListenerKeyboard::create();

  keyboardListener->onKeyPressed = [this](EventKeyboard::KeyCode keyCode,
                                          Event *event)
  {
    _keysPressed[keyCode] = true;

    if (keyCode == EventKeyboard::KeyCode::KEY_R)
    {
      toggleFakePlayer();
    }
  };

  keyboardListener->onKeyReleased = [this](EventKeyboard::KeyCode keyCode,
                                           Event *event)
  {
    _keysPressed[keyCode] = false;
  };

  _eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListener,
                                                           this);
}

void EcsTestScene::update(float delta)
{
  Layer::update(delta);

  // 更新虚拟玩家位置
  updateFakePlayerPosition(delta);

  // 更新ECS世界
  _world.update(delta);
}

void EcsTestScene::updateFakePlayerPosition(float delta)
{
  if (!_fakePlayerVisible || _fakePlayer == nullptr)
  {
    return;
  }

  float moveSpeed = 200.0f;
  Vec2 movement(0, 0);

  // WASD控制
  if (_keysPressed[EventKeyboard::KeyCode::KEY_W] ||
      _keysPressed[EventKeyboard::KeyCode::KEY_UP_ARROW])
  {
    movement.y += moveSpeed * delta;
  }
  if (_keysPressed[EventKeyboard::KeyCode::KEY_S] ||
      _keysPressed[EventKeyboard::KeyCode::KEY_DOWN_ARROW])
  {
    movement.y -= moveSpeed * delta;
  }
  if (_keysPressed[EventKeyboard::KeyCode::KEY_A] ||
      _keysPressed[EventKeyboard::KeyCode::KEY_LEFT_ARROW])
  {
    movement.x -= moveSpeed * delta;
  }
  if (_keysPressed[EventKeyboard::KeyCode::KEY_D] ||
      _keysPressed[EventKeyboard::KeyCode::KEY_RIGHT_ARROW])
  {
    movement.x += moveSpeed * delta;
  }

  if (movement.x != 0 || movement.y != 0)
  {
    Vec2 newPos = _fakePlayer->getPosition() + movement;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    newPos.x =
        std::max(origin.x + 20.0f,
                 std::min(newPos.x, origin.x + visibleSize.width - 20.0f));
    newPos.y =
        std::max(origin.y + 20.0f,
                 std::min(newPos.y, origin.y + visibleSize.height - 20.0f));

    _fakePlayer->setPosition(newPos);

    if (_playerLabel != nullptr)
    {
      _playerLabel->setPosition(
          Vec2(_fakePlayer->getPositionX(), _fakePlayer->getPositionY() + 40));
    }

    // 同步到ECS世界中的玩家实体
    if (_fakePlayerEntity != ecs::INVALID_ENTITY)
    {
      auto *transform =
          _world.getComponent<ecs::TransformComponent>(_fakePlayerEntity);
      if (transform)
      {
        transform->position = newPos;
      }
    }
  }
}

void EcsTestScene::toggleFakePlayer()
{
  _fakePlayerVisible = !_fakePlayerVisible;

  if (_fakePlayerVisible)
  {
    _fakePlayer->setVisible(true);
    if (_playerLabel != nullptr)
    {
      _playerLabel->setVisible(true);
    }
    CCLOG("EcsTestScene: Fake Player Visible");
  }
  else
  {
    _fakePlayer->setVisible(false);
    if (_playerLabel != nullptr)
    {
      _playerLabel->setVisible(false);
    }
    CCLOG("EcsTestScene: Fake Player Hidden");
  }
}
