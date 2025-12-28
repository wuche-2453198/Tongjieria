#include "block_behavior.h"
#include "systems/block_layer/block_layer.h"
#include "core/block_world.h"

BlockBehaviorRegistry::BlockBehaviorRegistry(entt::registry& registry, entt::dispatcher& dispatcher)
{
    _behaviors[entt::hashed_string("dirt_behavior")] = std::make_shared<DirtBehavior>(registry, dispatcher);
    _behaviors[entt::hashed_string("flower_behavior")] = std::make_shared<FlowerBehavior>(registry, dispatcher);
}

BlockBehaviorRegistry::~BlockBehaviorRegistry() = default;

const std::shared_ptr<BlockBehavior> const BlockBehaviorRegistry::getBehavior(entt::id_type id) const
{
    if(_behaviors.find(id) != _behaviors.end()) 
    {
        return _behaviors.at(id);
    }
    else
    {
        return nullptr;
    }
}

BlockBehavior::BlockBehavior(entt::registry& registry, entt::dispatcher& dispatcher)
    : _registry(registry), _dispatcher(dispatcher){}

DirtBehavior::DirtBehavior(entt::registry& registry, entt::dispatcher& dispatcher)
    : BlockBehavior(registry, dispatcher) {}

bool DirtBehavior::canBePlaced(const BlockPlacedEvent& event, const BlockHandle& blockHandle) { return true; }
void DirtBehavior::onBlockPlaced(const BlockPlacedEvent& event, const BlockHandle& blockHandle) {}
bool DirtBehavior::canBeDestroyed(const BlockDestroyEvent& event, const BlockHandle& blockHandle) { return true; }
void DirtBehavior::onBlockDestroyed(const BlockDestroyEvent& event, const BlockHandle& blockHandle) {}
bool DirtBehavior::canBeMined(const BlockMinedEvent& event, const BlockHandle& blockHandle) { return true; }
void DirtBehavior::onBlockMined(const BlockMinedEvent& event, const BlockHandle& blockHandle) {}
void DirtBehavior::onBlockNeighborChanged(const BlockChangedEvent& event, const BlockHandle& blockHandle) {}
bool DirtBehavior::canBeInteracted(const BlockInteractEvent& event, const BlockHandle& blockHandle) { return false; }
void DirtBehavior::onBlockInteracted(const BlockInteractEvent& event, const BlockHandle& blockHandle) {}
void DirtBehavior::onRamdomTick() {}

FlowerBehavior::FlowerBehavior(entt::registry& registry, entt::dispatcher& dispatcher)
    : DirtBehavior(registry, dispatcher) {}

bool FlowerBehavior::canBePlaced(const BlockPlacedEvent& event, const BlockHandle& blockHandle)
{
    auto& layer = _registry.ctx().get<BlockWorld>().getLayer(event.layer);

    auto blockOnFeet = layer.getBlockAtBlockPos(event.blockPos - Vec2i(0, 1));

    if (blockOnFeet.isIDVailed())
    {
        if (blockOnFeet.id.value() == entt::hashed_string("dirt_block") ||
            blockOnFeet.id.value() == entt::hashed_string("grass_block"))
        {
            return true;
        }
    }
    else
    {
        return false;
    }
    return false;
}

void FlowerBehavior::onBlockNeighborChanged(const BlockChangedEvent& event, const BlockHandle& blockHandle)
{
    if (blockHandle.blockPos - event.blockPos == Vec2i(0, 1))
    {
        if (event.id != entt::hashed_string("dirt_block") ||
            event.id != entt::hashed_string("grass_block"))
        {
            _registry.ctx().get<BlockWorld>().tryDestroy(blockHandle.blockType, blockHandle.blockPos, entt::null);
        }
    }
}
