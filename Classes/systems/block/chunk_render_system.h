#pragma once
#include "block_system_manager.h"

class ChunkBlocks;
class BlockBatchCommand;

/**
* @brief 区块渲染系统。
*
* 该系统负责生成区块渲染指令。
* - 当区块被添加的时候，生成区块渲染指令。
* - 当区块被移除的时候，移除区块渲染指令。
* - 根据脏标记，动态地修改渲染指令。
*
* 根据逻辑层的标记和数据来进行自动工作。
*/
class ChunkRenderCommandSystem : public ISystem
{
public:
    ChunkRenderCommandSystem(entt::registry& registry, entt::dispatcher& dispatcher);
    ~ChunkRenderCommandSystem();
    void update(float delta);
private: 
    void updateChunkCommand();
    void updateUnseen();
    void updateDirtyBlock();
};