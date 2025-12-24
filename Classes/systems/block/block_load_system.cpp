#include "components/block/block_component.h"
#include "block_load_system.h"

BlockLoadSystem::BlockLoadSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : ISystem(registry, dispatcher) {}
BlockLoadSystem::~BlockLoadSystem() = default;

void BlockLoadSystem::update(float delta)
{
    // 获取未生成方块的区块
    auto view = _registry.view<Position, ChunkHead>(entt::exclude<ChunkBlocks>);
    view.each([&](entt::entity entity, Position& pos, ChunkHead& head)
        {
            auto& blocks = _registry.emplace<ChunkBlocks>(entity);

            // 为区块中的每个方块生成方块ID
            for (int y = 0; y < CHUNK_SIZE; y++)
            {
                for (int x = 0; x < CHUNK_SIZE; x++)
                {
                    blocks.setBlockAt({ x,y }, WorldGenAt(pos.getPostion() + Vec2i(x, y)));
                }
            }
        });
}

ChunkBlocks BlockLoadSystem::loadChunk()
{
    return ChunkBlocks();
}

ChunkBlocks BlockLoadSystem::generateChunk()
{
    return ChunkBlocks();
}

entt::id_type BlockLoadSystem::WorldGenAt(const Vec2i& pos)
{
    // 一个非常简单的世界生成函数
    if (pos.y > 4)
    {
        return entt::hashed_string("air");
    }
    else if (pos.y > 0)
    {
        return entt::hashed_string("dirt");
    }
    else
    {
        return entt::hashed_string("stone");
    }
}