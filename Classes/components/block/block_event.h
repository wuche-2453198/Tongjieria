#pragma once
#include "entt/entt.hpp"
#include "core/consts.h"
#include "utils/vec2i.h"

using state = uint32_t;

/*
* @brief 方块相关的事件基类。
*
* 事件由BlockWorld发送。由外部系统触发。
* 对应的方块事件都发生在behavior类被处理之前。
*/
struct BlockEvent
{
    /**
    * @brief 一个事件的基础信息。
    * 
    * @param layer 方块所在的层
    * @param block_id 方块的ID
    * @param pos 方块在世界中的位置
    */
    BlockEvent(LayerType layer, entt::id_type block_id, const Vec2i& pos)
        : layer(layer), id(block_id), blockPos(pos) {}
    LayerType layer;
    entt::id_type id;   ///< 方块类型ID
    Vec2i blockPos;     ///< 方块在世界中的位置
};

/**
* @brief 方块被放置事件。
*/
struct BlockPlacedEvent : public BlockEvent 
{
    BlockPlacedEvent() = default;

    /**
    * @brief 构造一个放置事件
    * 
    * @param layer 方块所在的层
    * @param block_id 方块的ID
    * @param pos 方块在世界中的位置
    * @param placer 放置方块的实体
    */
    BlockPlacedEvent(LayerType layer, entt::id_type block_id, const Vec2i& pos, entt::entity placer)
        : BlockEvent(layer, block_id, pos), placer(placer) {}

    entt::entity placer; ///< 放置方块的实体
};

/**
*  @brief 方块被破坏事件。
*/
struct BlockDestroyEvent : public BlockEvent 
{
    BlockDestroyEvent() = default;

    /**
    * @brief 构造一个破坏事件
    * 
    * @param layer 方块所在的层
    * @param block_id 方块的ID
    * @param pos 方块在世界中的位置
    * @param breaker 破坏方块的实体
    */
    BlockDestroyEvent(LayerType layer, entt::id_type block_id, const Vec2i& pos, entt::entity breaker)
        : BlockEvent(layer, block_id, pos), breaker(breaker) {}

    entt::entity breaker; ///< 破坏方块的实体
};

/**
* @brief 方块状态变化事件。
*/
struct BlockChangedEvent : public BlockEvent 
{
    BlockChangedEvent() = default;

    /**
    * @brief 构造一个状态变化事件
    * 
    * @param layer 方块所在的层
    * @param block_id 方块的ID，变化后的id
    * @param pos 方块在世界中的位置
    * @param old_id 变化前的ID
    * @param old_state 变化前的状态
    * @param new_state 变化后的状态
    */
    BlockChangedEvent(LayerType layer, entt::id_type block_id, const Vec2i& pos,
        entt::id_type old_id, state old_state, state new_state)
        : BlockEvent(layer, block_id, pos), oldID(old_id), oldState(old_state), newState(new_state) {}
    entt::id_type oldID;    ///< 变化前的ID
    state oldState;         ///< 变化前的状态
    state newState;         ///< 变化后的状态
};

/**
* @brief 方块被挖掘事件。
*/
struct BlockMinedEvent : public BlockEvent
{
    BlockMinedEvent() = default;

    /**
    * @brief 构造一个挖掘事件
    * 
    * @param layer 方块所在的层
    * @param block_id 方块的ID
    * @param pos 方块在世界中的位置
    * @param miningFactor 挖掘的效率
    * @param miner 挖掘方块的实体
    */
    BlockMinedEvent(LayerType layer, entt::id_type block_id, const Vec2i& pos, float miningFactor, entt::entity miner)
        : BlockEvent(layer, block_id, pos), miningFactor(miningFactor), miner(miner) {}

    float miningFactor;
    entt::entity miner;    ///< 挖掘方块的实体
};

/**
* @brief 方块交互事件。
*/
struct BlockInteractEvent : public BlockEvent
{
    BlockInteractEvent() = default;

    /**
    * @brief 构造一个交互事件
    * 
    * @param layer 方块所在的层
    * @param block_id 方块的ID
    * @param pos 方块在世界中的位置
    * @param interactor 交互方块的实体
    */
    BlockInteractEvent(LayerType layer, entt::id_type block_id, const Vec2i& pos, entt::entity interactor)
        : BlockEvent(layer, block_id, pos), interactor(interactor) {}

    entt::entity interactor; ///< 交互方块的实体
};