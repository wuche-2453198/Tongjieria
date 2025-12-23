#include "block_world.h"
#include "block_layer.h"
#include "block_event.h"
#include "server/assets_manager.h"
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

bool BlockWorld::TryInteract(const Vec2i& pos, entt::entity interactor)
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

bool BlockWorld::TryInteractAtWorldPos(const cocos2d::Vec2& pos, entt::entity interactor)
{
    return TryInteract(BlockLayer::worldPosToBlockPos(pos), interactor);
}

bool BlockWorld::TryDestroy(const Vec2i& pos, entt::entity destroyer)
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

bool BlockWorld::TryDestroyAtWorldPos(const cocos2d::Vec2& pos, entt::entity destroyer)
{
    return TryDestroy(BlockLayer::worldPosToBlockPos(pos), destroyer);
}

bool BlockWorld::TryPlace(const Vec2i& blockPos, entt::id_type block_id, state block_state, entt::entity placer)
{
    // 检查方块位置是否合法
    if (!_blockLayer.hasChunkExistAtBlockPos(blockPos))
    {
        return false;
    }
    auto blockAtPos = _blockLayer.getBlockAtBlockPos(blockPos).id.value();
    auto config = _assetManager.getBlockConfig(blockAtPos);
    // 检查方块配置是否存在
    if (!config)
    {
        return false;
    }
    // 检查方块是否可替换
    if(!tools::get_bool_or(*config, "replaceable", false))
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
