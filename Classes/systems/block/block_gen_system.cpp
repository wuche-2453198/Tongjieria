#include "block_gen_system.h"
#include "components/block/block_component.h"

BlockGenSystem::BlockGenSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : ISystem(registry, dispatcher) {}

BlockGenSystem::~BlockGenSystem() {}

void BlockGenSystem::update(float delta)
{
    // 获取未生成方块的区块
    auto view = _registry.view<Position, ChunkHead, NeedGen>();
    view.each([&](entt::entity entity, Position& pos, ChunkHead& head)
        {
            CCLOG("Gen blocks at %d %d", pos.getPostion().x, pos.getPostion().y);
            auto& blocks = _registry.emplace<ChunkBlocks>(entity);
            

            // 生成方块
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

BlockState BlockGenSystem::genBlockAt(const Vec2i& blockPos)
{
    // 一个非常简单的世界生成函数
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

BlockState BlockGenSystem::genWallAt(const Vec2i& blockPos)
{
    // 一个非常简单的世界生成函数
    if (blockPos.y > 10)
    {
        return BlockState((entt::id_type)entt::hashed_string("air_wall"), 0);
    }
    else if (blockPos.y <= 10)
    {
        return BlockState((entt::id_type)entt::hashed_string("dirt_wall"), 0);
    }
}
