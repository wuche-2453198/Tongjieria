#include "block_gen_system.h"
#include "components/block/block_component.h"
#include "noise/noise.h"

#define BLOCK_GEN_LOG 0

BlockGenSystem::BlockGenSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : ISystem(registry, dispatcher) 
{
    initModule();
}

BlockGenSystem::~BlockGenSystem() {}

void BlockGenSystem::update(float delta)
{
    auto view = _registry.view<Position, ChunkHead, NeedGen>();
    view.each([&](entt::entity entity, Position& pos, ChunkHead& head)
        {
            auto& blocks = _registry.emplace<ChunkBlocks>(entity);
            for (int y = 0; y < CHUNK_SIZE; y++)
            {
                for (int x = 0; x < CHUNK_SIZE; x++)
                {
                    auto blockPos = BlockLayer::worldPosToBlockPos(pos) + Vec2i(x,y);
                    if (head.getLayerType() == LayerType::BLOCK)
                    {
                        blocks.setBlockAt({ x,y }, genBlockAt(blockPos));
                    }
                    else
                    {
                        blocks.setBlockAt({ x,y }, genWallAt(blockPos));
                    }
                }
            }
            _registry.remove<NeedGen>(entity);
        });
}

void BlockGenSystem::initModule()
{
    perlin.SetFrequency(0.05f);
    perlin.SetOctaveCount(3);
    perlin.SetPersistence(0.5f);
    perlin.SetLacunarity(2.0f);
    perlin.SetSeed(114514);

    scaleBias.SetSourceModule(0, perlin);
    scaleBias.SetScale(10.0f);
    scaleBias.SetBias(60.0f);
}

BlockState BlockGenSystem::testGenBlockAt(const Vec2i& blockPos)
{
    // һ���ǳ��򵥵��������ɺ���
    if (blockPos.y > 4)
    {
        return BlockState((entt::id_type)entt::hashed_string("air"), 0);
    }
    else if (blockPos.y > 0)
    {
        return BlockState((entt::id_type)entt::hashed_string("dirt"), 0);
    }
    else
    {
        return BlockState((entt::id_type)entt::hashed_string("stone"), 0);
    }
}

BlockState BlockGenSystem::testGenWallAt(const Vec2i& blockPos)
{
    // һ���ǳ��򵥵��������ɺ���
    if (blockPos.y > 10)
    {
        return BlockState((entt::id_type)entt::hashed_string("air_wall"), 0);
    }
    else if (blockPos.y <= 10)
    {
        return BlockState((entt::id_type)entt::hashed_string("dirt_wall"), 0);
    }
}

BlockState BlockGenSystem::genBlockAt(const Vec2i& blockPos)
{
    if (blockPos.y < scaleBias.GetValue(blockPos.x, 0, fixedZ))
    {
        return BlockState(entt::hashed_string("dirt"),0);
    }
    else
    {
        return BlockState(entt::hashed_string("air"), 0);
    }
}

BlockState BlockGenSystem::genWallAt(const Vec2i& blockPos)
{

    return BlockState::AIR;
}
