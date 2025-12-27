#include "cocos2d.h"
#include "block_component.h"
#include "chunk_render.h"
#include "core/assets_manager.h"
#include "core/world.h"

Position::Position() {}

Position::Position(float x, float y) : pos(x, y) {};

Position::Position(const Vec2i& vec) : pos(vec.x, vec.y) {};

Position::Position(const cocos2d::Vec2& vec) : pos(vec) {};

Position::operator cocos2d::Vec2() const { return pos; }

const const cocos2d::Vec2& Position::getPostion() const { return pos; }

void Position::setPosition(const cocos2d::Vec2& pos) { this->pos = pos; }

ChunkHead::ChunkHead(LayerType layerType) 
    : _priority(0), _layerType(layerType) {}
ChunkHead::ChunkHead(LayerType layerType,int priority) 
    : _priority(priority), _layerType(layerType) {}
ChunkHead::~ChunkHead() = default;

LayerType ChunkHead::getLayerType() const { return _layerType; }

int ChunkHead::getPriority() const { return _priority; }

void ChunkHead::setPriority(int priority) { _priority = priority; }

ChunkBlocks::ChunkBlocks() = default;
ChunkBlocks::~ChunkBlocks() = default;

BlockState ChunkBlocks::getBlockAt(const Vec2i& localPos) const
{
    return _blocks[localPos.x][localPos.y];
}

void ChunkBlocks::setBlockAt(const Vec2i& localPos, BlockState state)
{
    _blocks[localPos.x][localPos.y] = state;
}

void ChunkBlocks::setBlockState(const Vec2i& localPos, state stateCode)
{
    _blocks[localPos.x][localPos.y].stateCode = stateCode;
}

bool ChunkBlocks::hasChunkEntity()
{
    return _blockEntities.size();
}

bool ChunkBlocks::hasChunkEntityAt(const Vec2i& localPos)
{
    return _blockEntities.find(localPos) != _blockEntities.end();
}

entt::entity ChunkBlocks::getEntityAt(const Vec2i& localPos)
{
    return _blockEntities.at(localPos);
}

void ChunkBlocks::addEntity(const Vec2i& localPos, entt::entity entity)
{
    assert(!hasChunkEntityAt(localPos));
    _blockEntities[localPos] = entity;
}

void ChunkBlocks::removeEntity(const Vec2i blockPos)
{
    assert(hasChunkEntityAt(blockPos));
    _blockEntities.erase(blockPos);
}

const std::unordered_map<Vec2i, entt::entity>& ChunkBlocks::getEntityMapping()
{
    return _blockEntities;
}

std::vector<entt::entity> ChunkBlocks::getEntites()
{
    std::vector<entt::entity> entities;
    for (auto& entityMapping : _blockEntities)
    {
        entities.push_back(entityMapping.second);
    }
    return entities;
}

bool ChunkBlocks::isPosValied(const Vec2i& pos) const
{
    return 0 <= pos.x && pos.x < CHUNK_SIZE && 
           0 <= pos.y && pos.y < CHUNK_SIZE;
}

const BlockArray& const ChunkBlocks::getBlockView() const
{
    return _blocks;
}


BlockEntityHead::BlockEntityHead(LayerType layer, entt::id_type id, const Vec2i& blockPos)
 : layer(layer), id(id), blockPos(blockPos) {}

MiningProgress::MiningProgress() = default;

MiningProgress::MiningProgress(float progress) : progress(progress) {}


WorldScene::WorldScene(World* world) : _world(world) {}
WorldScene::~WorldScene() = default;
World& WorldScene::operator*() const { return *_world; }
World* WorldScene::operator->() const { return _world; }
WorldScene::operator bool() const { return _world != nullptr; }

CustomcommandPack::CustomcommandPack() = default;

CustomcommandPack::~CustomcommandPack() {
    releaseAllCommand();
}

CustomcommandPack::CustomcommandPack(CustomcommandPack&& other) noexcept
    : commands(std::move(other.commands)) 
{  
    other.commands.clear();
}

void CustomcommandPack::releaseAllCommand()
{
    for (auto ptr : commands) 
    {
        delete ptr;
    }
    commands.clear();
}

bool DirtyBlock::isClean()
{
    return !(collisionDirty || renderDirty);
}

DirtyChunkTag::DirtyChunkTag() {};

void DirtyChunkTag::addDirtyBlock(const Vec2i& pos)
{
    dirtyBlocks.push_back(DirtyBlock(pos));
}

bool DirtyChunkTag::isAllClean()
{
    for (auto& dirtyBlock : dirtyBlocks)
    {
        if (!dirtyBlock.isClean()) return false;
    }
    return true;
}
