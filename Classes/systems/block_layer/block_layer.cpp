#include "cocos2d.h"
#include "block_layer.h"
#include "core/consts.h"
#include "components/block/block_component.h"

BlockLayer::BlockLayer(entt::registry& registry) : _registry(registry) {}
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
    ChunkHead& head = _registry.emplace<ChunkHead>(entity, chunkPos);
    // 维护区块索引
    _chunkMappings[chunkPos] = entity;

    return { entity, head };
}

void BlockLayer::destroyChunk(const Vec2i& chunkPos) {
    assert(hasChunkExist(chunkPos));
    // 移除实体和区块
    _registry.destroy(getChunk(chunkPos));
    _chunkMappings.erase(chunkPos);
}

BlockHandle BlockLayer::getBlockAtBlockPos(const Vec2i& blockPos) const
{
    auto chunkPos = blockPosToChunkPos(blockPos);
    auto& chunk = _registry.get<ChunkBlocks>(getChunk(chunkPos));
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
    auto& chunk = _registry.get<ChunkBlocks>(getChunk(chunkPos));
    chunk.setBlockAt(blockPosToChunkLocalPos(blockPos), BlockState(state.id.value(), state.stateCode));
    return true;
}

entt::entity const BlockLayer::getChunkAtWorldPos(const cocos2d::Vec2& worldPos) const
{
    return getChunk(worldPosToChunkPos(worldPos));
}

entt::entity const BlockLayer::getChunk(const Vec2i& chunkPos) const
{
    return _chunkMappings.at(chunkPos);
}

const std::unordered_map<Vec2i, entt::entity>& BlockLayer::getChunkMappings() 
{
    return _chunkMappings;
}