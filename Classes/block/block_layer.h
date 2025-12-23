#include <memory>
#include "entt/entt.hpp"
#include "utils/vec2i.h"
#include "block_world.h"
#pragma once

namespace cocos2d {
    class Vec2;
}

class AssetManager;
class ChunkHead;

/*
* @brief 方块世界的高级访问中心，提供一套区块索引，以供快速查询和使用。
* 本身不直接参与任何逻辑。
*/
class BlockLayer
{
public:
    BlockLayer(entt::registry& registry);
    ~BlockLayer();

    /**
    * @brief 添加区块ID映射.
    * 
    * @param pos 区块坐标
    * @param entity 区块实体ID
    */
    void addChunkID(const Vec2i& pos, entt::entity entity);

    /**
    * @brief 移除区块ID映射.
    * 
    * @param pos 区块坐标
    */
    void removeChunkID(const Vec2i& pos);

    /**
    * @brief 将世界坐标转换到区块坐标
    * 
    * @param worldPos 世界坐标
    * @return 区块坐标
    */
    static Vec2i worldPosToChunkPos(const cocos2d::Vec2& worldPos);

    /**
    * @brief 将世界坐标转换到方块坐标。
    * 
    * @param worldPos 世界坐标
    * @return 方块坐标
    */
    static Vec2i worldPosToBlockPos(const cocos2d::Vec2& worldPos);

    /**
    * @brief 将方块坐标转换到区块坐标。
    * 
    * @param world_pos 方块坐标
    * @return 区块坐标
    */
    static Vec2i blockPosToChunkPos(const Vec2i& blockPos);

    /**
    * @brief 将方块坐标转换到区块局部坐标
    * 
    * @param world_pos 方块坐标
    * @return 区块局部坐标
    */
    static Vec2i blockPosToChunkLocalPos(const Vec2i& blockPos);

    /**
    * @brief 方块坐标下是否存在已加载区块
    * 
    * @param pos 方块坐标
    * @return 是否存在已加载区块
    */
    bool hasChunkExistAtBlockPos(const Vec2i& blockPos) const;

    /**
    * @brief 区块坐标下是否存在已加载区块
    * 
    * @param pos 区块坐标
    * @return 是否存在已加载区块
    */
    bool hasChunkExist(const Vec2i& chunkPos) const;

    BlockState getBlockAtWorldPos(const cocos2d::Vec2& pos) const;

    bool setBlockAtWorldPos(const cocos2d::Vec2& pos, const BlockState& state);

    BlockState getBlockAtBlockPos(const Vec2i& pos) const;

    bool setBlockAtBlockPos(const Vec2i& pos, const BlockState& state);

    entt::entity const getChunkIDAtWorldPos(const Vec2i& pos) const;

    entt::entity const getChunkID(const Vec2i& pos) const;

    const std::unordered_map<Vec2i, entt::entity>& getChunkMappings();
private:



    entt::registry& _registry;
    std::unordered_map<Vec2i, entt::entity> _chunkMappings;
};