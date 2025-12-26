#include "core/assets_manager.h"
#include "components/block/block_behavior.h"
#include "components/block/block_component.h"
#include "systems/block_layer/block_layer.h"
#include "block_interact_system.h"

BlockInteractSystem::BlockInteractSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : ISystem(registry, dispatcher), 
    _blockLayer(_registry.ctx().get<BlockLayer>()),
    _assetManager(_registry.ctx().get<AssetManager>())
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

    BlockHandle newState(event.blockPos, event.id);
    _blockLayer.setBlockAtBlockPos(event.blockPos, newState);

    auto chunkID = _blockLayer.getChunk(BlockLayer::blockPosToChunkPos(event.blockPos));
    addDirtyTag(chunkID, BlockLayer::blockPosToChunkLocalPos(event.blockPos));
}

void BlockInteractSystem::onBlockDestroyed(const BlockDestroyEvent& event)
{
    // 调用行为
    auto behavior = _behaviorRegistry->getBehavior(event.id);
    if (behavior) behavior->onBlockDestroyed(event);

    BlockHandle newState(event.blockPos, entt::hashed_string("air"));
    _blockLayer.setBlockAtBlockPos(event.blockPos, newState);

    auto chunkID = _blockLayer.getChunk(BlockLayer::blockPosToChunkPos(event.blockPos));
    addDirtyTag(chunkID, BlockLayer::blockPosToChunkLocalPos(event.blockPos));
}

void BlockInteractSystem::onBlockMined(const BlockMinedEvent& event)
{
    auto behavior = _behaviorRegistry->getBehavior(event.id);
    if (behavior) behavior->onBlockMined(event);

    BlockHandle handle = _blockLayer.getBlockAtBlockPos(event.blockPos);

    entt::entity blockEntity;
    if (!handle.blockEntiy.has_value())
    {
        // 创建一个新的方块实体
        blockEntity = _registry.create();

        _registry.emplace<Position>(blockEntity, event.blockPos * BLOCK_SIZE);
        _registry.emplace<ActiveBlock>(blockEntity, handle.id.value(), event.blockPos);

        auto chunkID = _blockLayer.getChunk(BlockLayer::blockPosToChunkPos(event.blockPos));
        _registry.get<ChunkBlocks>(chunkID).
            addEntity(BlockLayer::blockPosToChunkLocalPos(event.blockPos), blockEntity);
    }
    else
    {
        blockEntity = handle.blockEntiy.value();
    }

    auto& block = _registry.get<ActiveBlock>(blockEntity);

    // 防止重复加入
    if (!_registry.all_of<MiningTag>(blockEntity))
    {
        block.saveEmplace<MiningTag>(_registry, blockEntity, event.miningFactor, event.miner);
    }
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