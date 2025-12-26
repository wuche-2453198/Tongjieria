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

/*
* @brief 方块世界的高级访问中心，提供一套区块索引和基础方法，以供快速查询和使用。
* 
* 本身不直接参与任何逻辑。但是维护一套基于区块稀疏表的数据结构。
* @note 不应当使用registry进行区块头的添加，应该使用方块层的方法。
*/
class BlockLayer
{
public:
    BlockLayer(entt::registry& registry);
    ~BlockLayer();

    /**
    * @brief 添加区块头
    * 
    * @param pos 区块坐标
    */
    std::pair<entt::entity, ChunkHead&> addChunk(const Vec2i& chunkPos);

    /**
    * @brief 移除区块映射.
    * 
    * @param pos 区块坐标
    */
    void destroyChunk(const Vec2i& chunkPos);

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

    /**
    * @brief 世界坐标下是否存在已加载区块
    * 
    * @param worldPos 世界坐标
    * @return 是否存在已加载区块
    */
    bool hasChunkExistAtWorldPos(const cocos2d::Vec2& worldPos) const;

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

    /**
    * @brief 获取世界位置下的方块
    * 
    * @param worldPos 世界坐标
    * @return 方块句柄
    */
    BlockHandle getBlockAtWorldPos(const cocos2d::Vec2& worldPos) const;

    /**
    * @brief 设置世界位置下的方块
    * 
    * @param worldPos 世界坐标
    * @return 真，如果放置成功
    */
    bool setBlockAtWorldPos(const cocos2d::Vec2& worldPos, const BlockHandle& state);

    /**
    * @brief 获取方块坐标下的方块
    * 
    * @param blockPos 方块坐标
    * @return 方块句柄
    */
    BlockHandle getBlockAtBlockPos(const Vec2i& blockPos) const;

    /**
    * @brief 设置方块坐标下的方块
    * 
    * @param blockPos 方块坐标
    * @return 真，如果放置成功
    */
    bool setBlockAtBlockPos(const Vec2i& blockPos, const BlockHandle& state);

    /**
    * @brief 获取世界坐标下的区块
    * 
    * @param worldPos 世界坐标
    * @return 区块实体id
    */
    entt::entity const getChunkAtWorldPos(const cocos2d::Vec2& worldPos) const;

    /**
    * @brief 获取区块坐标下的区块
    * 
    * @param chunkPos 区块坐标
    * @return 区块实体id
    */
    entt::entity const getChunk(const Vec2i& chunkPos) const;

    /**
    * @brief 获取区块映射表
    */
    const std::unordered_map<Vec2i, entt::entity>& getChunkMappings();
private:

    entt::registry& _registry;                              ///< 组件总线
    std::unordered_map<Vec2i, entt::entity> _chunkMappings; ///< 区块映射
};