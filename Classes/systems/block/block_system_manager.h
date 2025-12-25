#pragma once
#include "entt/entt.hpp"

class ISystem
{
public:
    ISystem(entt::registry& registry, entt::dispatcher& dispatcher);
    virtual ~ISystem();
    void update(float delta);
protected:
    entt::registry& _registry;
    entt::dispatcher& _dispatcher;
};

class Vec2i;
class BlockLayer;
class ChunkLoadSystem;
class BlockLoadSystem;
class ChunkUnloadSystem;
class BlockPhysicsSystem;
class ChunkRenderSystem;
class BlockInteractSystem;
class DebugSystem;

class BlockSystemManager {
public:
    BlockSystemManager(entt::registry& registry, entt::dispatcher& dispatcher);
    ~BlockSystemManager();
    void update(float delta);
private:
    entt::registry& _registry;
    entt::dispatcher& _dispatcher;

    std::unique_ptr<ChunkLoadSystem> _chunkLoadSystem = nullptr;
    std::unique_ptr<BlockLoadSystem> _blockLoadSystem = nullptr;
    std::unique_ptr<ChunkUnloadSystem> _chunkUnloadSystem = nullptr;
    std::unique_ptr<BlockPhysicsSystem> _blockPhysicsSystem = nullptr;
    std::unique_ptr<ChunkRenderSystem> _chunkRenderCommandSystem = nullptr;
    std::unique_ptr<BlockInteractSystem> _blockInteractSystem = nullptr;
    std::unique_ptr<DebugSystem> _debugSystem = nullptr;
};