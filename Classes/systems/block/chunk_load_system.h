#pragma once
#include "block_system_manager.h"
#include "entt/entt.hpp"

/**
* @brief 区块加载系统。读取任何有**位置**且持有**加载票**的实体，根据它们的加载票加载对应区块。
*
* 区块的加载，读写权限和根据**优先级**决定。
* - 区块具有对应的**优先级**，计算公式为：优先级 = 加载票的半径 - 区块离加载中心有多少**层**。
* - 若区块附近有多个加载票，其优先级会取最高的那个。
* - 优先级 = 0的区块可以渲染，但是无法读写。
* - 优先级 < 0的区块会进入卸载流。
* - 优先级 > 0的区块会进入加载流。
*
* 因此一张票的理论最大加载范围是: （2 * 加载票半径 + 1）^ 2。其中最外层为软加载。
*
* 加载系统不会初始化区块的任何数据，加载系统只加载区块头。区块的内容由其他系统异步加载。
*
* @see LoadingTicket
*/
class ChunkLoadSystem : public ISystem
{
public:
    static inline int UPPER_LIMIT = 40;
    static inline int LOWWER_LIMIT = 0;
    ChunkLoadSystem(entt::registry& registry, entt::dispatcher& dispatcher);
    ~ChunkLoadSystem();
    void update(float delta);
private:

    void AddNewChunk();

    void AddChunk(LayerType layer, const Vec2i& chunkPos);

    /**
    * @brief 计算目前所有区块的优先级。
    */
    void updatePriority();

    /**
    * @brief 辅助检查函数，该位置是否合法。
    */
    bool isValiedChunkPos(const Vec2i& chunkPos);

    /**
    * @brief 工具函数，获取chunkPos在center的第几层外。
    */
    int inLayer(const Vec2i& center, const Vec2i& chunkPos);

    BlockWorld& _blockWorld;
};