#include "IntegrationTestScene.h"
#include "ui/items/InventoryLayer.h"
#include "ui/items/EquipmentPanel.h"
#include "systems/items/ItemManager.h"
#include "systems/items/Inventory.h"
#include "systems/player/PlayerFactory.h"
#include "systems/player/PlayerSystems.h"
#include "core/PlayerInput.h"
#include "components/player/PlayerComponents.h"

USING_NS_CC;

Scene* IntegrationTestScene::createScene() {
    CCLOG("========================================");
    CCLOG("= ENTERING INTEGRATION TEST SCENE");
    CCLOG("========================================");

    // Create scene with physics engine
    auto scene = Scene::createWithPhysics();

    // Set gravity
    auto physicsWorld = scene->getPhysicsWorld();
    physicsWorld->setGravity(Vec2(0, -980));

    // Enable physics debug drawing (optional)
    // physicsWorld->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);

    auto layer = IntegrationTestScene::create();
    scene->addChild(layer);

    CCLOG("IntegrationTestScene: Scene created with physics");
    return scene;
}

bool IntegrationTestScene::init() {
    if (!Layer::init()) {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // Create background
    auto background = LayerColor::create(Color4B(135, 206, 235, 255)); // Sky blue
    this->addChild(background, 0);

    // Load item data
    auto* itemMgr = ItemManager::getInstance();
    itemMgr->loadItems("items/items_json/items_equipment.json", false);
    itemMgr->loadItems("items/items_json/items_placeables.json", true);

    // Create physics environment
    createPhysicsEnvironment();

    // Create player
    createPlayer();

    // Setup test items
    setupTestItems();

    // Setup UI (initially hidden)
    setupUI();

    // Setup keyboard listener for ESC key
    setupKeyboardListener();

    // Delayed initialization (ensure scene is fully setup)
    this->scheduleOnce([this](float dt) {
        auto scene = this->getScene();
        if (scene) {
            PlayerInput::getInstance().initialize(scene);
            PlayerSystemsManager::setScene(scene);
            CCLOG("IntegrationTestScene: Input system and scene reference initialized");
        } else {
            CCLOG("IntegrationTestScene: ERROR - Scene is null!");
        }
    }, 0.1f, "init_input");

    // Start update
    this->scheduleUpdate();

    CCLOG("IntegrationTestScene: Initialization complete");
    return true;
}

void IntegrationTestScene::createPhysicsEnvironment() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    PhysicsMaterial material(1.0f, 0.0f, 1.0f); // density/restitution/friction

    // Create ground
    auto ground = Sprite::create();
    ground->setTextureRect(Rect(0, 0, visibleSize.width, 40));
    ground->setColor(Color3B(100, 150, 100)); // Grass green
    ground->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + 20));
    this->addChild(ground, 0);

    auto groundBody = PhysicsBody::createBox(ground->getContentSize(), material);
    groundBody->setDynamic(false);
    groundBody->setContactTestBitmask(0xFFFFFFFF);
    ground->setPhysicsBody(groundBody);

    // Create left wall
    auto leftWall = Sprite::create();
    leftWall->setTextureRect(Rect(0, 0, 10, visibleSize.height));
    leftWall->setColor(Color3B(80, 80, 80));
    leftWall->setPosition(Vec2(origin.x + 5, origin.y + visibleSize.height / 2));
    this->addChild(leftWall, 0);

    auto leftWallBody = PhysicsBody::createBox(leftWall->getContentSize(), material);
    leftWallBody->setDynamic(false);
    leftWallBody->setContactTestBitmask(0xFFFFFFFF);
    leftWall->setPhysicsBody(leftWallBody);

    // Create right wall
    auto rightWall = Sprite::create();
    rightWall->setTextureRect(Rect(0, 0, 10, visibleSize.height));
    rightWall->setColor(Color3B(80, 80, 80));
    rightWall->setPosition(Vec2(origin.x + visibleSize.width - 5,
                               origin.y + visibleSize.height / 2));
    this->addChild(rightWall, 0);

    auto rightWallBody = PhysicsBody::createBox(rightWall->getContentSize(), material);
    rightWallBody->setDynamic(false);
    rightWallBody->setContactTestBitmask(0xFFFFFFFF);
    rightWall->setPhysicsBody(rightWallBody);

    // Create platform 1
    auto platform1 = Sprite::create();
    platform1->setTextureRect(Rect(0, 0, 200, 20));
    platform1->setColor(Color3B(139, 90, 43)); // Brown
    platform1->setPosition(Vec2(origin.x + visibleSize.width / 2 - 180,
                               origin.y + visibleSize.height / 2 - 50));
    this->addChild(platform1, 0);

    auto platform1Body = PhysicsBody::createBox(platform1->getContentSize(), material);
    platform1Body->setDynamic(false);
    platform1Body->setContactTestBitmask(0xFFFFFFFF);
    platform1->setPhysicsBody(platform1Body);

    // Create platform 2
    auto platform2 = Sprite::create();
    platform2->setTextureRect(Rect(0, 0, 200, 20));
    platform2->setColor(Color3B(139, 90, 43)); // Brown
    platform2->setPosition(Vec2(origin.x + visibleSize.width / 2 + 180,
                               origin.y + visibleSize.height / 2));
    this->addChild(platform2, 0);

    auto platform2Body = PhysicsBody::createBox(platform2->getContentSize(), material);
    platform2Body->setDynamic(false);
    platform2Body->setContactTestBitmask(0xFFFFFFFF);
    platform2->setPhysicsBody(platform2Body);

    // Create platform 3
    auto platform3 = Sprite::create();
    platform3->setTextureRect(Rect(0, 0, 200, 20));
    platform3->setColor(Color3B(139, 90, 43)); // Brown
    platform3->setPosition(Vec2(origin.x + visibleSize.width / 2 + 180,
                               origin.y + visibleSize.height / 2-100));
    this->addChild(platform3, 0);

    auto platform3Body = PhysicsBody::createBox(platform3->getContentSize(), material);
    platform3Body->setDynamic(false);
    platform3Body->setContactTestBitmask(0xFFFFFFFF);
    platform3->setPhysicsBody(platform3Body);

    CCLOG("IntegrationTestScene: Physics environment created");

    // Create platform 4
    auto platform4 = Sprite::create();
    platform4->setTextureRect(Rect(0, 0, 200, 20));
    platform4->setColor(Color3B(139, 90, 43)); // Brown
    platform4->setPosition(Vec2(origin.x + visibleSize.width / 2 ,
                               origin.y + visibleSize.height / 2-150));
    this->addChild(platform4, 0);

    auto platform4Body = PhysicsBody::createBox(platform4->getContentSize(), material);
    platform4Body->setDynamic(false);
    platform4Body->setContactTestBitmask(0xFFFFFFFF);
    platform4->setPhysicsBody(platform4Body);

    CCLOG("IntegrationTestScene: Physics environment created");

    // Create platform 3
    auto platform5 = Sprite::create();
    platform5->setTextureRect(Rect(0, 0, 200, 20));
    platform5->setColor(Color3B(139, 90, 43)); // Brown
    platform5->setPosition(Vec2(origin.x + visibleSize.width / 2 ,
                               origin.y + visibleSize.height / 2-150));
    this->addChild(platform5, 0);

    auto platform5Body = PhysicsBody::createBox(platform5->getContentSize(), material);
    platform5Body->setDynamic(false);
    platform5Body->setContactTestBitmask(0xFFFFFFFF);
    platform5->setPhysicsBody(platform5Body);

    CCLOG("IntegrationTestScene: Physics environment created");


}

void IntegrationTestScene::createPlayer() {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // Use PlayerFactory to create player
    Vec2 spawnPos(visibleSize.width / 2, 300);
    _playerEntity = PlayerFactory::createPlayer(_registry, spawnPos, this);

    CCLOG("IntegrationTestScene: Player created!");
}

void IntegrationTestScene::setupTestItems() {
    auto* inventory = Inventory::getInstance();

    // Ensure inventory is initialized with correct slot count (59 slots)
    inventory->init(59);

    CCLOG("IntegrationTestScene: Inventory initialized with %d slots", (int)inventory->getSlots().size());

    // Add Wooden Sword (ID 1201)
    inventory->addItem(1201, 1);
    CCLOG("IntegrationTestScene: Added Wooden Sword (1201)");

    // Add armor pieces
    inventory->addItem(1101, 1); // Copper Helmet
    inventory->addItem(1102, 1); // Copper Chestplate
    inventory->addItem(1103, 1); // Copper Greaves
    CCLOG("IntegrationTestScene: Added Copper armor set");

    // Add Dirt blocks (ID 2001)
    inventory->addItem(2001, 99);
    CCLOG("IntegrationTestScene: Added Dirt blocks (2001)");

    // Link weapon slots (40-49) to hotbar
    auto& hotbar = _registry.get<ecs::PlayerHotbarComponent>(_playerEntity);
    for (int i = 0; i < 10 && i < ecs::PlayerHotbarComponent::HOTBAR_SIZE; ++i) {
        hotbar.slots[i] = 40 + i;  // Map to weapon slots (40-49)
    }

    CCLOG("IntegrationTestScene: Hotbar linked to weapon slots (40-49)");
}

void IntegrationTestScene::setupUI() {
    // Create InventoryLayer (initially hidden)
    _inventoryLayer = InventoryLayer::create();
    if (_inventoryLayer) {
        _inventoryLayer->setVisible(false);
        this->addChild(_inventoryLayer, 100);
        CCLOG("IntegrationTestScene: InventoryLayer created (hidden)");
    }

    // Create EquipmentPanel (initially hidden)
    _equipmentPanel = EquipmentPanel::create();
    if (_equipmentPanel) {
        _equipmentPanel->setVisible(false);
        this->addChild(_equipmentPanel, 100);
        CCLOG("IntegrationTestScene: EquipmentPanel created (hidden)");
    }

    // Link InventoryLayer and EquipmentPanel
    if (_inventoryLayer && _equipmentPanel) {
        _inventoryLayer->setEquipmentPanel(_equipmentPanel);
        CCLOG("IntegrationTestScene: UI layers linked");
    }

    _inventoryVisible = false;
}

void IntegrationTestScene::setupKeyboardListener() {
    auto listener = EventListenerKeyboard::create();

    listener->onKeyPressed = [this](EventKeyboard::KeyCode keyCode, Event* event) {
        if (keyCode == EventKeyboard::KeyCode::KEY_ESCAPE) {
            toggleInventory();
        }
    };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
    CCLOG("IntegrationTestScene: ESC key listener added");
}

void IntegrationTestScene::toggleInventory() {
    _inventoryVisible = !_inventoryVisible;

    if (_inventoryLayer) {
        _inventoryLayer->setVisible(_inventoryVisible);
    }
    if (_equipmentPanel) {
        _equipmentPanel->setVisible(_inventoryVisible);
    }

    CCLOG("IntegrationTestScene: Inventory %s", _inventoryVisible ? "SHOWN" : "HIDDEN");
}

void IntegrationTestScene::update(float delta) {
    // Update all player systems
    PlayerSystemsManager::updateAllSystems(_registry, delta);

    // Update input state (reset justPressed state for next frame)
    PlayerInput::getInstance().update(delta);
}
