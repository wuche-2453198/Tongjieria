#include "components/block/block_component.h"
#include "systems/block_layer/block_layer.h"
#include "chunk_unload_system.h"

ChunkUnloadSystem::ChunkUnloadSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : ISystem(registry, dispatcher) {}
ChunkUnloadSystem::~ChunkUnloadSystem() = default;

void ChunkUnloadSystem::update(float delta)
{
    auto view = _registry.view<Position, ChunkHead>();
    std::vector<entt::entity> unloadChunks;
    view.each([&](entt::entity entity, Position& pos, ChunkHead& head)
        {
            // 卸载所有低优先级的区块
            if (head.getPriority() < ChunkHead::UNLOADING_PRIORITY)
            {
                _registry.ctx().get<BlockWorld>().getLayer(head.getLayerType()).
                    destroyChunk(BlockLayer::worldPosToChunkPos(pos.getPostion()));
            }
        });
}