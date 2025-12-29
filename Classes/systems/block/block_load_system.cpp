#include "components/block/block_component.h"
#include "systems/block_layer/block_layer.h"
#include "block_load_system.h"

BlockLoadSystem::BlockLoadSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : ISystem(registry, dispatcher) {}
BlockLoadSystem::~BlockLoadSystem() = default;

void BlockLoadSystem::update(float delta)
{
    auto view = _registry.view<Position, ChunkHead>(entt::exclude<ChunkBlocks>);
    static int chunksNeedingGen = 0;
    view.each([this](auto entity, Position& pos, ChunkHead& head)
        {
            if (chunksNeedingGen < 5 && head.getLayerType() == LayerType::BLOCK) {  // Only log first 5 BLOCK chunks
                Vec2i chunkPos = BlockLayer::worldPosToChunkPos(pos);
                CCLOG("[BlockLoadSystem] Chunk (%d,%d) needs generation, adding NeedGen tag",
                      chunkPos.x, chunkPos.y);
                chunksNeedingGen++;
            }
            _registry.emplace<NeedGen>(entity);
        });
}