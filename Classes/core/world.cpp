#include <memory>
#include <new>
#include <cmath>
#include "components/block/chunk_render.h"
#include "components/block/block_component.h"
#include "components/block/block_behavior.h"
#include "core/assets_manager.h"
#include "core/input_manager.h"
#include "systems/block_layer/block_layer.h"
#include "systems/block_layer/block_physics_layer.h"
#include "systems/block/block_system_manager.h"
#include "systems/block/render_command_system.h"
#include "systems/AllSystems.h"
#include "core/factory/monster/MonsterMasterFactory.h"
#include "components/render/SpriteComponent.h"
#include "utils/tools.h"
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
		};
		_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
	}

	if (auto* cam = getDefaultCamera()) {
		cam->setPosition(cocos2d::Vec2(0.0f, 1200.0f));
		if (_registry && _registry->valid(_playerEntity)) {
			if (auto* transform = _registry->try_get<ecs::TransformComponent>(_playerEntity)) {
				transform->position = cam->getPosition();
			}
		}
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
	
	return true;
}

void World::update(float delta) 
{
	cocos2d::Scene::update(delta);

	if (_registry && _registry->valid(_playerEntity)) {
		if (auto* cam = getDefaultCamera()) {
			if (auto* transform = _registry->try_get<ecs::TransformComponent>(_playerEntity)) {
				transform->position = cam->getPosition();
			}
		}
	}

	if (_npcSystemManager) {
		_npcSystemManager->update(delta);
	}
	_blockSystemManager->update(delta);
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

	_npcSystemManager->addSystem<ecs::AggroSystemEntt>();
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

	// 预加载所有怪物配置目录，确保 getAllSupportedMonsterIds 能覆盖全部ID
	{
		auto& mmf = MonsterMasterFactory::getInstance();
		mmf.preloadConfigDirectory("config/slimes");
	}

	{
		_playerEntity = _registry->create();
		auto& transform = _registry->emplace<ecs::TransformComponent>(_playerEntity);
		transform.position = cocos2d::Vec2(0.0f, 1200.0f);
		_registry->emplace<ecs::PlayerTag>(_playerEntity);
		auto& health = _registry->emplace<ecs::HealthComponent>(_playerEntity);
		health.maxHealth = 1000.0f;
		health.currentHealth = 1000.0f;
		health.invincibleTime = 0.3f;
	}

	{
		auto& mmf = MonsterMasterFactory::getInstance();
		auto ids = mmf.getAllSupportedMonsterIds();

		const float baseX = -260.0f;
		const float baseY = 980.0f;
		const float stepX = 80.0f;
		const float stepY = 100.0f;
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

	return true;
}
