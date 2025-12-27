#include "block_world.h"
#include "systems/block_layer/block_layer.h"
#include "components/block/block_event.h"
#include "assets_manager.h"
#include "utils/tools.h"

#define DESTROY_LOG 1
#define PLACE_LOG 1

BlockWorld::BlockWorld(entt::registry& registry, entt::dispatcher& dispatcher)
    : _registry(registry), 
    _dispatcher(dispatcher), 
    _assetManager(_registry.ctx().get<AssetManager>()) 
{
    _blockLayer = std::make_unique<BlockLayer>(LayerType::BLOCK, _registry);
    _wallLayer = std::make_unique<BlockLayer>(LayerType::WALL, _registry);
}
BlockWorld::~BlockWorld() = default;

BlockLayer& BlockWorld::getLayer(LayerType layer) const
{
    if (layer == LayerType::BLOCK)
    {
        return *_blockLayer;
    }
    else
    {
        return *_wallLayer;
    }
}

BlockHandle BlockWorld::getBlockAtBlockPos(LayerType layerType, const Vec2i& blockPos) const
{
    auto& layer = getLayer(layerType);
    // 检查方块位置是否合法
    if (layer.hasChunkExist(BlockLayer::blockPosToChunkLocalPos(blockPos)))
    {
        return layer.getBlockAtBlockPos(blockPos);
    }
    else
    {
        return BlockHandle();
    }
}

BlockHandle BlockWorld::getBlockAtWorldPos(LayerType layerType, const cocos2d::Vec2& worldPos) const
{
    return getBlockAtBlockPos(layerType, BlockLayer::worldPosToBlockPos(worldPos));
}

bool BlockWorld::tryInteract(LayerType layerType, const Vec2i& blockPos, entt::entity interactor)
{
    auto& layer = getLayer(layerType); 
    // 检查方块位置是否合法
    if (!layer.hasChunkExistAtBlockPos(blockPos))
    {
        return false;
    }
    // 取得方块ID
    entt::id_type id = layer.getBlockAtBlockPos(blockPos).id.value();

    // 触发交互事件
    BlockInteractEvent event(layerType, id, blockPos, interactor);
    _dispatcher.trigger(event);

    return true;
}

bool BlockWorld::tryInteractAtWorldPos(LayerType layerType, const cocos2d::Vec2& worldPos, entt::entity interactor)
{
    return tryInteract(layerType, BlockLayer::worldPosToBlockPos(worldPos), interactor);
}

bool BlockWorld::tryMine(LayerType layerType, const Vec2i& blockPos, float mineFactor, entt::entity interactor)
{
    // 确认区块是否存在
    auto& layer = getLayer(layerType);
    if (!layer.hasChunkExistAtBlockPos(blockPos))
    {
        return false;
    }
    entt::id_type id = layer.getBlockAtBlockPos(blockPos).id.value();
    if (id == entt::hashed_string("air"))
    {
        return false;
    }
    BlockMinedEvent event(layerType, id, blockPos, mineFactor, interactor);
    _dispatcher.trigger(event);

    return true;
}

bool BlockWorld::tryMineAtWorldPos(LayerType layerType, const cocos2d::Vec2& worldPos, float mineFactor, entt::entity interactor)
{
    return tryMine(layerType, BlockLayer::worldPosToBlockPos(worldPos), mineFactor, interactor);
}

bool BlockWorld::tryDestroy(LayerType layerType, const Vec2i& blockPos, entt::entity destroyer)
{
#if DESTROY_LOG
    std::string layerstr = layerType == LayerType::BLOCK ? "block" : "wall";
    CCLOG("[BlockWorld]: Try destroy at %s %d %d", layerstr.c_str(), blockPos.x, blockPos.y);
#endif

    auto& layer = getLayer(layerType);
    // 检查方块是否存在
    if (!layer.hasChunkExistAtBlockPos(blockPos))
    {
#if DESTROY_LOG
        CCLOG("[BlockWorld]: Destroy faild at %d %d", blockPos.x, blockPos.y);
#endif
        return false;
    }
    // 触发破坏事件
    entt::id_type id = layer.getBlockAtBlockPos(blockPos).id.value();

    BlockDestroyEvent event(layerType, id, blockPos, destroyer);
    _dispatcher.trigger(event);

#if DESTROY_LOG
    CCLOG("[BlockWorld]: Destroy trigger at %d %d", blockPos.x, blockPos.y);
#endif

    return true;
}

bool BlockWorld::tryDestroyAtWorldPos(LayerType layerType, const cocos2d::Vec2& worldPos, entt::entity destroyer)
{
    return tryDestroy(layerType, BlockLayer::worldPosToBlockPos(worldPos), destroyer);
}

bool BlockWorld::tryPlace(const Vec2i& blockPos, entt::id_type blockID, state blockState, entt::entity placer)
{
    // 确认放置的方块是否是墙，同时获取对应的方块层
    auto& placedConfig = _assetManager.getBlockConfig(blockID);
    auto& layer = 
        placedConfig.getStateValOr<bool>("base", "wall", blockState, false) ?
        *_wallLayer : *_blockLayer;

#if PLACE_LOG 
    std::string layerstr = layer.getLayerType() == LayerType::BLOCK ? "block" : "wall";
    CCLOG("[BlockWorld]: Try Place at %s %d %d", layerstr.c_str(), blockPos.x, blockPos.y);
#endif

    // 检查方块位置是否合法
    if (!layer.hasChunkExistAtBlockPos(blockPos))
    {
#if PLACE_LOG 
        CCLOG("[BlockWorld]: Place at invailed chunk!");
#endif
        return false;
    }
    auto blockAtPos = layer.getBlockAtBlockPos(blockPos).id.value();
    auto& config = _assetManager.getBlockConfig(blockAtPos);

    // 检查方块配置是否存在
    if (!config)
    {
#if PLACE_LOG 
        CCLOG("[BlockWorld]: Place unkown block!");
#endif
        return false;
    }

    // 检查方块是否可替换
    if (!config.getOriginValOr("base","replaceable",false))
    {
#if PLACE_LOG 
        CCLOG("[BlockWorld]: target block is not replaceable!");
#endif
        return false;
    }

    // 获取方块ID
    entt::id_type id = layer.getBlockAtBlockPos(blockPos).id.value();

    // 触发放置事件
    BlockPlacedEvent event(layer.getLayerType(), blockID, blockPos, placer);
    _dispatcher.trigger(event);

    return true;
}

bool BlockWorld::tryPlaceAtWorldPos(const cocos2d::Vec2& pos, entt::id_type blockID, state blockState, entt::entity placer)
{
    return tryPlace(BlockLayer::worldPosToBlockPos(pos), blockID, blockState, placer);
}
