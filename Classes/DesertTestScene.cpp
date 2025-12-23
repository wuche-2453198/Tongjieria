#include "DesertTestScene.h"
#include "MainMenuScene.h"
#include "MonsterFactory.h"
#include "ecs/systems/PhysicsContactHandler.h"

USING_NS_CC;

Scene *DesertTestScene::createScene()
{
  auto scene = Scene::createWithPhysics();
  auto physicsWorld = scene->getPhysicsWorld();
  physicsWorld->setGravity(Vec2(0, -980));

  // 调试物理体
  // physicsWorld->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);

  physicsWorld->setSpeed(1.0f);
  physicsWorld->setSubsteps(8);

  auto layer = DesertTestScene::create();
  scene->addChild(layer);
  return scene;
}

bool DesertTestScene::init()
{
  if (!Layer::init())
  {
    return false;
  }

  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 创建背景（沙漠色调，黄褐色）- 设为最底层
  auto background = LayerColor::create(Color4B(194, 154, 108, 255)); // 沙漠黄褐色
  this->addChild(background, -10);  // 背景在最底层

  // 添加标题
  auto titleLabel = Label::createWithTTF("Desert Test Scene",
                                         "fonts/Marker Felt.ttf", 32);
  if (titleLabel != nullptr)
  {
    titleLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                 origin.y + visibleSize.height -
                                     titleLabel->getContentSize().height - 20));
    titleLabel->setColor(Color3B(139, 69, 19)); // 深褐色文字
    this->addChild(titleLabel, 1);
  }

  // 添加说明
  auto infoLabel = Label::createWithTTF(
      "Desert Monsters: Various desert creatures and their behaviors",
      "fonts/Marker Felt.ttf", 16);
  if (infoLabel != nullptr)
  {
    infoLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                origin.y + visibleSize.height - 60));
    infoLabel->setColor(Color3B(160, 82, 45)); // 沙色文字
    this->addChild(infoLabel, 1);
  }

  // 添加返回按钮
  auto backLabel =
      Label::createWithTTF("Back to Menu", "fonts/Marker Felt.ttf", 24);
  auto backItem = MenuItemLabel::create(
      backLabel, CC_CALLBACK_1(DesertTestScene::menuBackCallback, this));

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
  createDesertMonsters();
  setupKeyboardListener();
  this->scheduleUpdate();

  CCLOG("DesertTestScene: Initialized");
  return true;
}

void DesertTestScene::setupEcsSystems()
{
  auto &factory = MonsterFactory::getInstance();
  factory.clearConfigs();  // 清空之前场景的配置
  // 加载沙漠怪物配置（准备用于用户设计的沙漠怪物）
  factory.loadConfigsFromDir("config/desert");

  // 注册沙球射弹精灵资源（匹配AntlionAISystemEntt中的大写ID）
  ecs::SpriteResourceDescriptor sandBallDesc;
  sandBallDesc.resourceId = "Sand_Ball";
  sandBallDesc.anchorPoint = Vec2(0.5f, 0.5f);
  
  // 检查射弹图片是否存在
  std::string sandBallPath = FileUtils::getInstance()->fullPathForFilename("Projectile/Sand_Ball.png");
  if (!sandBallPath.empty() && FileUtils::getInstance()->isFileExist(sandBallPath)) {
    sandBallDesc.spritePath = "Projectile/Sand_Ball.png";
    CCLOG("Registered Sand_Ball projectile from file");
  } else {
    // 备用方案：创建黄色圆形作为射弹（使用RenderTexture）
    CCLOG("Sand_Ball.png not found, creating yellow circle fallback...");
    
    auto renderTexture = RenderTexture::create(16, 16);
    renderTexture->begin();
    
    // 绘制黄色实心圆
    auto drawNode = DrawNode::create();
    drawNode->drawSolidCircle(Vec2(8, 8), 7.0f, 0, 16, Color4F(1.0f, 0.78f, 0.0f, 1.0f));
    drawNode->visit(Director::getInstance()->getRenderer(), Mat4::IDENTITY, 0);
    
    renderTexture->end();
    
    // 保存为临时文件供SpriteManager使用
    std::string savePath = FileUtils::getInstance()->getWritablePath() + "sand_ball_fallback.png";
    renderTexture->saveToFile("sand_ball_fallback.png", cocos2d::Image::Format::PNG);
    
    sandBallDesc.spritePath = savePath;
    CCLOG("Created FALLBACK yellow circle saved to: %s", savePath.c_str());
  }
  
  ecs::SpriteManager::getInstance().registerResource(sandBallDesc);
  CCLOG("Registered Sand_Ball projectile sprite resource");

  CCLOG("========== Setting up EnTT Systems for Desert ==========");

  _systemManager.setRegistry(&_registry);

  // 按优先级顺序添加Systems
  _systemManager.addSystem<ecs::AggroSystemEntt>();  // 必须：追踪玩家
  
  // 添加各种AI系统，支持不同类型的怪物
  _systemManager.addSystem<ecs::EaterOfSoulsAISystemEntt>(); // 支持噬魂怪类型
  _systemManager.addSystem<ecs::DemonEyeAISystemEntt>();     // 支持飞行类型
  _systemManager.addSystem<ecs::WarriorAISystemEntt>();      // 支持近战类型
  _systemManager.addSystem<ecs::KingSlimeAISystemEntt>();    // 支持史莱姆类型
  _systemManager.addSystem<ecs::AntlionAISystemEntt>();      // 支持蚁狮类型
  
  // 新架构：使用RenderSystem
  _systemManager.addSystem<ecs::RenderSystem>();
  _systemManager.addSystem<ecs::AnimationSystem>(); // 动画系统
  _systemManager.addSystem<ecs::MonsterSyncSystemEntt>();  // 同步物理位置
  
  // 射弹系统（蚁狮需要）
  _systemManager.addSystem<ecs::ProjectileSystemEntt>();
  _projectileCollisionSystem = _systemManager.addSystem<ecs::ProjectileCollisionSystemEntt>();
  
  // 基础系统
  _systemManager.addSystem<ecs::HealthSystemEntt>();
  _systemManager.addSystem<ecs::CombatSystemEntt>();
  _systemManager.addSystem<ecs::LifetimeSystemEntt>();
  _systemManager.addSystem<ecs::DebuffSystemEntt>();        // 减益系统
  _systemManager.addSystem<ecs::SlowFallSystemEntt>();      // 缓降系统
  _systemManager.addSystem<ecs::JumpMovementSystemEntt>();  // 跳跃移动
  _systemManager.addSystem<ecs::GroundDetectorSystemEntt>(); // 地面检测
  
  // 注册Sprite销毁监听器
  ecs::SpriteDestructionObserver::registerToRegistry(_registry);

  CCLOG("EnTT Systems initialized: %zu systems", _systemManager.getSystemCount());
  CCLOG("==========================================");
}

void DesertTestScene::createPhysicsEnvironment()
{
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 沙漠风格墙壁材质：适度弹性支持各类怪物
  PhysicsMaterial wallMaterial(1.0f, 0.3f, 0.0f);

  // 左边界（沙漠石壁）
  auto leftWall = Sprite::create();
  leftWall->setTextureRect(Rect(0, 0, 10, visibleSize.height));
  leftWall->setColor(Color3B(205, 133, 63)); // 沙漠石色
  leftWall->setPosition(Vec2(origin.x + 5, origin.y + visibleSize.height / 2));
  this->addChild(leftWall, 0);
  auto leftWallBody = PhysicsBody::createBox(leftWall->getContentSize(), wallMaterial);
  leftWallBody->setDynamic(false);
  leftWallBody->setCategoryBitmask(0x0001);
  leftWallBody->setContactTestBitmask(0xFFFFFFFF);
  leftWallBody->setCollisionBitmask(0xFFFFFFFF);
  leftWallBody->setGroup(0);
  leftWall->setPhysicsBody(leftWallBody);

  // 右边界（沙漠石壁）
  auto rightWall = Sprite::create();
  rightWall->setTextureRect(Rect(0, 0, 10, visibleSize.height));
  rightWall->setColor(Color3B(205, 133, 63)); // 沙漠石色
  rightWall->setPosition(Vec2(origin.x + visibleSize.width - 5,
                              origin.y + visibleSize.height / 2));
  this->addChild(rightWall, 0);
  auto rightWallBody = PhysicsBody::createBox(rightWall->getContentSize(), wallMaterial);
  rightWallBody->setDynamic(false);
  rightWallBody->setCategoryBitmask(0x0001);
  rightWallBody->setContactTestBitmask(0xFFFFFFFF);
  rightWallBody->setCollisionBitmask(0xFFFFFFFF);
  rightWall->setPhysicsBody(rightWallBody);

  // 上边界（天空）
  auto topWall = Sprite::create();
  topWall->setTextureRect(Rect(0, 0, visibleSize.width, 10));
  topWall->setColor(Color3B(135, 206, 235)); // 天空蓝色
  topWall->setPosition(Vec2(origin.x + visibleSize.width / 2,
                            origin.y + visibleSize.height - 5));
  this->addChild(topWall, 0);
  auto topWallBody = PhysicsBody::createBox(topWall->getContentSize(), wallMaterial);
  topWallBody->setDynamic(false);
  topWallBody->setCategoryBitmask(0x0001);
  topWallBody->setContactTestBitmask(0xFFFFFFFF);
  topWallBody->setCollisionBitmask(0xFFFFFFFF);
  topWall->setPhysicsBody(topWallBody);

  // 沙漠地面材质：中等摩擦力+零弹性
  PhysicsMaterial groundMaterial(1.0f, 0.0f, 2.0f);
  auto ground = Sprite::create();
  ground->setTextureRect(Rect(0, 0, visibleSize.width, 50));
  ground->setColor(Color3B(238, 203, 173)); // 沙色地面
  ground->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + 25));
  this->addChild(ground, 0);  // 地形在中间层
  auto groundBody = PhysicsBody::createBox(ground->getContentSize(), groundMaterial);
  groundBody->setDynamic(false);
  groundBody->setCategoryBitmask(0x0001);
  groundBody->setContactTestBitmask(0xFFFFFFFF);
  groundBody->setCollisionBitmask(0xFFFFFFFF);
  groundBody->setGroup(0);
  ground->setPhysicsBody(groundBody);

  // 沙漠障碍物：沙丘和岩石
  auto sandDune1 = Sprite::create();
  sandDune1->setTextureRect(Rect(0, 0, 120, 60));
  sandDune1->setColor(Color3B(218, 165, 32)); // 金黄色沙丘
  sandDune1->setPosition(Vec2(origin.x + visibleSize.width / 4,
                              origin.y + 110));
  this->addChild(sandDune1, 0);  // 沙丘也在地形层
  auto sandDune1Body = PhysicsBody::createBox(sandDune1->getContentSize(), wallMaterial);
  sandDune1Body->setDynamic(false);
  sandDune1Body->setCategoryBitmask(0x0001);
  sandDune1Body->setContactTestBitmask(0xFFFFFFFF);
  sandDune1Body->setCollisionBitmask(0xFFFFFFFF);
  sandDune1->setPhysicsBody(sandDune1Body);

  auto rock1 = Sprite::create();
  rock1->setTextureRect(Rect(0, 0, 80, 100));
  rock1->setColor(Color3B(160, 82, 45)); // 沙漠岩石色
  rock1->setPosition(Vec2(origin.x + visibleSize.width / 2,
                          origin.y + visibleSize.height / 2));
  this->addChild(rock1, 2);
  auto rock1Body = PhysicsBody::createBox(rock1->getContentSize(), wallMaterial);
  rock1Body->setDynamic(false);
  rock1Body->setCategoryBitmask(0x0001);
  rock1Body->setContactTestBitmask(0xFFFFFFFF);
  rock1Body->setCollisionBitmask(0xFFFFFFFF);
  rock1->setPhysicsBody(rock1Body);

  auto sandDune2 = Sprite::create();
  sandDune2->setTextureRect(Rect(0, 0, 100, 80));
  sandDune2->setColor(Color3B(210, 180, 140)); // 淡沙色
  sandDune2->setPosition(Vec2(origin.x + visibleSize.width * 3 / 4,
                              origin.y + visibleSize.height / 3));
  this->addChild(sandDune2, 2);
  auto sandDune2Body = PhysicsBody::createBox(sandDune2->getContentSize(), wallMaterial);
  sandDune2Body->setDynamic(false);
  sandDune2Body->setCategoryBitmask(0x0001);
  sandDune2Body->setContactTestBitmask(0xFFFFFFFF);
  sandDune2Body->setCollisionBitmask(0xFFFFFFFF);
  sandDune2->setPhysicsBody(sandDune2Body);

  CCLOG("DesertTestScene: Physics environment created");
}

void DesertTestScene::createFakePlayerEntity()
{
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  _fakePlayer = Sprite::create();
  _fakePlayer->setTextureRect(Rect(0, 0, 30, 50));
  _fakePlayer->setColor(Color3B(255, 200, 100)); // 玩家颜色
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
    _playerLabel->setColor(Color3B(139, 69, 19)); // 深褐色标签
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

  CCLOG("DesertTestScene: Fake player created (EnTT entity %u)", _fakePlayerEntity);
}

void DesertTestScene::createDesertMonsters()
{
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  CCLOG("DesertTestScene: Creating desert monsters...");

  auto &factory = MonsterFactory::getInstance();
  
  // 创建蚁狮怪物
  const char* desertMonsters[] = {
    "Antlion"
  };
  const int monsterCount = 1;
  
  for (int i = 0; i < monsterCount; i++) {
    // 蚁狮放置在地面上的固定位置（用于测试）
    float x = origin.x + visibleSize.width / 2;
    float y = origin.y + 80; // 略高于地面，让蚁狮"埋"在地下
    
    ecs::EntityId entityId = factory.createMonster(_registry, desertMonsters[0], x, y, this);
    
    if (entityId != ecs::INVALID_ENTITY) {
      CCLOG("DesertTestScene: Created %s at (%.1f, %.1f), entity=%u",
            desertMonsters[0], x, y, entityId);
    } else {
      CCLOG("DesertTestScene: Failed to create %s", desertMonsters[0]);
    }
  }

  CCLOG("DesertTestScene: Created %d desert monsters", monsterCount);
}

void DesertTestScene::setupSharedContactListener()
{
  // 碰撞监听器：处理沙漠怪物和射弹碰撞
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

        // 射弹碰撞处理（参考SlimeTestScene）
        if (info.isValid) {
          // 检查是否有射弹参与碰撞
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
          
          // 如果有射弹，使用ProjectileCollisionSystemEntt处理
          if (projA || projB) {
            ecs::ProjectileComponent *proj = projA ? projA : projB;
            ecs::EntityId otherEntity = projA ? entityB : entityA;
            PhysicsBody *otherBody = projA ? bodyB : bodyA;
            entt::entity projEntity = projA ? static_cast<entt::entity>(entityA) : static_cast<entt::entity>(entityB);
            
            if (_projectileCollisionSystem) {
              if (_projectileCollisionSystem->handleProjectileCollision(proj, projEntity, otherEntity, otherBody)) {
                return true;
              }
            }
          }
        }

        return true;
      },
      nullptr // 无分离处理
  );

  _eventDispatcher->addEventListenerWithSceneGraphPriority(_sharedContactListener, this);
  CCLOG("DesertTestScene: Contact listener initialized with projectile collision handling");
}

void DesertTestScene::menuBackCallback(Ref *pSender)
{
  auto mainMenuScene = MainMenuScene::createScene();
  Director::getInstance()->replaceScene(TransitionFade::create(0.5f, mainMenuScene));
}

void DesertTestScene::setupKeyboardListener()
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

void DesertTestScene::update(float delta)
{
  Layer::update(delta);
  updateFakePlayerPosition(delta);
  _systemManager.update(delta);
}

void DesertTestScene::updateFakePlayerPosition(float delta)
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

void DesertTestScene::toggleFakePlayer()
{
  _fakePlayerVisible = !_fakePlayerVisible;
  _fakePlayer->setVisible(_fakePlayerVisible);
  if (_playerLabel)
    _playerLabel->setVisible(_fakePlayerVisible);
  CCLOG("DesertTestScene: Fake Player %s", _fakePlayerVisible ? "Visible" : "Hidden");
}
