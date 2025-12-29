#include "IntegrationTestScene.h"
#include "ui/items/InventoryLayer.h"
#include "ui/items/EquipmentPanel.h"
#include "ui/items/CraftBar.h"
#include "systems/items/ItemManager.h"
#include "systems/items/Inventory.h"
#include "systems/items/RecipeManager.h"
#include "systems/items/StationDetector.h"
#include "systems/player/PlayerFactory.h"
#include "systems/player/PlayerSystems.h"
#include "systems/player/PlayerCraftingSystem.h"
#include "core/PlayerInput.h"
#include "core/GameManager.h"
#include "core/assets_manager.h"
#include "core/block_world.h"
#include "components/player/PlayerComponents.h"
#include "components/block/block_component.h"
#include "systems/block/block_system_manager.h"
#include "systems/block_layer/block_physics_layer.h"
#include "systems/block/render_command_system.h"

USING_NS_CC;

IntegrationTestScene::IntegrationTestScene() {
    // 构造函数在这里定义，此时 BlockSystemManager 的完整定义可见
}

IntegrationTestScene::~IntegrationTestScene() {
    // Shutdown equipment sync system
    PlayerEquipmentSyncSystem::shutdown();

    // unique_ptr 会自动清理资源
}

Scene* IntegrationTestScene::createScene() {
    auto scene = new IntegrationTestScene();
    if (scene && scene->init()) {
        scene->autorelease();
        return scene;
    }
    CC_SAFE_DELETE(scene);
    return nullptr;
}

bool IntegrationTestScene::init() {
    // Initialize as Scene with physics
    if (!Scene::initWithPhysics()) {
        return false;
    }

    // Set gravity
    auto physicsWorld = this->getPhysicsWorld();
    physicsWorld->setGravity(Vec2(0, -480));

    // Enable physics debug drawing to see collision shapes
    physicsWorld->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // Create background
    auto background = LayerColor::create(Color4B(135, 206, 235, 255)); // Sky blue
    this->addChild(background, 0);

    // Create UI layer (fixed to screen, not affected by camera movement)
    _uiLayer = Node::create();
    _uiLayer->setPosition(Vec2::ZERO);
    _uiLayer->setAnchorPoint(Vec2::ZERO);
    _uiLayer->setGlobalZOrder(1000);  // Very high Z order to always be on top
    this->addChild(_uiLayer, 1000);

    // Create a separate camera for UI that follows the default camera
    _uiCamera = Camera::createOrthographic(
        visibleSize.width,
        visibleSize.height,
        -1024, 1024
    );
    _uiCamera->setCameraFlag(CameraFlag::USER1);
    _uiCamera->setDepth(10);  // Higher depth so it renders on top

    // Position UI camera at screen center (orthographic projection)
    _uiCamera->setPosition(visibleSize.width / 2, visibleSize.height / 2);
    this->addChild(_uiCamera);

    // Set UI layer to only be visible to the UI camera
    _uiLayer->setCameraMask((unsigned short)CameraFlag::USER1);

    CCLOG("IntegrationTestScene: UI layer and UI camera created (will follow default camera)");

    // 1. 初始化 ECS 系统（必须在创建玩家之前）
    _registry = std::make_unique<entt::registry>();
    _dispatcher = std::make_unique<entt::dispatcher>();
    CCLOG("IntegrationTestScene: Registry and dispatcher created");

    // 2. 注册上下文（参考 WorldTest）
    // IntegrationTestScene 现在是 Scene，所以直接传入 this
    _registry->ctx().emplace<WorldScene>(this);
    _registry->ctx().emplace<BlockPhysicsLayer>();
    _registry->ctx().emplace<AssetManager>();
    CCLOG("IntegrationTestScene: Context registered (WorldScene, BlockPhysicsLayer, AssetManager)");

    // 3. 创建 BlockWorld（在上下文中）
    _registry->ctx().emplace<BlockWorld>(*_registry, *_dispatcher);
    CCLOG("IntegrationTestScene: BlockWorld created");

    // 4. 创建 BlockSystemManager
    _blockSystemManager = std::make_unique<BlockSystemManager>(*_registry, *_dispatcher);
    CCLOG("IntegrationTestScene: BlockSystemManager created");

    // 5. 创建渲染命令系统（负责方块渲染）
    _renderingCommandsSystem = new CommandSystem(*_registry, *_dispatcher);
    addChild(_renderingCommandsSystem);
    CCLOG("IntegrationTestScene: CommandSystem created and added");

    // Load item data
    auto* itemMgr = ItemManager::getInstance();
    itemMgr->loadItems("items/items_json/items_equipment.json", false);
    itemMgr->loadItems("items/items_json/items_placeables.json", true);
    itemMgr->loadItems("items/items_json/items_consumables.json", true);
    itemMgr->loadItems("items/items_json/items_materials.json", true);

    // Load recipes for crafting system
    auto* recipeMgr = RecipeManager::getInstance();
    recipeMgr->loadRecipes("items/items_json/recipes.json");
    CCLOG("IntegrationTestScene: Loaded %zu recipes", recipeMgr->getRecipeCount());

    // Initialize StationDetector with Hand station by default
    auto* stationDetector = StationDetector::getInstance();
    stationDetector->addStation(StationType::Hand);
    CCLOG("IntegrationTestScene: StationDetector initialized with Hand station");

    // 不使用这个物理环境而是使用blocks系统
    //createPhysicsEnvironment();

    // Create player
    createPlayer();

    // Setup test items
    setupTestItems();

    // Setup UI (initially hidden)
    setupUI();

    // Setup keyboard listener for ESC key
    setupKeyboardListener();

    // Initialize input system
    PlayerInput::getInstance().initialize(this);
    PlayerSystemsManager::setScene(this);
    CCLOG("IntegrationTestScene: Input system initialized");

    // Initialize equipment sync system
    PlayerEquipmentSyncSystem::initialize(*_registry);
    CCLOG("IntegrationTestScene: Equipment sync system initialized");

    // Start update
    this->scheduleUpdate();

    CCLOG("IntegrationTestScene: Initialization complete (with BlockWorld)");
    return true;
}

void IntegrationTestScene::createPlayer() {
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // Use PlayerFactory to create player

    Vec2 spawnPos(visibleSize.width / 2, 1220);  // 在地形上方一点
    _playerEntity = PlayerFactory::createPlayer(*_registry, spawnPos, this, _uiLayer);

    // Register player entity in GameManager (for UI and other systems)
    GameManager::getInstance()->setPlayerEntity(_playerEntity);

    // NOTE: Position and LoadingTicket components are now added in PlayerFactory
    // No need to add them here anymore

    // Add PhysicsTicket for block collision (方块碰撞检测需要PhysicsTicket)
    // 碰撞箱大小应该与玩家物理体一致
    Vec2 collisionSize(80.0f, 122.0f);  // 与PlayerFactory中的物理体大小一致
    Vec2 collisionOffset(0.0f, 0.0f);   // 碰撞箱中心偏移
    _registry->emplace<PhysicsTicket>(_playerEntity, collisionSize, collisionOffset);
    CCLOG("IntegrationTestScene: PhysicsTicket component added to player (size: %.1fx%.1f)",
          collisionSize.x, collisionSize.y);

    // NOTE: LoadingTicket is now added in PlayerFactory with radius=8
    // No need to add it here anymore
    CCLOG("IntegrationTestScene: Player has LoadingTicket (radius=8, added by PlayerFactory)");

    CCLOG("IntegrationTestScene: Player created with all block system components");
}

void IntegrationTestScene::setupTestItems() {
    auto* inventory = Inventory::getInstance();

    // Ensure inventory is initialized with correct slot count (59 slots)
    inventory->init(59);

    CCLOG("IntegrationTestScene: Inventory initialized with %d slots", (int)inventory->getSlots().size());

    // Add Wooden Sword (ID 1201)
    inventory->addItem(1201, 1);
    inventory->addItem(1211, 1); 
    CCLOG("IntegrationTestScene: Added Wooden Sword (1201)");

    // Add pickaxes
    inventory->addItem(1203, 1); // Wooden Pick
    inventory->addItem(1213, 1); // Stone Pick
    inventory->addItem(1223, 1); // Copper Pick
    CCLOG("IntegrationTestScene: Added pickaxes");

    // Add armor pieces
    inventory->addItem(1101, 1); // Copper Helmet
    inventory->addItem(1102, 1); // Copper Chestplate
    inventory->addItem(1103, 1); // Copper Greaves
    CCLOG("IntegrationTestScene: Added Copper armor set");

    // Add Dirt blocks (ID 2001)
    inventory->addItem(2001, 99);
    CCLOG("IntegrationTestScene: Added Dirt blocks (2001)");

    // Add consumables - Food items (heals 20-50 HP, uses "eat" animation)
    inventory->addItem(4001, 10);  // Apple x10
    inventory->addItem(4002, 8);   // Apricot x8
    inventory->addItem(4003, 5);   // Bacon x5
    inventory->addItem(4004, 6);   // Coconut x6
    inventory->addItem(4005, 4);   // Cooked Fish x4
    CCLOG("IntegrationTestScene: Added 5 types of food items");

    // Add consumables - Potions (heals 50-200 HP, uses "drink" animation)
    inventory->addItem(4011, 15);  // Lesser Healing Potion x15
    inventory->addItem(4012, 10);  // Healing Potion x10
    inventory->addItem(4013, 5);   // Greater Healing Potion x5
    inventory->addItem(4014, 3);   // Super Healing Potion x3
    CCLOG("IntegrationTestScene: Added 4 types of potions");

    // Link weapon slots (40-49) to hotbar
    auto& hotbar = _registry->get<ecs::PlayerHotbarComponent>(_playerEntity);
    for (int i = 0; i < 10 && i < ecs::PlayerHotbarComponent::HOTBAR_SIZE; ++i) {
        hotbar.slots[i] = 40 + i;  // Map to weapon slots (40-49)
    }

    CCLOG("IntegrationTestScene: Hotbar linked to weapon slots (40-49)");
}

void IntegrationTestScene::setupUI() {
    // Create InventoryLayer (initially hidden) - Add to UI layer
    _inventoryLayer = InventoryLayer::create();
    if (_inventoryLayer) {
        _inventoryLayer->setVisible(false);
        _uiLayer->addChild(_inventoryLayer, 100);
        CCLOG("IntegrationTestScene: InventoryLayer created (hidden) - added to UI layer");
    }

    // Create EquipmentPanel (initially hidden) - Add to UI layer
    _equipmentPanel = EquipmentPanel::create();
    if (_equipmentPanel) {
        _equipmentPanel->setVisible(false);
        _uiLayer->addChild(_equipmentPanel, 100);
        CCLOG("IntegrationTestScene: EquipmentPanel created (hidden) - added to UI layer");
    }

    // Create CraftBar (initially hidden) - Add to UI layer
    _craftBar = CraftBar::create();
    if (_craftBar) {
        _craftBar->setVisible(false);
        _uiLayer->addChild(_craftBar, 100);
        CCLOG("IntegrationTestScene: CraftBar created (hidden) - added to UI layer");
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
    if (_craftBar) {
        _craftBar->setVisible(_inventoryVisible);
    }

    // Also notify PlayerCraftingSystem
    if (_inventoryVisible) {
        PlayerCraftingSystem::openCraftingUI();
    } else {
        PlayerCraftingSystem::closeCraftingUI();
    }

    CCLOG("IntegrationTestScene: Inventory %s", _inventoryVisible ? "SHOWN" : "HIDDEN");
}
//UI界面实时刷新

void IntegrationTestScene::update(float delta) {
    // Update block systems (chunk loading, terrain generation, mining, etc.)
    if (_blockSystemManager) {
        _blockSystemManager->update(delta);
    }

    // Update all player systems
    PlayerSystemsManager::updateAllSystems(*_registry, delta);

    // CRITICAL: UI camera MUST follow default camera to keep UI fixed on screen
    // When player moves, DefaultCamera follows player in world coordinates
    // UICamera must sync with DefaultCamera so UI elements (in world coords) stay on screen
    // This way: UI world position changes, but screen position stays constant
    if (_uiCamera) {
        auto defaultCamera = this->getDefaultCamera();
        if (defaultCamera) {
            Vec3 camPos = defaultCamera->getPosition3D();
            _uiCamera->setPosition3D(camPos);
        }
    }

    // Update block systems
    if (_blockSystemManager) {
        _blockSystemManager->update(delta);
    }

    // Debug: Check Position component update
    // static int debugCounter = 0;
    // if (debugCounter++ % 120 == 0) {  // Every 2 seconds
    //     if (_registry->all_of<Position, ecs::PlayerStatsComponent>(_playerEntity)) {
    //         auto& pos = _registry->get<Position>(_playerEntity);
    //         auto& stats = _registry->get<ecs::PlayerStatsComponent>(_playerEntity);
    //         auto& sprite = _registry->get<ecs::PlayerSpriteComponent>(_playerEntity);

    //         Vec2 actualPos = sprite.sprite->getPosition();
    //         CCLOG("=== PLAYER DEBUG ===");
    //         CCLOG("Position component: (%.1f, %.1f)", pos.getPostion().x, pos.getPostion().y);
    //         CCLOG("Sprite position: (%.1f, %.1f)", actualPos.x, actualPos.y);
    //         CCLOG("isOnGround: %s", stats.isOnGround ? "YES" : "NO");
    //         if (sprite.sprite->getPhysicsBody()) {
    //             Vec2 vel = sprite.sprite->getPhysicsBody()->getVelocity();
    //             CCLOG("Velocity: (%.1f, %.1f)", vel.x, vel.y);
    //         }
    //     }
    //}

    // Update input state (reset justPressed state for next frame)
    PlayerInput::getInstance().update(delta);
}
