#pragma once
#include "block_system_manager.h"

class ChunkBlocks;
class BlockBatchCommand;
class ChunkRenderBatchID;
class CustomcommandPack;
class AssetManager;

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
* 
* @tease f**k 图形学你害人不浅，哎哟卧槽open gay l 怎么这么坏啊。
* 彩蛋：顶点数组jumpscare!!!!!!
* verts=
* {0.0f,0.0f},{0.0f,0.5f},{0.5f,0.5f},{0.5f,0.0f}
* {0.5f,0.0f},{0.5f,0.5f},{1.0f,0.5f},{1.0f,0.0f}
* indces=
* {0,1,2,2,3,0,4,5,6,6,7}
* 害怕 open gay l 的小朋友已经跑了，你还在看吗？
*/
class ChunkRenderSystem : public ISystem
{
public:
    ChunkRenderSystem(entt::registry& registry, entt::dispatcher& dispatcher);
    ~ChunkRenderSystem();
    void update(float delta);
private: 

    /**
    * @brief 生成区块渲染指令
    */
    void createCommand();

    /**
    * @brief 剔除过远的指令
    */
    void cutCommand();

    /**
    * @brief 更新脏区块的渲染指令
    */
    void updateDirtyBlock();

    /**
    * @brief 构建一个区块的命令
    * 
    * @param chunkPos 区块坐标
    * @blocks 区块数据
    * @param pack 区块渲染指令包
    * @param id 区块渲染索引缓存
    */
    void constructPack(LayerType layerType, const Vec2i chunkPos, const ChunkBlocks& blocks, CustomcommandPack& pack);

    AssetManager& _assetManager;
};