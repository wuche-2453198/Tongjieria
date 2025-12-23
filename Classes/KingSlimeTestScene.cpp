#include "KingSlimeTestScene.h"
#include "MainMenuScene.h"
#include "MonsterFactory.h"
#include "ecs/components/SpriteComponent.h"
#include "ecs/systems/PhysicsContactHandler.h"
#include "ui/CocosGUI.h"
#include <entt/entt.hpp>

USING_NS_CC;

Scene *KingSlimeTestScene::createScene()
{
  auto scene = Scene::createWithPhysics();
  auto physicsWorld = scene->getPhysicsWorld();
  physicsWorld->setGravity(Vec2(0, -980));

  // 调试物理体
  // physicsWorld->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);

  // 提高物理引擎精度
  physicsWorld->setSpeed(1.0f);
  physicsWorld->setSubsteps(8);

  auto layer = KingSlimeTestScene::create();
  scene->addChild(layer);
  return scene;
}

bool KingSlimeTestScene::init()
{
  if (!Layer::init())
  {
    return false;
  }

  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 初始化参数
  _fakePlayerVisible = true;
  _playerOnGround = false;
  _playerGroundContactCount = 0;
  _playerMoveSpeed = 300.0f;
  _playerJumpForce = 600.0f;
  _damageButton = nullptr;
  
  // Boss相关初始化
  _kingSlimeBossEntity = ecs::INVALID_ENTITY;
  _bossNameLabel = nullptr;
  _bossHealthBarBg = nullptr;
  _bossHealthBarFg = nullptr;

  // 创建背景（Boss战专用深色背景）
  auto background = LayerColor::create(Color4B(25, 25, 35, 255));
  this->addChild(background, 0);

  // 添加标题
  auto titleLabel = Label::createWithTTF("King Slime Test Arena",
                                         "fonts/Marker Felt.ttf", 36);
  if (titleLabel != nullptr)
  {
    titleLabel->setColor(Color3B(220, 180, 100));
    titleLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                 origin.y + visibleSize.height - 40));
    this->addChild(titleLabel, 10);  // 高层级，始终可见
  }

  // 添加操作说明
  auto infoLabel = Label::createWithTTF(
      "WASD: Move  SPACE: Jump  Left Click: Shoot (100 DMG)  ESC: Back to Menu",
      "fonts/Marker Felt.ttf", 16);
  if (infoLabel != nullptr)
  {
    infoLabel->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                origin.y + visibleSize.height - 70));
    infoLabel->setColor(Color3B(180, 180, 200));
    this->addChild(infoLabel, 10);
  }

  createPhysicsEnvironment();
  setupEcsSystems();
  setupSharedContactListener();
  createFakePlayerEntity();
  setupInputListeners();
  
  // 延迟1秒生成Boss，确保所有系统已初始化
  this->scheduleOnce([this](float dt) {
    spawnKingSlimeBoss();
    createDamageButton();  // 创建伤害按钮
  }, 1.0f, "spawn_boss");
  
  this->scheduleUpdate();

  CCLOG("KingSlimeTestScene: Initialized with camera follow");
  return true;
}

void KingSlimeTestScene::setupEcsSystems()
{
  CCLOG("========== Setting up EnTT Systems for King Slime Arena ==========");
  
  _systemManager.setRegistry(&_registry);
  
  // 基础系统
  _systemManager.addSystem<ecs::HealthSystemEntt>();
  _systemManager.addSystem<ecs::CombatSystemEntt>();
  _systemManager.addSystem<ecs::LifetimeSystemEntt>();
  
  // 怪物AI和移动系统
  _systemManager.addSystem<ecs::JumpMovementSystemEntt>();
  _systemManager.addSystem<ecs::AggroSystemEntt>();
  _systemManager.addSystem<ecs::GroundDetectorSystemEntt>();
  
  // 投射物系统（用于尖刺史莱姆的射弹）
  _systemManager.addSystem<ecs::ProjectileAttackSystemEntt>();
  _systemManager.addSystem<ecs::ProjectileSystemEntt>();
  
  // 投射物碰撞系统（处理投射物与障碍物、目标的碰撞）
  _projectileCollisionSystem = _systemManager.addSystem<ecs::ProjectileCollisionSystemEntt>();
  
  // 减益系统
  _systemManager.addSystem<ecs::DebuffSystemEntt>();
  
  // 新架构：使用RenderSystem和AnimationSystem
  _systemManager.addSystem<ecs::RenderSystem>();
  _systemManager.addSystem<ecs::AnimationSystem>();
  _systemManager.addSystem<ecs::SlimeSyncSystemEntt>();  // 保留用于物理同步
  
  // 注册精灵销毁观察者
  ecs::SpriteDestructionObserver::registerToRegistry(_registry);
  
  // 史莱姆王专用AI系统
  auto* kingSlimeAI = _systemManager.addSystem<ecs::KingSlimeAISystemEntt>();
  
  // 设置场景上下文，让系统能够直接创建史莱姆
  if (kingSlimeAI) {
    kingSlimeAI->setSceneContext(this);
  }
  
  CCLOG("EnTT Systems initialized: %zu systems (including KingSlimeAI)", _systemManager.getSystemCount());
  CCLOG("===========================================");
}

void KingSlimeTestScene::createPhysicsEnvironment()
{
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();
  
  // 创建一个超大的平地arena（比视窗大3倍）
  float arenaWidth = visibleSize.width * 3.0f;
  float arenaHeight = 100.0f;
  
  // 统一地面材质：中等摩擦力+零弹性
  PhysicsMaterial groundMaterial(1.0f, 0.0f, 2.0f);
  
  auto ground = Sprite::create();
  ground->setTextureRect(Rect(0, 0, arenaWidth, arenaHeight));
  ground->setColor(Color3B(60, 40, 80));  // 深紫色地面
  ground->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + arenaHeight / 2));
  this->addChild(ground, 0);
  
  auto groundBody = PhysicsBody::createBox(ground->getContentSize(), groundMaterial);
  groundBody->setDynamic(false);
  groundBody->setCategoryBitmask(0x0001);     // 地形类别
  groundBody->setContactTestBitmask(0xFFFFFFFF);
  groundBody->setCollisionBitmask(0xFFFFFFFF);
  groundBody->setGroup(0);
  ground->setPhysicsBody(groundBody);
  
  CCLOG("KingSlimeTestScene: Created large arena ground (%.1fx%.1f)", arenaWidth, arenaHeight);
}

void KingSlimeTestScene::createFakePlayerEntity()
{
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  _fakePlayer = Sprite::create();
  _fakePlayer->setTextureRect(Rect(0, 0, 32, 48));
  _fakePlayer->setColor(Color3B(100, 150, 255));
  _fakePlayer->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + 200));
  this->addChild(_fakePlayer, 5);

  // 玩家物理体 - 使用零摩擦防止粘滞
  PhysicsMaterial playerMaterial(1.0f, 0.0f, 0.0f);
  auto playerBody = PhysicsBody::createBox(Size(28, 44), playerMaterial, Vec2(0, 0));
  playerBody->setDynamic(true);
  playerBody->setMass(1.0f);
  playerBody->setRotationEnable(false);
  playerBody->setVelocityLimit(600.0f);
  playerBody->setCategoryBitmask(0x0004);        // 玩家类别
  playerBody->setContactTestBitmask(0xFFFFFFFF);
  playerBody->setCollisionBitmask(0xFFFFFFFF);
  playerBody->setGroup(0);
  _fakePlayer->setPhysicsBody(playerBody);

  _playerLabel = Label::createWithTTF("Player", "fonts/Marker Felt.ttf", 14);
  if (_playerLabel)
  {
    _playerLabel->setPosition(Vec2(_fakePlayer->getPositionX(), _fakePlayer->getPositionY() + 35));
    this->addChild(_playerLabel, 6);
  }

  // 创建玩家ECS实体
  auto playerEntity = _registry.create();
  
  auto& transform = _registry.emplace<ecs::TransformComponent>(playerEntity);
  transform.position = _fakePlayer->getPosition();
  
  auto& health = _registry.emplace<ecs::HealthComponent>(playerEntity);
  health.maxHealth = 1000.0f;
  health.currentHealth = 1000.0f;
  
  _registry.emplace<ecs::PlayerTag>(playerEntity);
  
  _fakePlayerEntity = entt::to_integral(playerEntity);
  ecs::NodeEntityMap::getInstance().registerNode(_fakePlayer, _fakePlayerEntity);
  
  CCLOG("KingSlimeTestScene: Player created with 1000 HP at arena center");
}

void KingSlimeTestScene::setupSharedContactListener()
{
  _sharedContactListener = ecs::PhysicsContactHandler::createContactListener(
    _registry,
    // 碰撞开始处理：玩家地面检测 + 子弹伤害处理
    [this](PhysicsContact& contact, const ecs::PhysicsContactHandler::ContactInfo& info) -> bool {
      // 玩家地面检测
      if (info.isValid && info.isGroundContact && info.dynamicNode == _fakePlayer) {
        _playerOnGround = true;
        _playerGroundContactCount++;
      }
      
      // 投射物碰撞处理（使用ProjectileCollisionSystemEntt）
      if (info.isValid) {
        // 直接从contact获取body和node，确保顺序一致
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
            if (_projectileCollisionSystem->handleProjectileCollision(proj, projEntity, otherEntity, otherBody)) {
              return true;
            }
          }
        }
      }
      
      return true;
    },
    // 碰撞分离处理：玩家地面检测
    [this](PhysicsContact& contact, const ecs::PhysicsContactHandler::ContactInfo& info) {
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

void KingSlimeTestScene::setupInputListeners()
{
  // 键盘监听
  _keyboardListener = EventListenerKeyboard::create();
  _keyboardListener->onKeyPressed = CC_CALLBACK_2(KingSlimeTestScene::onKeyPressed, this);
  _keyboardListener->onKeyReleased = CC_CALLBACK_2(KingSlimeTestScene::onKeyReleased, this);
  _eventDispatcher->addEventListenerWithSceneGraphPriority(_keyboardListener, this);
  
  // 不再需要鼠标监听
}

void KingSlimeTestScene::update(float delta)
{
  Layer::update(delta);
  updateFakePlayerMovement(delta);
  updateCameraFollow();
  updateBossHealthBar();
  _systemManager.update(delta);
}

void KingSlimeTestScene::updateFakePlayerMovement(float delta)
{
  if (!_fakePlayerVisible || !_fakePlayer) return;
  
  auto body = _fakePlayer->getPhysicsBody();
  if (!body) return;
  
  Vec2 velocity = body->getVelocity();
  
  // 水平移动
  float targetVelX = 0.0f;
  if (_keysPressed[EventKeyboard::KeyCode::KEY_A] || _keysPressed[EventKeyboard::KeyCode::KEY_LEFT_ARROW])
    targetVelX = -_playerMoveSpeed;
  if (_keysPressed[EventKeyboard::KeyCode::KEY_D] || _keysPressed[EventKeyboard::KeyCode::KEY_RIGHT_ARROW])
    targetVelX = _playerMoveSpeed;
  
  velocity.x = targetVelX;
  
  // 跳跃
  if (_playerOnGround && 
      (_keysPressed[EventKeyboard::KeyCode::KEY_SPACE] || 
       _keysPressed[EventKeyboard::KeyCode::KEY_W] ||
       _keysPressed[EventKeyboard::KeyCode::KEY_UP_ARROW])) {
    velocity.y = _playerJumpForce;
    _playerOnGround = false;
  }
  
  body->setVelocity(velocity);
  
  // 更新标签位置
  if (_playerLabel) {
    _playerLabel->setPosition(Vec2(_fakePlayer->getPositionX(), _fakePlayer->getPositionY() + 35));
  }
  
  // 同步ECS位置
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

void KingSlimeTestScene::updateCameraFollow()
{
  if (!_fakePlayer) return;
  
  Vec2 playerPos = _fakePlayer->getPosition();
  Vec2 visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();
  
  // 计算摄像机目标位置（玩家在屏幕中心）
  Vec2 cameraTarget = Vec2(
    origin.x + visibleSize.x / 2 - playerPos.x,
    origin.y + visibleSize.y / 2 - playerPos.y
  );
  
  // 平滑跟随
  Vec2 currentPos = this->getPosition();
  Vec2 newPos = currentPos.lerp(cameraTarget, 0.1f);
  
  this->setPosition(newPos);
}

void KingSlimeTestScene::createDamageButton()
{
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();
  
  // 创建伤害按钮，位于屏幕中心
  _damageButton = Button::create();
  _damageButton->setTitleText("Damage Boss (-100 HP)");
  _damageButton->setTitleFontSize(24);
  _damageButton->setTitleColor(Color3B::WHITE);
  
  // 设置按钮外观
  _damageButton->setScale9Enabled(true);
  _damageButton->setContentSize(Size(250, 60));
  _damageButton->setColor(Color3B(200, 50, 50));
  
  // 位置在屏幕中心
  _damageButton->setPosition(Vec2(origin.x + visibleSize.width / 2, 
                                   origin.y + visibleSize.height / 2));
  
  // 添加点击事件
  _damageButton->addClickEventListener(CC_CALLBACK_1(KingSlimeTestScene::onDamageButtonClicked, this));
  
  // 添加到场景，使用高层级确保可见
  this->addChild(_damageButton, 100);
  
  CCLOG("KingSlimeTestScene: Damage button created at screen center");
}

void KingSlimeTestScene::onDamageButtonClicked(Ref* sender)
{
  if (_kingSlimeBossEntity == ecs::INVALID_ENTITY) {
    CCLOG("No KingSlime boss to damage!");
    return;
  }
  
  auto bossEntity = static_cast<entt::entity>(_kingSlimeBossEntity);
  if (!_registry.valid(bossEntity)) {
    CCLOG("KingSlime entity is invalid!");
    return;
  }
  
  auto* health = _registry.try_get<ecs::HealthComponent>(bossEntity);
  if (health) {
    health->currentHealth -= 100.0f;
    if (health->currentHealth < 0) {
      health->currentHealth = 0;
    }
    
    CCLOG(">>> DAMAGE BUTTON: Dealt 100 damage to KingSlime! Health: %.0f/%.0f", 
          health->currentHealth, health->maxHealth);
  } else {
    CCLOG("KingSlime has no HealthComponent!");
  }
}

void KingSlimeTestScene::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event)
{
  _keysPressed[keyCode] = true;
  
  if (keyCode == EventKeyboard::KeyCode::KEY_ESCAPE) {
    auto mainMenuScene = MainMenuScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, mainMenuScene));
  }
}

void KingSlimeTestScene::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event)
{
  _keysPressed[keyCode] = false;
}

// 已移除鼠标点击处理

void KingSlimeTestScene::spawnKingSlimeBoss()
{
  CCLOG("========== Spawning King Slime Boss ==========");
  
  // 加载Boss配置
  auto& factory = MonsterFactory::getInstance();
  factory.loadSingleConfig("config/bosses/KingSlime.json");
  
  if (!factory.getConfig("KingSlime")) {
    CCLOG("ERROR: Failed to load KingSlime configuration!");
    return;
  }
  
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();
  
  // 在场景中央稍右侧生成Boss
  float bossX = origin.x + visibleSize.width / 2 + 300.0f;
  float bossY = origin.y + 300.0f;
  
  // 使用MonsterFactory创建史莱姆王
  auto bossEntityId = factory.createMonster(_registry, "KingSlime", bossX, bossY, this);
  
  if (bossEntityId != ecs::INVALID_ENTITY) {
    _kingSlimeBossEntity = bossEntityId;
    
    // 获取Boss实体和组件
    auto bossEntity = static_cast<entt::entity>(bossEntityId);
    
    // 确保Boss有正确的组件
    if (auto* kingSlime = _registry.try_get<ecs::KingSlimeComponent>(bossEntity)) {
      CCLOG("KingSlime Boss spawned successfully!");
      CCLOG("  Position: (%.1f, %.1f)", bossX, bossY);
      CCLOG("  Health: %.0f/%.0f", 
            _registry.get<ecs::HealthComponent>(bossEntity).currentHealth,
            _registry.get<ecs::HealthComponent>(bossEntity).maxHealth);
      CCLOG("  Scale: %.2fx (will shrink with health)", kingSlime->currentScale);
      CCLOG("  Will spawn %d slimes total as health decreases", kingSlime->totalSlimesToSpawn);
      
      // 添加Boss血条显示
      createBossHealthBar();
    } else {
      CCLOG("ERROR: KingSlime created but missing KingSlimeComponent!");
    }
  } else {
    CCLOG("ERROR: Failed to create KingSlime boss!");
  }
  
  CCLOG("==========================================");
}

void KingSlimeTestScene::createBossHealthBar()
{
  if (_kingSlimeBossEntity == ecs::INVALID_ENTITY) return;
  
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();
  
  // Boss名称标签 - 位于右下角
  auto bossNameLabel = Label::createWithTTF("KING SLIME", "fonts/Marker Felt.ttf", 24);
  if (bossNameLabel) {
    bossNameLabel->setPosition(Vec2(origin.x + visibleSize.width - 150, origin.y + 80));
    bossNameLabel->setColor(Color3B(255, 215, 0)); // 金色
    bossNameLabel->setAnchorPoint(Vec2(0.5f, 0.5f));
    this->addChild(bossNameLabel, 10);
    _bossNameLabel = bossNameLabel;
  }
  
  // Boss血条背景 - 位于右下角
  auto healthBarBg = Sprite::create();
  if (healthBarBg) {
    healthBarBg->setTextureRect(Rect(0, 0, 300, 20));
    healthBarBg->setColor(Color3B(80, 20, 20)); // 深红色背景
    healthBarBg->setPosition(Vec2(origin.x + visibleSize.width - 150, origin.y + 50));
    healthBarBg->setAnchorPoint(Vec2(0.5f, 0.5f));
    this->addChild(healthBarBg, 9);
    _bossHealthBarBg = healthBarBg;
  }
  
  // Boss血条前景 - 位于右下角
  auto healthBarFg = Sprite::create();
  if (healthBarFg) {
    healthBarFg->setTextureRect(Rect(0, 0, 300, 20));
    healthBarFg->setColor(Color3B(255, 50, 50)); // 红色血条
    healthBarFg->setPosition(Vec2(origin.x + visibleSize.width - 150, origin.y + 50));
    healthBarFg->setAnchorPoint(Vec2(0.5f, 0.5f));
    this->addChild(healthBarFg, 10);
    _bossHealthBarFg = healthBarFg;
  }
  
  CCLOG("KingSlimeTestScene: Boss health bar created at bottom-right corner");
}

void KingSlimeTestScene::updateBossHealthBar()
{
  if (_kingSlimeBossEntity == ecs::INVALID_ENTITY || !_bossHealthBarFg) return;
  
  auto bossEntity = static_cast<entt::entity>(_kingSlimeBossEntity);
  if (!_registry.valid(bossEntity)) return;
  
  auto* health = _registry.try_get<ecs::HealthComponent>(bossEntity);
  if (!health) return;
  
  float healthPercent = health->currentHealth / health->maxHealth;
  healthPercent = std::max(0.0f, std::min(1.0f, healthPercent));
  
  // 更新血条宽度
  _bossHealthBarFg->setTextureRect(Rect(0, 0, 300 * healthPercent, 20));
  
  // 更新血条位置跟随相机（始终在屏幕右下角）
  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();
  Vec2 cameraOffset = -this->getPosition();  // 相机偏移
  
  if (_bossNameLabel) {
    _bossNameLabel->setPosition(Vec2(origin.x + visibleSize.width - 150 + cameraOffset.x, 
                                      origin.y + 80 + cameraOffset.y));
  }
  
  if (_bossHealthBarBg) {
    _bossHealthBarBg->setPosition(Vec2(origin.x + visibleSize.width - 150 + cameraOffset.x, 
                                        origin.y + 50 + cameraOffset.y));
  }
  
  if (_bossHealthBarFg) {
    _bossHealthBarFg->setPosition(Vec2(origin.x + visibleSize.width - 150 + cameraOffset.x, 
                                        origin.y + 50 + cameraOffset.y));
  }
  
  // 血量低时改变颜色
  if (healthPercent < 0.25f) {
    _bossHealthBarFg->setColor(Color3B(255, 100, 100)); // 浅红色
  } else if (healthPercent < 0.5f) {
    _bossHealthBarFg->setColor(Color3B(255, 150, 50)); // 橙色
  } else {
    _bossHealthBarFg->setColor(Color3B(255, 50, 50)); // 标准红色
  }
  
  // Boss死亡处理
  if (health->currentHealth <= 0 && _bossNameLabel) {
    _bossNameLabel->setString("KING SLIME - DEFEATED!");
    _bossNameLabel->setColor(Color3B(150, 150, 150)); // 灰色
  }
}

void KingSlimeTestScene::menuBackCallback(Ref* pSender)
{
  auto mainMenuScene = MainMenuScene::createScene();
  Director::getInstance()->replaceScene(TransitionFade::create(0.5f, mainMenuScene));
}
