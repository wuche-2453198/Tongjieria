#include "components/block/block_component.h"
#include "block_load_system.h"

BlockLoadSystem::BlockLoadSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : ISystem(registry, dispatcher) {}
BlockLoadSystem::~BlockLoadSystem() = default;

void BlockLoadSystem::update(float delta)
{
    auto view = _registry.view<Position, ChunkHead>(entt::exclude<ChunkBlocks>);
    view.each([this](auto entity, Position& pos, ChunkHead& head)
        {
            _registry.emplace<NeedGen>(entity);
        });
}