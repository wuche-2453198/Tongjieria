#pragma once
#include "block_system_manager.h"

class ChunkBlocks;
class BlockBatchCommand;
class ChunkRenderBatchID;
class CustomcommandPack;

/**
* @class ChunkRenderSystem 
* 
* @brief 区块渲染系统。
*
* 该系统负责生成区块渲染指令。
* - 当区块被添加的时候，生成区块渲染指令。
* - 当区块被移除的时候，移除区块渲染指令。
* - 根据脏标记，动态地修改渲染指令。
* 渲染系统会剔除一定距离外的区块渲染，以提高性能。
* 
* 修改的原理是重新生成区块的几何数据，目前性能影响较小。
* 
* 根据逻辑层的标记和数据来进行自动工作。
* 
* @todo 会生成一个缓存组件以供性能优化，但是这个组件还没有使用。
*/
class ChunkRenderSystem : public ISystem
{
public:
    ChunkRenderSystem(entt::registry& registry, entt::dispatcher& dispatcher);
    ~ChunkRenderSystem();
    void update(float delta);
private: 
    void updateChunkCommand();
    void updateUnseen();
    void updateDirtyBlock();
    void constructPack(const Vec2i chunkPos, const ChunkBlocks& blocks, CustomcommandPack& pack, ChunkRenderBatchID& id);
};