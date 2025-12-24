#include "core/assets_manager.h"
#include "components/block/block_behavior.h"
#include "components/block/block_component.h"
#include "systems/block_layer/block_layer.h"
#include "block_interact_system.h"

BlockInteractSystem::BlockInteractSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : ISystem(registry, dispatcher)
{
    _behaviorRegistry = std::make_unique<BlockBehaviorRegistry>();

    _dispatcher.sink<BlockDestroyEvent>().connect<&BlockInteractSystem::onBlockDestroyed>(this);
    _dispatcher.sink<BlockPlacedEvent>().connect<&BlockInteractSystem::onBlockPlaced>(this);
    _dispatcher.sink<BlockMinedEvent>().connect<&BlockInteractSystem::onBlockMined>(this);
    _dispatcher.sink<BlockInteractEvent>().connect<&BlockInteractSystem::onBlockInteracted>(this);
}

BlockInteractSystem::~BlockInteractSystem()
{
}

void BlockInteractSystem::onBlockPlaced(const BlockPlacedEvent& event)
{
    // 调用行为
    auto behavior = _behaviorRegistry->getBehavior(event.id);
    if (behavior) behavior->onBlockPlaced(event);

    auto& assetManager = _registry.ctx().get<AssetManager>();
    auto& blockLayer = _registry.ctx().get<BlockLayer>();

    BlockState newState(event.blockPos, event.id);
    blockLayer.setBlockAtBlockPos(event.blockPos, newState);

    auto chunkID = blockLayer.getChunkID(BlockLayer::blockPosToChunkPos(event.blockPos));
    addDirtyTag(chunkID, BlockLayer::blockPosToChunkLocalPos(event.blockPos));
}

void BlockInteractSystem::onBlockDestroyed(const BlockDestroyEvent& event)
{
    // 调用行为
    auto behavior = _behaviorRegistry->getBehavior(event.id);
    if (behavior) behavior->onBlockDestroyed(event);

    auto& assetManager = _registry.ctx().get<AssetManager>();
    auto& blockLayer = _registry.ctx().get<BlockLayer>();

    BlockState newState(event.blockPos, entt::hashed_string("air"));
    blockLayer.setBlockAtBlockPos(event.blockPos, newState);

    auto chunkID = blockLayer.getChunkID(BlockLayer::blockPosToChunkPos(event.blockPos));
    addDirtyTag(chunkID, BlockLayer::blockPosToChunkLocalPos(event.blockPos));
}

void BlockInteractSystem::onBlockMined(const BlockMinedEvent& event)
{
    _behaviorRegistry->getBehavior(event.id)->onBlockMined(event);
}

void BlockInteractSystem::onBlockNeighborChanged()
{
}

void BlockInteractSystem::onBlockInteracted(const BlockInteractEvent& event)
{
    _behaviorRegistry->getBehavior(event.id)->onBlockInteracted(event);
}

void BlockInteractSystem::onRandomTick()
{
}

void BlockInteractSystem::addDirtyTag(entt::entity chunk, const Vec2i& localPos)
{
    auto& tag = _registry.get_or_emplace<DirtyChunkTag>(chunk);
    tag.addDirtyBlock(localPos);
}