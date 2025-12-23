#include "cocos2d.h"
#include "block_component.h"
#include "chunk_render.h"
#include "server/assets_manager.h"
#include "level/world.h"

Position::Position() {}

Position::Position(float x, float y) : pos(x, y) {};

Position::Position(const Vec2i& vec) : pos(vec.x, vec.y) {};

Position::Position(const cocos2d::Vec2& vec) : pos(vec) {};

Position::operator cocos2d::Vec2() const { return pos; }

Position::operator Vec2i() const { return pos; }

const const cocos2d::Vec2& Position::getPostion() const { return pos; }

void Position::setPosition(const cocos2d::Vec2& pos) { this->pos = pos; }

ChunkHead::ChunkHead() = default;
ChunkHead::ChunkHead(int priority): _priority(priority) {}
ChunkHead::~ChunkHead() = default;

int ChunkHead::getPriority() const { return _priority; }

void ChunkHead::setPriority(int priority) { _priority = priority; }

ChunkBlocks::ChunkBlocks() = default;
ChunkBlocks::~ChunkBlocks() = default;

entt::id_type ChunkBlocks::getBlockAt(const Vec2i& pos) const
{
    return _blocks[pos.x][pos.y];
}

void ChunkBlocks::setBlockAt(const Vec2i& pos, entt::id_type id)
{
    _blocks[pos.x][pos.y] = id;
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

WorldScene::WorldScene(World* world) : _world(world) {}
WorldScene::~WorldScene() = default;
World& WorldScene::operator*() const { return *_world; }
World* WorldScene::operator->() const { return _world; }
WorldScene::operator bool() const { return _world != nullptr; }

CustomcommandPack::CustomcommandPack() = default;

CustomcommandPack::~CustomcommandPack() {
    for (auto ptr : commands) {
        delete ptr;
    }
}

CustomcommandPack::CustomcommandPack(CustomcommandPack&& other) noexcept
    : commands(std::move(other.commands)) 
{  
    other.commands.clear();
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
