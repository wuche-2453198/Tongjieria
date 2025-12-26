#pragma once
#include <memory>
#include "entt/entt.hpp"
#include "utils/vec2i.h"
#include "core/block_world.h"

namespace cocos2d {
    class Vec2;
}

class AssetManager;
class ChunkHead;

struct BlockState
{
    BlockState() : id(entt::null), stateCode(0) {}
    BlockState(entt::id_type id, state stateCode) : id(id), stateCode(stateCode) {}
    entt::id_type id;   ///< 方块ID
    state stateCode;    ///< 方块状态
};

/*
* @brief 方块世界的高级访问中心，提供一套区块索引和基础方法，以供快速查询和使用。
* 
* 本身不直接参与任何逻辑。但是区块索引的维护由其自身实现，本质是高级容器。
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
    entt::entity addChunk(const Vec2i& chunkPos, entt::entity entity);

    /**
    * @brief 移除区块ID映射.
    * 
    * @param pos 区块坐标
    */
    void removeChunk(const Vec2i& chunkPos);

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
    * @param blockPos 方块坐标
    * @return 区块坐标
    */
    static Vec2i blockPosToChunkPos(const Vec2i& blockPos);

    /**
    * @brief 将方块坐标转换到区块局部坐标
    * 
    * @param blockPos 方块坐标
    * @return 区块局部坐标
    */
    static Vec2i blockPosToChunkLocalPos(const Vec2i& blockPos);

    bool hasChunkExistAtWorldPos(const cocos2d::Vec2& worldPos);

    /**
    * @brief 方块坐标下是否存在已加载区块
    * 
    * @param blockPos 方块坐标
    * @return 是否存在已加载区块
    */
    bool hasChunkExistAtBlockPos(const Vec2i& blockPos) const;

    /**
    * @brief 区块坐标下是否存在已加载区块
    * 
    * @param chunkPos 区块坐标
    * @return 是否存在已加载区块
    */
    bool hasChunkExist(const Vec2i& chunkPos) const;

    BlockHandle getBlockAtWorldPos(const cocos2d::Vec2& pos) const;

    bool setBlockAtWorldPos(const cocos2d::Vec2& pos, const BlockHandle& state);

    BlockHandle getBlockAtBlockPos(const Vec2i& pos) const;

    bool setBlockAtBlockPos(const Vec2i& pos, const BlockHandle& state);

    entt::entity const getChunkAtWorldPos(const Vec2i& pos) const;

    entt::entity const getChunk(const Vec2i& pos) const;

    const std::unordered_map<Vec2i, entt::entity>& getChunkMappings();
private:

    entt::registry& _registry;
    std::unordered_map<Vec2i, entt::entity> _chunkMappings;
};