#include "components/block/block_component.h"
#include "block_entity_clean_system.h"

#define ENTITY_NUM_LOG 0
#define ENTITY_DESTROY_LOG 1

BlockEntityCleanSystem::BlockEntityCleanSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : ISystem(registry, dispatcher)
{
}

BlockEntityCleanSystem::~BlockEntityCleanSystem()
{
}

void BlockEntityCleanSystem::update(float delta)
{
    auto view = _registry.view<BlockEntityHead>();

#if ENTITY_NUM_LOG
    CCLOG("[BlockEntityCleanSystem]:%s blockEntity: %d", __func__, view.size());
#endif
    view.each([&](entt::entity entity, BlockEntityHead& blockEntity)
        {
            if (blockEntity.componentCount == 0)
            {
#if ENTITY_DESTROY_LOG
                CCLOG("[BlockEntityCleanSystem]: Destory a entity at %d %d!", blockEntity.blockPos.x, blockEntity.blockPos.y);
#endif
                auto& layer = _registry.ctx().get<BlockWorld>().getLayer(blockEntity.layer);
                layer.destroyBlockEntity(blockEntity.blockPos);
            }
        });
}
