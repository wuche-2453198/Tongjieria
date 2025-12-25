#include "core/consts.h"
#include "core/world.h"
#include "core/assets_manager.h"
#include "components/block/block_component.h"
#include "systems/block_layer/block_physics_layer.h"
#include "systems/block_layer/block_layer.h"
#include "utils/tools.h"
#include "cocos2d.h"
#include "block_physics_system.h"

BlockPhysicsSystem::BlockPhysicsSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : ISystem(registry, dispatcher),
    _assetManager(registry.ctx().get<AssetManager>()),
    _blockLayer(registry.ctx().get<BlockLayer>()),
    _physicsLayer(registry.ctx().get<BlockPhysicsLayer>())
{
    _physicsNode = cocos2d::Node::create();
    _physicsNode->setPosition(cocos2d::Vec2(0.5f, 0.5f) * BLOCK_SIZE);
    _registry.ctx().get<WorldScene>()->addChild(_physicsNode);
    _body = cocos2d::PhysicsBody::create();
    _body->setDynamic(false);
    _physicsNode->addComponent(_body);
}
BlockPhysicsSystem::~BlockPhysicsSystem() = default;

void BlockPhysicsSystem::update(float delta)
{
    // 1. 确认什么在域内
    // 2. 确认什么要去除

    // 3. 更新脏方块

    std::vector<Vec2i> toAdd = getAllAddIn();
    addAll(toAdd);
    std::vector<Vec2i> toRemove = getAllRemoveOut();
    removeAll(toRemove);

    updateDirtyBlock();
}

std::vector<Vec2i> BlockPhysicsSystem::getAllAddIn()
{
    auto view = _registry.view<Position, PhysicsTicket>();
    std::vector<Vec2i> toAdd;
    view.each([&](Position& worldPos, PhysicsTicket& ticket)
        {
            // debug
            Vec2i blockUpperLeft = getUpperLeft(worldPos, ticket);
            Vec2i blockLowerRight = getLowerRight(worldPos, ticket);

            for (int y = blockUpperLeft.y; y >= blockLowerRight.y; y--)
            {
                for (int x = blockUpperLeft.x; x <= blockLowerRight.x; x++)
                {
                    Vec2i blockPos = Vec2i(x, y);

                    if (hasCollision(blockPos) && !_physicsLayer.hasPhysicsShapeTag(blockPos))
                    {
                        toAdd.push_back(blockPos);
                    }
                }
            }
        });
    return toAdd;
}

std::vector<Vec2i> BlockPhysicsSystem::getAllRemoveOut()
{
    auto view = _registry.view<Position, PhysicsTicket>();
    std::vector<Vec2i> toRemove;

    for (auto& activePhysicsShape : _physicsLayer.getPhysicsBodyTags())
    {
        bool shouldRemoved = true;
        Vec2i blockPos = activePhysicsShape.first;
        view.each([&](Position& worldPos, PhysicsTicket& ticket)
            {
                Vec2i blockUpperLeft = getUpperLeft(worldPos, ticket);
                Vec2i blockLowerRight = getLowerRight(worldPos, ticket);
                if (isInside(blockPos, blockUpperLeft, blockLowerRight))
                {
                    shouldRemoved = false;
                    return;
                }
            });
        if (shouldRemoved)
        {
            toRemove.push_back(blockPos);
        }
    }
    return toRemove;
}

void BlockPhysicsSystem::addAll(std::vector<Vec2i> allAdded)
{
    for (auto& added : allAdded)
    {
        if (_physicsLayer.hasPhysicsShapeTag(added)) continue;
        _body->addShape(createBoxAtBlockPos(added));
        _physicsLayer.addPhysicsShapeTag(added, Vec2i::vec2ihash(added));
    }
}

void BlockPhysicsSystem::removeAll(std::vector<Vec2i> allRemoved)
{
    for (auto& removed : allRemoved)
    {
        _physicsLayer.removePhysicsShapeTag(removed);
        _body->removeShape(Vec2i::vec2ihash(removed));
    }
}

void BlockPhysicsSystem::updateDirtyBlock()
{
    auto view = _registry.view<DirtyChunkTag>();
    view.each([&](entt::entity chunkID, DirtyChunkTag& tag)
        {
            const auto chunkPos = BlockLayer::worldPosToChunkPos(_registry.get<Position>(chunkID));

            for (auto& dirtyBlock : tag.dirtyBlocks)
            {
                const Vec2i blockPos = chunkPos * CHUNK_SIZE + dirtyBlock.localPos;
                auto shape = createBoxAtBlockPos(blockPos);

                // 如果原有形体已经存在，则移除
                if (_physicsLayer.hasPhysicsShapeTag(blockPos))
                {
                    _body->removeShape(Vec2i::vec2ihash(blockPos));
                    _physicsLayer.removePhysicsShapeTag(blockPos);
                }
                // 如果新形体存在，则添加
                if (shape)
                {
                    _body->addShape(shape);
                    _physicsLayer.addPhysicsShapeTag(blockPos, Vec2i::vec2ihash(blockPos));
                }

                dirtyBlock.collisionDirty = false;
            }

            // 如果所有方块都清理干净了，则移除脏块标记
            if (tag.isAllClean())
            {
                _registry.remove<DirtyChunkTag>(chunkID);
            }
        });
}

bool BlockPhysicsSystem::isInside(const Vec2i& blockPos, const Vec2i& blockUpperLeft, const Vec2i& blockLowerRight)
{
    return blockPos.x >= blockUpperLeft.x &&
        blockPos.x <= blockLowerRight.x &&
        blockPos.y <= blockUpperLeft.y &&
        blockPos.y >= blockLowerRight.y;
}

bool BlockPhysicsSystem::hasCollision(const Vec2i& blockPos)
{
    auto blockState = _blockLayer.getBlockAtBlockPos(blockPos);

    bool collision = false;
    if (blockState.id.has_value())
    {
        collision = _assetManager.getBlockConfig(blockState.id.value()).hasCollision();
    }
    return collision;
}

Vec2i BlockPhysicsSystem::getUpperLeft(const Position& worldPos, const PhysicsTicket& ticket)
{
    Vec2i upperleft =
        worldPos.getPostion() + ticket.offset + Vec2i(-ticket.size.x, ticket.size.y) / 2;
    return _blockLayer.worldPosToBlockPos(upperleft);
}

Vec2i BlockPhysicsSystem::getLowerRight(const Position& worldPos, const PhysicsTicket& ticket)
{
    Vec2i lowerright =
        worldPos.getPostion() + ticket.offset + Vec2i(ticket.size.x, -ticket.size.y) / 2;
    return _blockLayer.worldPosToBlockPos(lowerright);
}

cocos2d::PhysicsShapeBox* BlockPhysicsSystem::createBoxAtBlockPos(const Vec2i& blockPos)
{
    // 如果没有碰撞，直接返回
    if (!hasCollision(blockPos))
    {
        return nullptr;
    }
    auto box = cocos2d::PhysicsShapeBox::create(
        { BLOCK_SIZE, BLOCK_SIZE },
        { 0.1, 1, 1 },
        { blockPos * BLOCK_SIZE });

    CCLOG("create box at %d %d", blockPos.x, blockPos.y);
    box->setTag(Vec2i::vec2ihash(blockPos));
    return box;
}