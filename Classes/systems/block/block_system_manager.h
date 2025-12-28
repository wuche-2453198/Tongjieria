#pragma once
#include "entt/entt.hpp"

class BlockWorld;

class ISystem
{
public:
    ISystem(entt::registry& registry, entt::dispatcher& dispatcher);
    virtual ~ISystem();
    virtual void update(float delta);
protected:
    entt::registry& _registry;
    entt::dispatcher& _dispatcher;
    BlockWorld& _blockWorld;
};

class Vec2i;
class BlockLayer;
class ChunkLoadSystem;
class BlockLoadSystem;
class BlockGenSystem;
class ChunkUnloadSystem;
class BlockMiningSystem;
class BlockEntityCleanSystem;
class BlockPhysicsSystem;
class ChunkRenderSystem;
class BlockInteractSystem;
class DebugSystem;
class NpcBlockTicketSyncSystem;

class BlockSystemManager {
public:
    BlockSystemManager(entt::registry& registry, entt::dispatcher& dispatcher);
    ~BlockSystemManager();
    void update(float delta);
private:
    entt::registry& _registry;
    entt::dispatcher& _dispatcher;

    std::unique_ptr<NpcBlockTicketSyncSystem> _npcBlockTicketSyncSystem = nullptr;
    std::unique_ptr<ChunkLoadSystem> _chunkLoadSystem = nullptr;
    std::unique_ptr<BlockLoadSystem> _blockLoadSystem = nullptr;
    std::unique_ptr<BlockGenSystem> _blockGenSystem = nullptr;
    std::unique_ptr<ChunkUnloadSystem> _chunkUnloadSystem = nullptr;
    std::unique_ptr<BlockMiningSystem> _blockMiningSystem = nullptr;
    std::unique_ptr<BlockEntityCleanSystem> _blockEntityCleanSystem = nullptr;
    std::unique_ptr<BlockPhysicsSystem> _blockPhysicsSystem = nullptr;
    std::unique_ptr<ChunkRenderSystem> _chunkRenderCommandSystem = nullptr;
    std::unique_ptr<BlockInteractSystem> _blockInteractSystem = nullptr;
    std::unique_ptr<DebugSystem> _debugSystem = nullptr;
};