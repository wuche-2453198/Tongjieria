#include "core/block_world.h"
#include "chunk_load_system.h"
#include "block_load_system.h"
#include "block_gen_system.h"
#include "chunk_unload_system.h"
#include "block_mining_system.h"
#include "block_entity_clean_system.h"
#include "block_physics_system.h"
#include "chunk_render_system.h"
#include "block_interact_system.h"
#include "debug_system.h" 
#include "npc_block_ticket_sync_system.h"

ISystem::ISystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : _registry(registry), 
    _dispatcher(dispatcher), 
    _blockWorld(_registry.ctx().get<BlockWorld>()) {}

ISystem::~ISystem() = default;

void ISystem::update(float delta) {}

BlockSystemManager::BlockSystemManager(entt::registry& registry, entt::dispatcher& dispatcher)
    : _registry(registry), _dispatcher(dispatcher)
{
    _debugSystem = std::make_unique<DebugSystem>(_registry, _dispatcher);
    _npcBlockTicketSyncSystem = std::make_unique<NpcBlockTicketSyncSystem>(_registry, _dispatcher);

    // 初始化各个子系统
    _chunkLoadSystem = std::make_unique<ChunkLoadSystem>(_registry, _dispatcher);
    _blockLoadSystem = std::make_unique<BlockLoadSystem>(_registry, _dispatcher);
    _blockGenSystem = std::make_unique<BlockGenSystem>(_registry, _dispatcher);
    _chunkUnloadSystem = std::make_unique<ChunkUnloadSystem>(_registry, _dispatcher);
    _blockMiningSystem = std::make_unique<BlockMiningSystem>(_registry, _dispatcher);
    _blockEntityCleanSystem = std::make_unique<BlockEntityCleanSystem>(_registry, _dispatcher);
    _blockPhysicsSystem = std::make_unique<BlockPhysicsSystem>(_registry, _dispatcher);
    _chunkRenderCommandSystem = std::make_unique<ChunkRenderSystem>(_registry, _dispatcher);
    _blockInteractSystem = std::make_unique<BlockInteractSystem>(_registry, _dispatcher);
}

BlockSystemManager::~BlockSystemManager() {}

void BlockSystemManager::update(float delta)
{
    // 按顺序更新各个子系统
    _npcBlockTicketSyncSystem->update(delta);
    _chunkLoadSystem->update(delta);
    _blockLoadSystem->update(delta);
    _blockGenSystem->update(delta);
    _chunkUnloadSystem->update(delta);
    _blockMiningSystem->update(delta);
    _blockEntityCleanSystem->update(delta);
    _blockPhysicsSystem->update(delta);
    _chunkRenderCommandSystem->update(delta);
    _debugSystem->update(delta);
}