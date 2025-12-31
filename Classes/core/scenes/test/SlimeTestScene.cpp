#include "SlimeTestScene.h"
#include "core/scenes/MainMenuScene.h"
#include "core/factory/monster/MonsterMasterFactory.h"
#include "components/render/SpriteComponent.h"
#include "systems/physics/PhysicsContactHandler.h"

USING_NS_CC;

Scene *SlimeTestScene::createScene()
{
  auto scene = Scene::createWithPhysics();
  auto physicsWorld = scene->getPhysicsWorld();
  physicsWorld->setGravity(Vec2(0, -980));

  //调试物理体
  // physicsWorld->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);

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
  auto &factory = MonsterMasterFactory::getInstance();
  (void)factory; // 工厂在构造时已加载史莱姆配置

  // ==================== 使用EnTTSystems（史莱姆专用）====================
  CCLOG("========== Setting up EnTT Systems for Slimes ==========");
  
  _systemManager.setRegistry(&_registry);
  
  // 注册精灵销毁观察者（新架构）
  ecs::SpriteDestructionObserver::registerToRegistry(_registry);
  
  // 按优先级顺序添加Systems（仅史莱姆相关）
  _systemManager.addSystem<ecs::AggroSystemEntt>();
  _systemManager.addSystem<ecs::GroundDetectorSystemEntt>();
  _systemManager.addSystem<ecs::SlowFallSystemEntt>();
  _systemManager.addSystem<ecs::JumpMovementSystemEntt>();
  _systemManager.addSystem<ecs::ProjectileAttackSystemEntt>();
  _systemManager.addSystem<ecs::ProjectileSystemEntt>();
  _projectileCollisionSystem = _systemManager.addSystem<ecs::ProjectileCollisionSystemEntt>();
  _systemManager.addSystem<ecs::DebuffSystemEntt>();
  
  // 新架构：使用RenderSystem和AnimationSystem替代旧的渲染系统
  _systemManager.addSystem<ecs::RenderSystem>();
  _systemManager.addSystem<ecs::AnimationSystem>();
  _systemManager.addSystem<ecs::PhysicsSyncSystemEntt>();  // 统一物理同步系统
  
  _systemManager.addSystem<ecs::HealthSystemEntt>();
  _systemManager.addSystem<ecs::CombatSystemEntt>();
  _systemManager.addSystem<ecs::LifetimeSystemEntt>();

  CCLOG("EnTT Systems initialized: %zu systems", _systemManager.getSystemCount());
  CCLOG("==========================================");
}

void SlimeTestScene::createPhysicsEnvironment()
{
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();
  // 统一墙壁材质：适度弹性支持各类怪物
  PhysicsMaterial wallMaterial(1.0f, 0.3f, 0.0f);

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

  // 统一地面材质：中等摩擦力+零弹性
  PhysicsMaterial groundMaterial(1.0f, 0.0f, 2.0f);  // density, restitution, friction
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

  // 创建玩家实体
  auto playerEntity = _registry.create();
  
  auto& transform = _registry.emplace<ecs::TransformComponent>(playerEntity);
  transform.position.x = _fakePlayer->getPositionX();
  transform.position.y = _fakePlayer->getPositionY();

  auto &health = _registry.emplace<ecs::HealthComponent>(playerEntity);
  health.maxHealth = 1000.0f;
  health.currentHealth = 1000.0f;
  health.invincibleTime = 0.3f;
  
  _registry.emplace<ecs::PlayerTag>(playerEntity);
  
  _fakePlayerEntity = entt::to_integral(playerEntity);
  
  // 注册到NodeEntityMap (让史莱姆能锁定玩家)
  ecs::NodeEntityMap::getInstance().registerNode(_fakePlayer, _fakePlayerEntity);
  
  CCLOG("SlimeTestScene: Fake player created (EnTT entity %u) and registered to NodeEntityMap", _fakePlayerEntity);
}


//在测试场景中生成各种史莱姆
void SlimeTestScene::createEcsSlime()
{
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();
  auto &factory = MonsterMasterFactory::getInstance();

  CCLOG("SlimeTestScene: visibleSize=(%.1f, %.1f), origin=(%.1f, %.1f)",
        visibleSize.width, visibleSize.height, origin.x, origin.y);

  const char *slimeTypes[] = {
      "GreenSlime", "BlueSlime", "RedSlime",
      "YellowSlime", "PurpleSlime", "PinkSlime", "IceSlime",
      "SpikedSlime", "SpikedIceSlime", "SpikedJungleSlime", "UmbrellaSlime", "MotherSlime", "BabySlime"};
  int slimeCount = sizeof(slimeTypes) / sizeof(slimeTypes[0]);

  float groundTop = origin.y + 50.0f;
  CCLOG("SlimeTestScene: groundTop=%.1f, will spawn %d slimes", groundTop, slimeCount);

  for (int i = 0; i < slimeCount; i++)
  {
    float X = origin.x + 100.0f + i * (visibleSize.width - 200.0f) / (slimeCount - 1);
    float Y = groundTop + 100.0f;
    CCLOG("SlimeTestScene: Spawning %s at (%.1f, %.1f) [i=%d]", slimeTypes[i], X, Y, i);
    factory.createMonster(_registry, slimeTypes[i], X, Y, this);
  }
  CCLOG("SlimeTestScene: All slimes spawned (EnTT version)");
}

void SlimeTestScene::setupSharedContactListener()
{
  // 使用PhysicsContactHandler创建碰撞监听器，使用ProjectileCollisionSystemEntt处理投射物碰撞
  _sharedContactListener = ecs::PhysicsContactHandler::createContactListener(
    _registry,
    // 碰撞开始处理：使用ProjectileCollisionSystemEntt处理投射物
    [this](PhysicsContact& contact, const ecs::PhysicsContactHandler::ContactInfo& info) -> bool {
      auto bodyA = contact.getShapeA()->getBody();
      auto bodyB = contact.getShapeB()->getBody();
      Node *nodeA = bodyA->getNode();
      Node *nodeB = bodyB->getNode();
      
      ecs::EntityId entityA = nodeA ? ecs::NodeEntityMap::getInstance().findEntity(nodeA) : ecs::INVALID_ENTITY;
      ecs::EntityId entityB = nodeB ? ecs::NodeEntityMap::getInstance().findEntity(nodeB) : ecs::INVALID_ENTITY;
      
      // 投射物碰撞处理
      ecs::ProjectileComponent *projA = nullptr;
      ecs::ProjectileComponent *projB = nullptr;
      
      if (entityA != ecs::INVALID_ENTITY) {
        auto entA = static_cast<entt::entity>(entityA);
        if (_registry.valid(entA)) projA = _registry.try_get<ecs::ProjectileComponent>(entA);
      }
      if (entityB != ecs::INVALID_ENTITY) {
        auto entB = static_cast<entt::entity>(entityB);
        if (_registry.valid(entB)) projB = _registry.try_get<ecs::ProjectileComponent>(entB);
      }
      
      if (projA || projB) {
        ecs::ProjectileComponent *proj = projA ? projA : projB;
        ecs::EntityId otherEntity = projA ? entityB : entityA;
        PhysicsBody *otherBody = projA ? bodyB : bodyA;
        entt::entity projEntity = projA ? static_cast<entt::entity>(entityA) : static_cast<entt::entity>(entityB);
        
        // 使用ProjectileCollisionSystemEntt处理碰撞
        if (_projectileCollisionSystem) {
          const auto* data = contact.getContactData();
          const cocos2d::Vec2* hitPos = (data && data->count > 0) ? &data->points[0] : nullptr;
          if (_projectileCollisionSystem->handleProjectileCollision(proj, projEntity, otherEntity, otherBody, hitPos)) {
            return true;
          }
        }
      }
      return true;
    },
    nullptr  // 无自定义分离处理
  );

  _eventDispatcher->addEventListenerWithSceneGraphPriority(_sharedContactListener, this);
  CCLOG("SlimeTestScene: Contact listener initialized (using ProjectileCollisionSystemEntt)");
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
  _systemManager.update(delta);
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
    // 同步玩家位置到EnTT registry
    if (_fakePlayerEntity != ecs::INVALID_ENTITY) {
      auto playerEntity = static_cast<entt::entity>(_fakePlayerEntity);
      if (_registry.valid(playerEntity)) {
        auto* transform = _registry.try_get<ecs::TransformComponent>(playerEntity);
        if (transform) {
          transform->position = newPos;
        }
      }
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
