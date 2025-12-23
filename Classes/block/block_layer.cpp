#include "cocos2d.h"
#include "block_layer.h"
#include "server/consts.h"
#include "block_component.h"

BlockLayer::BlockLayer(entt::registry& registry) : _registry(registry) {}
BlockLayer::~BlockLayer() {}

void BlockLayer::addChunkID(const Vec2i& chunk_pos, entt::entity entity) {
    assert(!hasChunkExist(chunk_pos));
    _chunkMappings[chunk_pos] = entity;
}

void BlockLayer::removeChunkID(const Vec2i& chunk_pos) {
    assert(hasChunkExist(chunk_pos));
    _chunkMappings.erase(chunk_pos);
}

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

bool BlockLayer::hasChunkExistAtBlockPos(const Vec2i& worldPos) const {
    return hasChunkExist(blockPosToChunkPos(worldPos));
}

bool BlockLayer::hasChunkExist(const Vec2i& chunkPos) const {
    return _chunkMappings.find(chunkPos) != _chunkMappings.end();
}

BlockState BlockLayer::getBlockAtWorldPos(const cocos2d::Vec2& worldPos) const
{
    return getBlockAtBlockPos(worldPosToBlockPos(worldPos));
}

bool BlockLayer::setBlockAtWorldPos(const cocos2d::Vec2& worldPos, const BlockState& state)
{
    return setBlockAtBlockPos(worldPosToBlockPos(worldPos), state);
}

BlockState BlockLayer::getBlockAtBlockPos(const Vec2i& blockPos) const
{
    auto chunkPos = blockPosToChunkPos(blockPos);
    if (!hasChunkExist(chunkPos))
    {
        return BlockState();
    }
    // todo 添加是否初始化检查
    auto& chunk = _registry.get<ChunkBlocks>(getChunkID(chunkPos));
    
    BlockState state;
    state.blockPos = blockPos;
    state.id = chunk.getBlockAt(blockPosToChunkLocalPos(blockPos));
    state.stateCode = 0;      // todo 设置方块状态码
    state.blockEntiy = entt::null; // todo 设置方块实体
    
    // todo 设置其他数据，如方块状态，方块实体
    return state;
}

bool BlockLayer::setBlockAtBlockPos(const Vec2i& blockPos, const BlockState& state)
{
    auto chunkPos = blockPosToChunkPos(blockPos);
    if (!hasChunkExist(chunkPos) || !state.id.has_value())
    {
        return false;
    }
    // todo 添加是否初始化检查
    auto& chunk = _registry.get<ChunkBlocks>(getChunkID(chunkPos));
    chunk.setBlockAt(blockPosToChunkLocalPos(blockPos), state.id.value());

    // todo 设置其他数据，如方块状态，方块实体
    return true;
}

entt::entity const BlockLayer::getChunkIDAtWorldPos(const Vec2i& worldPos) const
{
    return getChunkID(worldPosToChunkPos(worldPos));
}

entt::entity const BlockLayer::getChunkID(const Vec2i& chunkPos) const
{
    return _chunkMappings.at(chunkPos);
}

const std::unordered_map<Vec2i, entt::entity>& BlockLayer::getChunkMappings() 
{
    return _chunkMappings;
}