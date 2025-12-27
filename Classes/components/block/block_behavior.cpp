#include "block_behavior.h"

BlockBehavior::BlockBehavior(entt::registry& registry) : _registry(registry)
{
}

BlockBehaviorRegistry::BlockBehaviorRegistry()
{
    //_behaviors[entt::hashed_string("dirt")] = std::make_shared<DirtBehavior>();
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

DirtBehavior::DirtBehavior(entt::registry& registry) : BlockBehavior(registry)
{
}

void DirtBehavior::onBlockPlaced(const BlockPlacedEvent& event)
{
}

void DirtBehavior::onBlockDestroyed(const BlockDestroyEvent& event)
{
}

void DirtBehavior::onBlockMined(const BlockMinedEvent& event)
{

}

void DirtBehavior::onBlockNeighborChanged()
{
}

void DirtBehavior::onBlockInteracted(const BlockInteractEvent& event)
{
}

void DirtBehavior::onRamdomTick()
{
}
