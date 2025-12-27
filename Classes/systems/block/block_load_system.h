#pragma once
#include "block_system_manager.h"
#include "entt/entt.hpp"

class ChunkBlocks;

struct BlockState;


class BlockLoadSystem : public ISystem
{
public:
    BlockLoadSystem(entt::registry& registry, entt::dispatcher& dispatcher);
    ~BlockLoadSystem();
    void update(float delta);
private:
};