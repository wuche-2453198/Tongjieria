#include <memory>
#include <new>
#include <cmath>
#include <algorithm>
#include <string>
#include <vector>
#include <cstdio>
#include <limits>
#include "components/block/chunk_render.h"
#include "components/block/block_component.h"
#include "components/block/block_behavior.h"
#include "components\player\PlayerComponents.h"
#include "core/assets_manager.h"
#include "core/input_manager.h"
#include "systems/block_layer/block_layer.h"
#include "systems/block_layer/block_physics_layer.h"
#include "systems/block/block_system_manager.h"
#include "systems/block/render_command_system.h"
#include "systems/AllSystems.h"
#include "systems/player/PlayerFactory.h"
#include "systems/player/PlayerSystems.h"
#include "core/PlayerInput.h"
#include "core/factory/monster/MonsterMasterFactory.h"
#include "components/render/SpriteComponent.h"
#include "utils/tools.h"
#include "ui/items/InventoryLayer.h"
#include "ui/items/EquipmentPanel.h"
#include "ui/items/CraftBar.h"
#include "systems/item/Inventory.h"
#include "systems/item/ItemManager.h"
#include "components/player/PlayerComponents.h"
#include "world.h"

World::~World() = default;

World* World::create() {
	World* pRet = new (std::nothrow) World();
	if (pRet && pRet->init()) {
		pRet->autorelease();
		return pRet;
	}
	delete pRet;
	pRet = nullptr;
	return nullptr;
}

cocos2d::Scene* World::createScene() {
	return World::create();
}

bool World::init() 
{
	if (!Scene::initWithPhysics())
	{
		return false;
	}

	if (auto* physicsWorld = getPhysicsWorld()) {
		physicsWorld->setGravity(cocos2d::Vec2(0, -980));
		physicsWorld->setSpeed(1.0f);
		physicsWorld->setSubsteps(8);
	}
	
	scheduleUpdate();

	initServers();

	{
		_monsterCountLabel = cocos2d::Label::createWithSystemFont("Mobs: 0", "Arial", 18);
		if (_monsterCountLabel) {
			_monsterCountLabel->setAnchorPoint(cocos2d::Vec2(0.0f, 1.0f));
			const auto vs = cocos2d::Director::getInstance()->getVisibleSize();
			const auto origin = cocos2d::Director::getInstance()->getVisibleOrigin();
			_monsterCountLabel->setPosition(origin.x + 6.0f, origin.y + vs.height - 6.0f);
			addChild(_monsterCountLabel, std::numeric_limits<int>::max());
		}
		_monsterCountLabelTimer = 0.0f;
	}

	{
		const float interval = cocos2d::Director::getInstance()->getAnimationInterval();
		if (interval <= 0.0f) {
			_frameRateLimitMode = FrameRateLimitMode::Unlimited;
		}
		else if (std::fabs(interval - (1.0f / 240.0f)) < 0.00001f) {
			_frameRateLimitMode = FrameRateLimitMode::Fps240;
		}
		else {
			_frameRateLimitMode = FrameRateLimitMode::Fps60;
		}
	}

	{
		auto* listener = cocos2d::EventListenerKeyboard::create();
		listener->onKeyPressed = [this](cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event*) {
			if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_F1) {
				cycleFrameRateLimitMode();
			}
			else if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_ESCAPE) {
				// Toggle inventory, equipment panel, and crafting bar visibility
				if (_inventoryLayer && _equipmentPanel && _craftBar) {
					bool isVisible = _inventoryLayer->isVisible();
					_inventoryLayer->setVisible(!isVisible);
					_equipmentPanel->setVisible(!isVisible);
					_craftBar->setVisible(!isVisible);
					CCLOG("Inventory UI toggled: %s", !isVisible ? "visible" : "hidden");
				}
			}
		};
		_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
	}

	// 添加鼠标监听器用于触发破坏、放置和挖掘动画
	{
		auto* mouseListener = cocos2d::EventListenerMouse::create();

		// 鼠标按下事件
		mouseListener->onMouseDown = [this](cocos2d::Event* event) {
			auto* mouseEvent = static_cast<cocos2d::EventMouse*>(event);
			if (!_registry) {
				return;
			}

			auto view = _registry->view<ecs::PlayerTag, ecs::PlayerAnimationComponent, ecs::PlayerHotbarComponent, ecs::TransformComponent>();
			view.each([this, mouseEvent](auto entity, auto& tag, auto& animation, auto& hotbar, auto& transform) {
				if (mouseEvent->getMouseButton() == cocos2d::EventMouse::MouseButton::BUTTON_LEFT) {
					// 左键 - 根据手持物品触发不同动画
					auto* inventory = Inventory::getInstance();
					int currentSlot = hotbar.slots[hotbar.selectedIndex];
					const auto& slots = inventory->getSlots();
					int itemId = (currentSlot >= 0 && currentSlot < slots.size()) ? slots[currentSlot].itemId : 0;

					if (itemId != 0) {
						auto* itemMgr = ItemManager::getInstance();
						auto itemData = itemMgr->getItemData(itemId);

						if (itemData) {
							// 检查物品类型
							if (itemData->equipType == EquipType::Pickaxe) {
								// 镐子 - 开始播放MINE动画（持续循环）+ 破坏方块
								CCLOG("=== 开始MINE动画（持有镐子，持续挖掘）===");
								// 不设置isPlayingOneShot，让动画持续循环
								PlayerAnimationSystem::transitionToState(animation, ecs::PlayerAnimationComponent::AnimState::MINE);


								auto mousePos = tools::MouseDebugTool::getWorldPosition();
								// 转换为世界坐标
								auto camera = this->getDefaultCamera();
								if (camera) {
									// 获取玩家位置
									cocos2d::Vec2 playerPos = transform.position;

									// 计算玩家和鼠标点击位置的距离
									float distance = playerPos.distance(mousePos);

									// 7x7范围 = 2.5个方块的半径（对角线距离）
									const float BLOCK_SIZE = 16.0f;
									const float MAX_MINING_RANGE = 3.5f * BLOCK_SIZE * 1.414f; // 对角线距离

									if (distance <= MAX_MINING_RANGE) {
										// 在范围内，尝试挖掘方块
										auto& blockWorld = _registry->ctx().get<BlockWorld>();

										// 使用tryMineAtWorldPos进行挖掘，mineFactor基于镐子的挖掘力
										
										float mineFactor = 10.0f; // 默认挖掘力
										bool mined = blockWorld.tryMineAtWorldPos(LayerType::BLOCK, mousePos, mineFactor, entity);

										if (mined) {
											CCLOG("Block mined at distance: %.2f", distance);
										}
									} else {
										CCLOG("Block too far! Distance: %.2f, Max: %.2f", distance, MAX_MINING_RANGE);
									}
								}
							}
							else if (itemData->equipType == EquipType::Weapon || itemData->equipType == EquipType::Sword) {
								// 武器 - 播放ATTACK动画
								CCLOG("=== 触发ATTACK动画（持有武器）===");
								animation.isPlayingOneShot = true;
								PlayerAnimationSystem::transitionToState(animation, ecs::PlayerAnimationComponent::AnimState::ATTACK);
							}
							else {
								// 其他物品 - 播放BREAK动画
								CCLOG("=== 触发BREAK动画（其他物品）===");
								animation.isPlayingOneShot = true;
								PlayerAnimationSystem::transitionToState(animation, ecs::PlayerAnimationComponent::AnimState::BREAK);
							}
						} else {
							// 物品数据未找到 - 默认BREAK
							CCLOG("=== 触发BREAK动画（物品数据未找到）===");
							animation.isPlayingOneShot = true;
							PlayerAnimationSystem::transitionToState(animation, ecs::PlayerAnimationComponent::AnimState::BREAK);
						}
					} else {
						// 空手 - 播放BREAK动画
						CCLOG("=== 触发BREAK动画（空手）===");
						animation.isPlayingOneShot = true;
						PlayerAnimationSystem::transitionToState(animation, ecs::PlayerAnimationComponent::AnimState::BREAK);
					}
				}
				else if (mouseEvent->getMouseButton() == cocos2d::EventMouse::MouseButton::BUTTON_RIGHT) {
					// 右键 - 放置方块动画
					CCLOG("=== 触发PLACE动画（右键点击）===");
					animation.isPlayingOneShot = true;
					PlayerAnimationSystem::transitionToState(animation, ecs::PlayerAnimationComponent::AnimState::PLACE);
				}
			});
		};

		// 鼠标松开事件 - 停止挖掘动画
		mouseListener->onMouseUp = [this](cocos2d::Event* event) {
			auto* mouseEvent = static_cast<cocos2d::EventMouse*>(event);
			if (!_registry) {
				return;
			}

			auto view = _registry->view<ecs::PlayerTag, ecs::PlayerAnimationComponent>();
			view.each([mouseEvent](auto entity, auto& tag, auto& animation) {
				if (mouseEvent->getMouseButton() == cocos2d::EventMouse::MouseButton::BUTTON_LEFT) {
					// 如果当前正在播放MINE动画，则停止并返回IDLE
					if (animation.currentState == ecs::PlayerAnimationComponent::AnimState::MINE) {
						CCLOG("=== 停止MINE动画（松开左键）===");
						PlayerAnimationSystem::transitionToState(animation, ecs::PlayerAnimationComponent::AnimState::IDLE);
					}
				}
			});
		};

		_eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);
	}
	this->scheduleOnce([this](float) {
		if (!_registry) {
			return;
		}
		auto& blockWorld = _registry->ctx().get<BlockWorld>();
		for (int y = 12; y <= 18; ++y) {
			blockWorld.tryPlace(Vec2i(12, y), entt::hashed_string("dirt"), 0, _playerEntity);
		}
	}, 0.3f, "world_spawn_test_wall");

	// 测试：1秒后切换到BREAK动画，验证动画是否加载成功
	this->scheduleOnce([this](float) {
		if (!_registry) {
			return;
		}
		auto view = _registry->view<ecs::PlayerTag, ecs::PlayerAnimationComponent>();
		view.each([](auto entity, auto& tag, auto& animation) {
			CCLOG("=== 测试切换到BREAK动画 ===");
			PlayerAnimationSystem::transitionToState(animation, ecs::PlayerAnimationComponent::AnimState::BREAK);
		});
	}, 1.0f, "test_break_animation");
	
	return true;
}

void World::update(float delta)
{
	cocos2d::Scene::update(delta);

	if (_registry) {
		PlayerSystemsManager::updateAllSystems(*_registry, delta);
		PlayerInput::getInstance().update(delta);
	}

	if (_npcSystemManager) {
		_npcSystemManager->update(delta);
	}
	_blockSystemManager->update(delta);

	if (_monsterCountLabel && _registry) {
		_monsterCountLabelTimer += std::max(0.0f, delta);
		if (_monsterCountLabelTimer >= _monsterCountLabelInterval) {
			_monsterCountLabelTimer = 0.0f;

			int count = 0;
			auto view = _registry->view<ecs::PhysicsBodyComponent>();
			for (auto entity : view) {
				if (auto* pooled = _registry->try_get<ecs::PooledEntity>(entity)) {
					if (!pooled->inUse) {
						continue;
					}
				}
				if (_registry->any_of<ecs::PlayerTag>(entity)) {
					continue;
				}
				if (_registry->any_of<ecs::ProjectileComponent>(entity)) {
					continue;
				}

				constexpr int NPC_CATEGORY = 0x0002;
				const auto& physics = view.get<ecs::PhysicsBodyComponent>(entity);
				if ((physics.categoryBitmask & NPC_CATEGORY) == 0) {
					continue;
				}
				count++;
			}

			char buf[64];
			std::snprintf(buf, sizeof(buf), "Mobs: %d", count);
			_monsterCountLabel->setString(buf);
		}
	}
}

void World::applyFrameRateLimitMode()
{
	auto* director = cocos2d::Director::getInstance();
	switch (_frameRateLimitMode) {
	case FrameRateLimitMode::Fps60:
		director->setAnimationInterval(1.0f / 60.0f);
		break;
	case FrameRateLimitMode::Fps240:
		director->setAnimationInterval(1.0f / 240.0f);
		break;
	case FrameRateLimitMode::Unlimited:
		director->setAnimationInterval(0.0f);
		break;
	}
}

void World::cycleFrameRateLimitMode()
{
	switch (_frameRateLimitMode) {
	case FrameRateLimitMode::Fps60:
		_frameRateLimitMode = FrameRateLimitMode::Fps240;
		break;
	case FrameRateLimitMode::Fps240:
		_frameRateLimitMode = FrameRateLimitMode::Unlimited;
		break;
	case FrameRateLimitMode::Unlimited:
		_frameRateLimitMode = FrameRateLimitMode::Fps60;
		break;
	}
	applyFrameRateLimitMode();
}

bool World::initServers()
{

	_registry = std::make_unique<entt::registry>();
	_dispatcher = std::make_unique<entt::dispatcher>();


	_registry->ctx().emplace<WorldScene>(this);
	_registry->ctx().emplace<BlockPhysicsLayer>();
	_registry->ctx().emplace<AssetManager>();
	_registry->ctx().emplace<BlockWorld>(*_registry, *_dispatcher);
	_registry->ctx().emplace<InputManager>(*_dispatcher).init(this);
	_registry->ctx().emplace<BlockBehaviorRegistry>(*_registry, *_dispatcher);

	_blockSystemManager = std::make_unique<BlockSystemManager>(*_registry, *_dispatcher);

	_npcSystemManager = std::make_unique<ecs::SystemManagerEntt>();
	_npcSystemManager->setRegistry(_registry.get());
	ecs::SpriteDestructionObserver::registerToRegistry(*_registry);
	{
		auto* spawnSystem = _npcSystemManager->addSystem<ecs::MonsterSpawnSystemEntt>();
		if (spawnSystem) {
			spawnSystem->setRelaxedSpawnSearchEnabled(true);
			spawnSystem->setRelaxedSpawnSearchRangeTiles(10, 6);
			spawnSystem->setRelaxedSpawnMarginTiles(0.0f);
			spawnSystem->setRelaxedTreatMissingChunksAsClear(false);
		}
	}

	_npcSystemManager->addSystem<ecs::AggroSystemEntt>();
	_npcSystemManager->addSystem<ecs::WarriorAISystemEntt>();
	_npcSystemManager->addSystem<ecs::BatAISystemEntt>();
	_npcSystemManager->addSystem<ecs::DemonEyeAISystemEntt>();
	_npcSystemManager->addSystem<ecs::DemonAISystemEntt>();
	_npcSystemManager->addSystem<ecs::EaterOfSoulsAISystemEntt>();
	_npcSystemManager->addSystem<ecs::VultureAISystemEntt>();
	_npcSystemManager->addSystem<ecs::AntlionAISystemEntt>();
	_npcSystemManager->addSystem<ecs::AngryBonesAISystemEntt>();
	_npcSystemManager->addSystem<ecs::KingSlimeAISystemEntt>();
	_npcSystemManager->addSystem<ecs::GroundDetectorSystemEntt>();
	_npcSystemManager->addSystem<ecs::SlowFallSystemEntt>();
	_npcSystemManager->addSystem<ecs::JumpMovementSystemEntt>();
	_npcSystemManager->addSystem<ecs::ProjectileAttackSystemEntt>();
	_npcSystemManager->addSystem<ecs::ProjectileSystemEntt>();
	_projectileCollisionSystem = _npcSystemManager->addSystem<ecs::ProjectileCollisionSystemEntt>();
	_npcSystemManager->addSystem<ecs::DebuffSystemEntt>();
	_npcSystemManager->addSystem<ecs::RenderSystem>();
	_npcSystemManager->addSystem<ecs::AnimationSystem>();
	_npcSystemManager->addSystem<ecs::PhysicsSyncSystemEntt>();
	_npcSystemManager->addSystem<ecs::HealthSystemEntt>();
	_npcSystemManager->addSystem<ecs::CombatSystemEntt>();
	_npcSystemManager->addSystem<ecs::LifetimeSystemEntt>();

	_renderingCommandsSystem = new CommandSystem(*_registry, *_dispatcher);
	addChild(_renderingCommandsSystem);

	tools::MouseDebugTool::init(this);

	_sharedContactListener = ecs::PhysicsContactHandler::createContactListener(
		*_registry,
		[this](cocos2d::PhysicsContact& contact, const ecs::PhysicsContactHandler::ContactInfo& info) -> bool {
			(void)info;
			auto bodyA = contact.getShapeA() ? contact.getShapeA()->getBody() : nullptr;
			auto bodyB = contact.getShapeB() ? contact.getShapeB()->getBody() : nullptr;
			cocos2d::Node* nodeA = bodyA ? bodyA->getNode() : nullptr;
			cocos2d::Node* nodeB = bodyB ? bodyB->getNode() : nullptr;

			ecs::EntityId entityA = nodeA ? ecs::NodeEntityMap::getInstance().findEntity(nodeA) : ecs::INVALID_ENTITY;
			ecs::EntityId entityB = nodeB ? ecs::NodeEntityMap::getInstance().findEntity(nodeB) : ecs::INVALID_ENTITY;

			ecs::ProjectileComponent* projA = nullptr;
			ecs::ProjectileComponent* projB = nullptr;
			if (entityA != ecs::INVALID_ENTITY) {
				auto entA = static_cast<entt::entity>(entityA);
				if (_registry->valid(entA)) {
					projA = _registry->try_get<ecs::ProjectileComponent>(entA);
				}
			}
			if (entityB != ecs::INVALID_ENTITY) {
				auto entB = static_cast<entt::entity>(entityB);
				if (_registry->valid(entB)) {
					projB = _registry->try_get<ecs::ProjectileComponent>(entB);
				}
			}

			if (projA || projB) {
				ecs::ProjectileComponent* proj = projA ? projA : projB;
				ecs::EntityId otherEntity = projA ? entityB : entityA;
				cocos2d::PhysicsBody* otherBody = projA ? bodyB : bodyA;
				entt::entity projEntity = projA ? static_cast<entt::entity>(entityA) : static_cast<entt::entity>(entityB);

				if (_projectileCollisionSystem) {
					const auto* data = contact.getContactData();
					const cocos2d::Vec2* hitPos = (data && data->count > 0) ? &data->points[0] : nullptr;
					_projectileCollisionSystem->handleProjectileCollision(proj, projEntity, otherEntity, otherBody, hitPos);
				}
			}
			return true;
		},
		nullptr);
	if (_sharedContactListener) {
		_eventDispatcher->addEventListenerWithSceneGraphPriority(_sharedContactListener, this);
	}

	{
		auto& mmf = MonsterMasterFactory::getInstance();
		const char* dirs[] = {
			"config/slimes",
			"config/zombies",
			"config/bats",
			"config/eyes",
			"config/demons",
			"config/eaters",
			"config/skeletons",
			"config/desert",
			"config/bosses",
		};
		for (const char* dir : dirs) {
			mmf.preloadConfigDirectory(dir);
		}
	}

	{
		auto uiLayer = cocos2d::Node::create();
		uiLayer->setName("player_ui_layer");
		addChild(uiLayer, std::numeric_limits<int>::max());

		_playerEntity = PlayerFactory::createPlayer(*_registry,
			cocos2d::Vec2(0.0f, 1200.0f),
			this,
			uiLayer);

		PlayerSystemsManager::setScene(this);
		PlayerInput::getInstance().initialize(this);

		// Create and add inventory UI
		_inventoryLayer = InventoryLayer::create();
		if (_inventoryLayer) {
			uiLayer->addChild(_inventoryLayer, 100);
			_inventoryLayer->setVisible(false);  // Initially hidden, toggle with ESC
			CCLOG("InventoryLayer added to player UI layer");
		} else {
			CCLOG("Failed to create InventoryLayer");
		}

		// Create and add equipment panel UI
		_equipmentPanel = EquipmentPanel::create();
		if (_equipmentPanel) {
			uiLayer->addChild(_equipmentPanel, 100);
			_equipmentPanel->setVisible(false);  // Initially hidden, toggle with ESC
			CCLOG("EquipmentPanel added to player UI layer");
		} else {
			CCLOG("Failed to create EquipmentPanel");
		}

		// Create and add crafting bar UI
		_craftBar = CraftBar::create();
		if (_craftBar) {
			uiLayer->addChild(_craftBar, 100);
			_craftBar->setVisible(false);  // Initially hidden, toggle with ESC
			CCLOG("CraftBar added to player UI layer");
		} else {
			CCLOG("Failed to create CraftBar");
		}

		// Link inventory and equipment panel for cross-UI interaction
		if (_inventoryLayer && _equipmentPanel) {
			_inventoryLayer->setEquipmentPanel(_equipmentPanel);
		}
	}

	{
		auto& mmf = MonsterMasterFactory::getInstance();
		auto ids = mmf.getAllSupportedMonsterIds();

		const float baseX = -260.0f;
		const float baseY = 980.0f;
		const float stepX = 80.0f;
		const float stepY = 16 * 16 * 21;
		const int columns = 8;

		for (int i = 0; i < static_cast<int>(ids.size()); ++i) {
			if (ids[i].find("Slime") == std::string::npos) {
				continue;
			}
			const int col = i % columns;
			const int row = i / columns;
			const float x = baseX + col * stepX;
			const float y = baseY + row * stepY;

			ecs::EntityId id = mmf.createMonster(*_registry, ids[i], x, y, this);
			if (id == ecs::INVALID_ENTITY) {
				continue;
			}

			auto ent = static_cast<entt::entity>(id);
			if (_registry->valid(ent)) {
				if (auto* aggro = _registry->try_get<ecs::AggroComponent>(ent)) {
					aggro->aggroRange = std::max(aggro->aggroRange, 1500.0f);
					aggro->deaggroRange = std::max(aggro->deaggroRange, 2000.0f);
					aggro->targetTag = "Player";
				}
			}
		}
	}

	// Setup test items for inventory
	setupTestItems();

	return true;
}

void World::setupTestItems() {
    auto* inventory = Inventory::getInstance();

    // Ensure inventory is initialized with correct slot count (59 slots)
    inventory->init(59);

    CCLOG("World: Inventory initialized with %d slots", (int)inventory->getSlots().size());

    // Add Wooden Sword (ID 1201)
    //inventory->addItem(1201, 1);
    inventory->addItem(1211, 1);
    CCLOG("World: Added Wooden Sword (1201)");

    // Add pickaxes
    inventory->addItem(1203, 1); // Wooden Pick
    inventory->addItem(1213, 1); // Stone Pick
    inventory->addItem(1223, 1); // Copper Pick
    CCLOG("World: Added pickaxes");

    // Add armor pieces
    inventory->addItem(1101, 1); // Copper Helmet
    inventory->addItem(1102, 1); // Copper Chestplate
    inventory->addItem(1103, 1); // Copper Greaves
    CCLOG("World: Added Copper armor set");

    // Add Dirt blocks (ID 2001)
    inventory->addItem(2001, 99);
    CCLOG("World: Added Dirt blocks (2001)");

    // Add consumables - Food items (heals 20-50 HP, uses "eat" animation)
    inventory->addItem(4001, 10);  // Apple x10
    inventory->addItem(4002, 8);   // Apricot x8
    inventory->addItem(4003, 5);   // Bacon x5
    inventory->addItem(4004, 6);   // Coconut x6
    inventory->addItem(4005, 4);   // Cooked Fish x4
    CCLOG("World: Added 5 types of food items");

    // Add consumables - Potions (heals 50-200 HP, uses "drink" animation)
    inventory->addItem(4011, 15);  // Lesser Healing Potion x15
    inventory->addItem(4012, 10);  // Healing Potion x10
    inventory->addItem(4013, 5);   // Greater Healing Potion x5
    inventory->addItem(4014, 3);   // Super Healing Potion x3
    CCLOG("World: Added 4 types of potions");

    // Link weapon slots (40-49) to hotbar
    if (_registry && _registry->valid(_playerEntity)) {
        auto& hotbar = _registry->get<ecs::PlayerHotbarComponent>(_playerEntity);
        for (int i = 0; i < 10 && i < ecs::PlayerHotbarComponent::HOTBAR_SIZE; ++i) {
            hotbar.slots[i] = 40 + i;  // Map to weapon slots (40-49)
        }
        CCLOG("World: Hotbar linked to weapon slots (40-49)");
    } else {
        CCLOG("World: Warning - Player entity not valid, skipping hotbar setup");
    }
}
