#include "block_world.h"
#include "systems/block_layer/block_layer.h"
#include "components/block/block_event.h"
#include "assets_manager.h"
#include "utils/tools.h"

BlockWorld::BlockWorld(entt::registry& registry, entt::dispatcher& dispatcher)
    : _registry(registry), 
    _dispatcher(dispatcher), 
    _blockLayer(registry.ctx().get<BlockLayer>()),
    _assetManager(_registry.ctx().get<AssetManager>()) {}
BlockWorld::~BlockWorld() = default;

BlockState BlockWorld::getBlockAtBlockPos(const Vec2i& blockPos) const
{
    if (_blockLayer.hasChunkExist(BlockLayer::blockPosToChunkLocalPos(blockPos)))
    {
        return _blockLayer.getBlockAtBlockPos(blockPos);
    }
    else
    {
        return BlockState();
    }
}

BlockState BlockWorld::getBlockAtWorldPos(const cocos2d::Vec2& worldPos) const
{
    return getBlockAtBlockPos(BlockLayer::worldPosToBlockPos(worldPos));
}

bool BlockWorld::tryInteract(const Vec2i& pos, entt::entity interactor)
{
    // 检查方块是否存在
    if (!_blockLayer.hasChunkExistAtBlockPos(pos))
    {
        return false;
    }
    // 取得方块ID
    entt::id_type id = _blockLayer.getBlockAtBlockPos(pos).id.value();

    // 触发交互事件
    BlockInteractEvent event(id, pos, interactor);
    _dispatcher.trigger(event);

    return true;
}

bool BlockWorld::tryInteractAtWorldPos(const cocos2d::Vec2& pos, entt::entity interactor)
{
    return tryInteract(BlockLayer::worldPosToBlockPos(pos), interactor);
}

bool BlockWorld::tryDestroy(const Vec2i& pos, entt::entity destroyer)
{
    // 检查方块是否存在
    if (!_blockLayer.hasChunkExistAtBlockPos(pos))
    {
        return false;
    }
    // 触发破坏事件
    entt::id_type id = _blockLayer.getBlockAtBlockPos(pos).id.value();

    BlockDestroyEvent event(id, pos, destroyer);
    _dispatcher.trigger(event);

    return true;
}

bool BlockWorld::tryDestroyAtWorldPos(const cocos2d::Vec2& pos, entt::entity destroyer)
{
    return tryDestroy(BlockLayer::worldPosToBlockPos(pos), destroyer);
}

bool BlockWorld::TryPlace(const Vec2i& blockPos, entt::id_type block_id, state block_state, entt::entity placer)
{
    // 检查方块位置是否合法
    if (!_blockLayer.hasChunkExistAtBlockPos(blockPos))
    {
        return false;
    }
    auto blockAtPos = _blockLayer.getBlockAtBlockPos(blockPos).id.value();
    auto& config = _assetManager.getBlockConfig(blockAtPos);
    // 检查方块配置是否存在
    if (!config)
    {
        return false;
    }
    // 检查方块是否可替换
    if(!config.isReplacable())
    {
        return false;
    }

    // 获取方块ID
    entt::id_type id = _blockLayer.getBlockAtBlockPos(blockPos).id.value();

    // 触发放置事件
    BlockPlacedEvent event(block_id, blockPos, placer);
    _dispatcher.trigger(event);

    return true;
}

bool BlockWorld::TryPlaceAtWorldPos(const cocos2d::Vec2& pos, entt::id_type block_id, state block_state, entt::entity placer)
{
    return TryPlace(BlockLayer::worldPosToBlockPos(pos), block_id, block_state, placer);
}
