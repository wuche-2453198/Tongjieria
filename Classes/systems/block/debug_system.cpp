#include "core/world.h"
#include "core/input_manager.h"
#include "core/assets_manager.h"
#include "components/block/block_component.h"
#include "components/block/block_event.h"
#include "components/block/chunk_render.h"
#include "systems/block_layer/block_layer.h"
#include "systems/block_layer/block_physics_layer.h"
#include "utils/tools.h"
#include "debug_system.h"

DebugSystem::DebugSystem(entt::registry& registry, entt::dispatcher& dispatcher) 
    : ISystem(registry, dispatcher)
{
    auto& world = _registry.ctx().get<WorldScene>();
    auto& assetManager = _registry.ctx().get<AssetManager>();
    auto physicsWorld = world->getPhysicsWorld();
    physicsWorld->setDebugDrawMask(cocos2d::PhysicsWorld::DEBUGDRAW_ALL);
    world->setPhysics3DDebugCamera(cocos2d::Camera::getDefaultCamera());
    _dispatcher.sink<MouseEvent>().connect<&DebugSystem::onMouseEvent>(this);
    
    entt::entity entity = getEntity(_registry, "mouse");
    _registry.emplace<Position>(entity, cocos2d::Vec2::ZERO);
    _registry.emplace<LoadingTicket>(entity, entity, 1, false);

    auto mouseDrawNode = getDrawNode(_registry, "mouse");
    mouseDrawNode->drawDot({0,0}, 3, cocos2d::Color4F::RED);
    mouseDrawNode->setGlobalZOrder(10);

    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            getDrawNode(_registry, "edge")->drawDot(cocos2d::Vec2(i, j) * CHUNK_SIZE * BLOCK_SIZE, 1, cocos2d::Color4F::GREEN);
        }
    }

    for (int i = 0; i < 2; i++)
    {
        addAPhysicsSprites();
        physicsSprites[i]->setPosition(700 + i * 100, 100);
        _registry.emplace<LoadingTicket>(physicsEntity[i], physicsEntity[i], 1, false);
    }
}

DebugSystem::~DebugSystem() = default;

void DebugSystem::onMouseEvent(const MouseEvent& event)
{
    auto& blockWorld = _registry.ctx().get<BlockWorld>();
    auto blockPos = BlockLayer::worldPosToBlockPos(event.worldPos);

    cocos2d::Color4F color;
    
    if (event.button == cocos2d::EventMouse::MouseButton::BUTTON_RIGHT)
    {
        Vec2i blockPos =
            BlockLayer::worldPosToBlockPos(tools::MouseDebugTool::getWorldPosition());
        blockWorld.tryPlace(blockPos, entt::hashed_string("dirt"), 0, testEntites.at("mouse"));
    }
    else if(event.button == cocos2d::EventMouse::MouseButton::BUTTON_LEFT)
    {
        Vec2i blockPos =
            BlockLayer::worldPosToBlockPos(tools::MouseDebugTool::getWorldPosition());
        blockWorld.tryMine(LayerType::BLOCK, blockPos, 10, testEntites.at("mouse"));
    }
}

void DebugSystem::update(float delta)
{

    for (int i = 0; i < physicsEntity.size(); i++)
    {
        entt::entity entity = physicsEntity[i];
        auto sprite = physicsSprites[i];

        Position& pos = _registry.get<Position>(entity);
        pos = sprite->getPosition();
    }

    auto camera = cocos2d::Camera::getDefaultCamera();
    if (camera)
    {
        _registry.get<Position>(testEntites.at("mouse")) = camera->getPosition();
    }

    drawNodes.at("mouse")->setPosition(tools::MouseDebugTool::getWorldPosition());
}

entt::entity DebugSystem::getEntity(entt::registry& registry, const std::string& name)
{
    if (testEntites.find(name) == testEntites.end())
    {
        entt::entity entity = registry.create();
        testEntites[name] = entity;
        return entity;
    }
    else
    {
        return testEntites.at(name);
    }
}

cocos2d::DrawNode* DebugSystem::getDrawNode(entt::registry& registry, const std::string& name)
{
    if (drawNodes.find(name) == drawNodes.end())
    {
        auto& world = registry.ctx().get<WorldScene>();
        auto drawNode = cocos2d::DrawNode::create();
        drawNode->setPosition(cocos2d::Vec2::ZERO);
        world->addChild(drawNode);
        drawNodes[name] = drawNode;
        return drawNode;
    }
    else
    {
        return drawNodes.at(name);
    }
}

void DebugSystem::addAPhysicsSprites()
{
    auto& world = _registry.ctx().get<WorldScene>();
    auto& assetManager = _registry.ctx().get<AssetManager>();
    auto texture = assetManager.getTexture("a_block.bmp");

    auto sprites = cocos2d::Sprite::createWithTexture(texture);
    sprites->addComponent(cocos2d::PhysicsBody::createBox({ BLOCK_SIZE,BLOCK_SIZE }));
    world->addChild(sprites);

    entt::entity entity = _registry.create();
    _registry.emplace<Position>(entity, cocos2d::Vec2::ZERO);
    _registry.emplace<PhysicsTicket>(entity, cocos2d::Vec2(30, 30), cocos2d::Vec2::ZERO);

    physicsEntity.push_back(entity);
    physicsSprites.push_back(sprites);
}
