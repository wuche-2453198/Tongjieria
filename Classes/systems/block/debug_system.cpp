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
    
    testEntites.push_back(_registry.create());
    _registry.emplace<Position>(testEntites[0], cocos2d::Vec2::ZERO);
    _registry.emplace<LoadingTicket>(testEntites[0], testEntites[0], 1, false);

    addADrawNode();
    drawNodes[0]->drawDot({0,0}, 10, cocos2d::Color4F::RED);

    addADrawNode();
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            drawNodes[1]->drawDot(cocos2d::Vec2(i, j) * CHUNK_SIZE * BLOCK_SIZE, 1, cocos2d::Color4F::GREEN);
        }
    }
    addADrawNode();

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
    auto& blockLayer = _registry.ctx().get<BlockLayer>();
    auto& blockWorld = _registry.ctx().get<BlockWorld>();
    auto blockPos = blockLayer.worldPosToBlockPos(event.worldPos);
    auto blockState = blockLayer.getBlockAtBlockPos(blockPos);

    cocos2d::Color4F color;
    
    if (event.button == cocos2d::EventMouse::MouseButton::BUTTON_RIGHT)
    {
        Vec2i blockPos =
            BlockLayer::worldPosToBlockPos(tools::MouseDebugTool::getWorldPosition());
        blockWorld.TryPlace(blockPos, entt::hashed_string("dirt"), 0, testEntites[0]);
    }
    else if(event.button == cocos2d::EventMouse::MouseButton::BUTTON_LEFT)
    {
        Vec2i blockPos =
            BlockLayer::worldPosToBlockPos(tools::MouseDebugTool::getWorldPosition());
        blockWorld.tryDestroy(blockPos, testEntites[0]);
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
        _registry.get<Position>(testEntites[0]) = camera->getPosition();
    }

    drawNodes[0]->setPosition(tools::MouseDebugTool::getWorldPosition());
}

void DebugSystem::addADrawNode()
{
    auto& world = _registry.ctx().get<WorldScene>();
    auto drawNode = cocos2d::DrawNode::create();
    drawNode->setPosition(cocos2d::Vec2::ZERO);
    world->addChild(drawNode);
    drawNodes.push_back(drawNode);
}

void DebugSystem::addAPhysicsSprites()
{
    auto& world = _registry.ctx().get<WorldScene>();
    auto& assetManager = _registry.ctx().get<AssetManager>();
    auto texture = assetManager.getTexture("blocks\\textures\\red_block.png");

    auto sprites = cocos2d::Sprite::createWithTexture(texture);
    sprites->addComponent(cocos2d::PhysicsBody::createBox({ BLOCK_SIZE,BLOCK_SIZE }));
    world->addChild(sprites);

    entt::entity entity = _registry.create();
    _registry.emplace<Position>(entity, cocos2d::Vec2::ZERO);
    _registry.emplace<PhysicsTicket>(entity, cocos2d::Vec2(30, 30), cocos2d::Vec2::ZERO);

    physicsEntity.push_back(entity);
    physicsSprites.push_back(sprites);
}
