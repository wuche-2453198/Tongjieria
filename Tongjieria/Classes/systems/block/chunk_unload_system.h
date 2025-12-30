#pragma once
#include "entt/entt.hpp"
#include "block_system_manager.h"

/**
* @brief 根据优先级卸载区块。
*
* 优先级 < 0 的方块会进入卸载流。
* - 如果区块被修改过，推入保存流
* - 如果区块没有被修改过，直接卸载。
*
* 特别地，如果一个区块自生成以来没有被修改过，那么它将不会进入保存流。
* 这意味着，这个区块在每次被加载的时候都会被世界系统从噪声中重新生成。
* 但是性能仍然是可接受的，因为系统IO的性能消耗更高。
*/
class ChunkUnloadSystem : public ISystem
{
public:
    ChunkUnloadSystem(entt::registry& registry, entt::dispatcher& dispatcher);
    ~ChunkUnloadSystem();
    void update(float delta);
private:
};