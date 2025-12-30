#pragma once
#include "block_system_manager.h"
#include "entt/entt.hpp"

class BlockEntityCleanSystem : public ISystem
{
public:
    BlockEntityCleanSystem(entt::registry& registry, entt::dispatcher& dispatcher);
    ~BlockEntityCleanSystem();
    void update(float delta);
private:
    
};