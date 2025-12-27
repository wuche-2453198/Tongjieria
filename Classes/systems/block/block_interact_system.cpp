#include "core/assets_manager.h"
#include "components/block/block_behavior.h"
#include "components/block/block_component.h"
#include "systems/block_layer/block_layer.h"
#include "block_interact_system.h"

#define ON_DESTORY_LOG 1

BlockInteractSystem::BlockInteractSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : ISystem(registry, dispatcher), 
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
    // 获取对应的层
    auto& layer = _blockWorld.getLayer(event.layer);
    // 调用行为
    auto behavior = _behaviorRegistry->getBehavior(event.id);
    if (behavior) behavior->onBlockPlaced(event);

    // 创建新的方块
    BlockHandle newState(event.blockPos, event.id);
    layer.setBlockAtBlockPos(event.blockPos, newState);

    // 分发对应的标配
    auto chunkID = layer.getChunkEntity(BlockLayer::blockPosToChunkPos(event.blockPos));
    addDirtyTag(chunkID, BlockLayer::blockPosToChunkLocalPos(event.blockPos));
}

void BlockInteractSystem::onBlockDestroyed(const BlockDestroyEvent& event)
{
    // 获取对应的层
    auto& layer = _blockWorld.getLayer(event.layer);
    // 调用行为
    auto behavior = _behaviorRegistry->getBehavior(event.id);
    if (behavior) behavior->onBlockDestroyed(event);

#if ON_DESTORY_LOG
    std::string layerstr = event.layer == LayerType::BLOCK ? "block" : "wall";
    CCLOG("[BlockInteractSystem]: onDestroyed: %s %d, %d",layerstr.c_str(), event.blockPos.x, event.blockPos.y);
#endif // 


    BlockHandle newState(event.blockPos, entt::hashed_string("air"));
    layer.setBlockAtBlockPos(event.blockPos, newState);

    Vec2i chunkPos = BlockLayer::blockPosToChunkPos(event.blockPos);
    if (!layer.hasChunkExist(chunkPos))
    {
        return;
    }
    auto chunkID = layer.getChunkEntity(chunkPos);
    addDirtyTag(chunkID, BlockLayer::blockPosToChunkLocalPos(event.blockPos));
}

void BlockInteractSystem::onBlockMined(const BlockMinedEvent& event)
{
    // 获取对应的层
    auto& layer = _blockWorld.getLayer(event.layer);

    // 执行方块行为
    auto behavior = _behaviorRegistry->getBehavior(event.id);
    if (behavior) behavior->onBlockMined(event);
    
    auto blockEntity = layer.getBlockEntityAt(event.blockPos);
    if (!blockEntity.has_value())
    {
        blockEntity = layer.addBlockEntity(event.blockPos).first;
    }
    // 防止重复加入
    if (!_registry.all_of<MiningTag>(blockEntity.value()))
    {
        auto& block = _registry.get<BlockEntityHead>(blockEntity.value());
        block.saveEmplace<MiningTag>(_registry, blockEntity.value(), event.miningFactor, event.miner);
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