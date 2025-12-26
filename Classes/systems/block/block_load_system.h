#pragma once
#include "block_system_manager.h"
#include "entt/entt.hpp"

class ChunkBlocks;

struct BlockState;

/**
* @brief 生成区块的方块。调度加载系统和生成系统。
*
* 使用噪声库生成的噪声（主要是柏林噪声）进行逐方块的生成。
* 性能消耗中等，不建议在每帧调用。
*/
class BlockLoadSystem : public ISystem
{
public:
    BlockLoadSystem(entt::registry& registry, entt::dispatcher& dispatcher);
    ~BlockLoadSystem();
    void update(float delta);
private:
    ChunkBlocks loadChunk();
    ChunkBlocks generateChunk();

    /**
    * @brief 根据世界位置生成方块。
    */
    BlockState WorldGenAt(const Vec2i& pos);
};