#include "entt/entt.hpp"
#include "utils/vec2i.h"
#pragma once

/*
* @brief 方块相关的事件基类。
* 
* 事件由BlockWorld发送。由外部系统触发。
* 对应的方块事件都发生在behavior类被处理之前。
*/


/**
* @brief 表示方块相关的事件基类。
* 对应的方块事件都发生在behavior类被处理之前。
*/
struct BlockEvent
{
    BlockEvent() = default;
    BlockEvent(entt::id_type block_id, const Vec2i& pos)
        : id(block_id), blockPos(pos) {}
    entt::id_type id;  ///< 方块类型ID
    Vec2i blockPos;  ///< 方块在世界中的位置
};

/**
* @brief 方块被放置事件。
*/
struct BlockPlacedEvent : public BlockEvent 
{
    BlockPlacedEvent() = default;
    BlockPlacedEvent(entt::id_type block_id, const Vec2i& pos, entt::entity placer)
        : BlockEvent(block_id, pos), placer(placer) {}

    entt::entity placer; ///< 放置方块的实体
};

/**
*  @brief 方块被破坏事件。
*/
struct BlockDestroyEvent : public BlockEvent 
{
    BlockDestroyEvent() = default;
    BlockDestroyEvent(entt::id_type block_id, const Vec2i& pos, entt::entity breaker)
        : BlockEvent(block_id, pos), breaker(breaker) {}

    entt::entity breaker; ///< 破坏方块的实体
};

/**
* @brief 方块状态变化事件。
*/
struct BlockStateChangedEvent : public BlockEvent 
{
    BlockStateChangedEvent() = default;
    BlockStateChangedEvent(entt::id_type block_id, const Vec2i& pos, uint8_t old_state, uint8_t new_state)
        : BlockEvent(block_id, pos), oldState(old_state), newState(new_state) {}

    uint8_t oldState;    ///< 变化前的状态
    uint8_t newState;    ///< 变化后的状态
};

/**
* @brief 方块被挖掘事件。
*/
struct BlockMinedEvent : public BlockEvent
{
    BlockMinedEvent() = default;
    BlockMinedEvent(entt::id_type block_id, const Vec2i& pos, float progress, entt::entity miner)
        : BlockEvent(block_id, pos), miningProgress(progress), miner(miner) {}

    float miningProgress; ///< 当前挖掘进度
    entt::entity miner;    ///< 挖掘方块的实体
};

/**
* @brief 方块交互事件。
*/
struct BlockInteractEvent : public BlockEvent
{
    BlockInteractEvent() = default;
    BlockInteractEvent(entt::id_type block_id, const Vec2i& pos, entt::entity interactor)
        : BlockEvent(block_id, pos), interactor(interactor) {}

    entt::entity interactor; ///< 交互方块的实体
};