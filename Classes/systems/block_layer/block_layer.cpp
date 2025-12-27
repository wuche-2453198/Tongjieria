#include "cocos2d.h"
#include "block_layer.h"
#include "core/consts.h"
#include "components/block/block_component.h"

BlockLayer::BlockLayer(LayerType type, entt::registry& registry) 
    : _layerType(type), _registry(registry) {}
BlockLayer::~BlockLayer() {}

Vec2i BlockLayer::worldPosToChunkPos(const cocos2d::Vec2& worldPos)
{
    return Vec2i(blockPosToChunkPos(worldPosToBlockPos(worldPos)));
}

Vec2i BlockLayer::worldPosToBlockPos(const cocos2d::Vec2& worldPos)
{
    return Vec2i(floor(worldPos.x / BLOCK_SIZE), floor(worldPos.y / BLOCK_SIZE));
}


Vec2i BlockLayer::blockPosToChunkPos(const Vec2i& blockPos) {
    return Vec2i(floor(blockPos.x * 1.0f / CHUNK_SIZE), floor(blockPos.y * 1.0f / CHUNK_SIZE));
}

Vec2i BlockLayer::blockPosToChunkLocalPos(const Vec2i& worldPos) {
    return worldPos - blockPosToChunkPos(worldPos) * CHUNK_SIZE;
}

bool BlockLayer::hasChunkExistAtWorldPos(const cocos2d::Vec2& worldPos) const
{
    return hasChunkExist(worldPosToChunkPos(worldPos));
}

bool BlockLayer::hasChunkExistAtBlockPos(const Vec2i& worldPos) const {
    return hasChunkExist(blockPosToChunkPos(worldPos));
}

bool BlockLayer::hasChunkExist(const Vec2i& chunkPos) const {
    return _chunkMappings.find(chunkPos) != _chunkMappings.end();
}

BlockHandle BlockLayer::getBlockAtWorldPos(const cocos2d::Vec2& worldPos) const
{
    return getBlockAtBlockPos(worldPosToBlockPos(worldPos));
}

bool BlockLayer::setBlockAtWorldPos(const cocos2d::Vec2& worldPos, const BlockHandle& state)
{
    return setBlockAtBlockPos(worldPosToBlockPos(worldPos), state);
}

std::pair<entt::entity, ChunkHead&> BlockLayer::addChunk(const Vec2i& chunkPos) {
    assert(!hasChunkExist(chunkPos));
    // 创建实体和区块头
    entt::entity entity = _registry.create();
    _registry.emplace<Position>(entity, chunkPos * CHUNK_SIZE * BLOCK_SIZE);
    ChunkHead& head = _registry.emplace<ChunkHead>(entity, _layerType);
    // 维护区块索引
    _chunkMappings[chunkPos] = entity;

    return { entity, head };
}

void BlockLayer::destroyChunk(const Vec2i& chunkPos) {
    assert(hasChunkExist(chunkPos));
    // 移除实体和区块
    auto& chunkBlocks = _registry.get<ChunkBlocks>(getChunkEntity(chunkPos));

    // 移除所有方块实体
    auto& blockEntites = chunkBlocks.getEntityMapping();
    for (auto [pos, entity] : blockEntites)
    {
        _registry.destroy(entity);
    }
    
    _registry.destroy(getChunkEntity(chunkPos));
    _chunkMappings.erase(chunkPos);
}

std::pair<entt::entity, BlockEntityHead&> BlockLayer::addBlockEntity(const Vec2i& blockPos)
{
    auto handle = getBlockAtBlockPos(blockPos);

    // 生成方块实体
    entt::entity entity = _registry.create();
    _registry.emplace<Position>(entity, blockPos * BLOCK_SIZE);
    auto& head = _registry.emplace<BlockEntityHead>(entity, _layerType, handle.id.value(), blockPos);

    // 维护索引
    auto& chunkBlocks = _registry.get<ChunkBlocks>(getChunkEntity(blockPosToChunkPos(blockPos)));
    chunkBlocks.addEntity(blockPosToChunkLocalPos(blockPos), entity);

    return { entity, head };
}

void BlockLayer::destroyBlockEntity(const Vec2i& blockPos)
{
    auto& chunkBlocks = _registry.get<ChunkBlocks>(getChunkEntity(blockPosToChunkPos(blockPos)));

    Vec2i localPos = blockPosToChunkLocalPos(blockPos);

    // 清除实体同时维护实体表
    _registry.destroy(chunkBlocks.getEntityAt(localPos));
    chunkBlocks.removeEntity(localPos);
}

std::optional<entt::entity> BlockLayer::getBlockEntityAt(const Vec2i& blockPos) const
{
    // todo 可能会移除，减少一点性能消耗，安全性由blockWorld保证
    if (!hasChunkExist(blockPosToChunkPos(blockPos)))
    {
        return std::nullopt;
    }

    auto& chunkBlocks = _registry.get<ChunkBlocks>(getChunkEntity(blockPosToChunkPos(blockPos)));

    Vec2i localPos = blockPosToChunkLocalPos(blockPos);

    if (chunkBlocks.hasChunkEntityAt(localPos))
    {
        return chunkBlocks.getEntityAt(localPos);
    }
    else
    {
        return std::nullopt;
    }
}

BlockHandle BlockLayer::getBlockAtBlockPos(const Vec2i& blockPos) const
{
    auto chunkPos = blockPosToChunkPos(blockPos);

    // todo 可能会移除，减少一点性能消耗，安全性由blockWorld保证
    if (!hasChunkExist(chunkPos))
    {
        return BlockHandle();
    }
    auto& chunk = _registry.get<ChunkBlocks>(getChunkEntity(chunkPos));
    auto blockState = chunk.getBlockAt(blockPosToChunkLocalPos(blockPos));

    BlockHandle handle;
    handle.blockPos = blockPos;
    handle.id = blockState.id;
    handle.stateCode = blockState.stateCode;
    
    // 尝试获取实体
    Vec2i localPos = BlockLayer::blockPosToChunkLocalPos(blockPos);
    if (chunk.hasChunkEntityAt(localPos))
    {
        handle.blockEntiy = chunk.getEntityAt(localPos);
    }
    
    return handle;
}

bool BlockLayer::setBlockAtBlockPos(const Vec2i& blockPos, const BlockHandle& state)
{

    auto chunkPos = blockPosToChunkPos(blockPos);

    // todo 可能会移除，减少一点性能消耗，安全性由blockWorld保证
    if (!hasChunkExist(chunkPos))
    {
        return false;
    }
    auto& chunk = _registry.get<ChunkBlocks>(getChunkEntity(chunkPos));
    chunk.setBlockAt(blockPosToChunkLocalPos(blockPos), BlockState(state.id.value(), state.stateCode));
    return true;
}

entt::entity const BlockLayer::getChunkEntityAtWorldPos(const cocos2d::Vec2& worldPos) const
{
    return getChunkEntity(worldPosToChunkPos(worldPos));
}

entt::entity const BlockLayer::getChunkEntity(const Vec2i& chunkPos) const
{
    return _chunkMappings.at(chunkPos);
}

LayerType BlockLayer::getLayerType() const
{
    return _layerType;
}

const std::unordered_map<Vec2i, entt::entity>& BlockLayer::getChunkMappings() 
{
    return _chunkMappings;
}