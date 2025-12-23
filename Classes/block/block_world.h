#include <optional>
#include "entt/entt.hpp"
#include "utils/vec2i.h" 
#pragma once

using optional_id = std::optional<entt::id_type>;
using optional_entity = std::optional<entt::entity>;
using state = uint8_t;

/**
* @brief 方块类型枚举。
*/
enum class BlockType
{
    BLOCK, WALL
};

/**
* @brief 方块状态结构体。
* 本质是一个方块的描述符。用于传递方块信息。
*
* @todo 完善方块状态
*/
struct BlockState {
    BlockState(
        const Vec2i& pos = Vec2i(0,0),
        optional_id block_id = std::nullopt, 
        optional_entity entity = std::nullopt,
        state state_code = 0, 
        BlockType type = BlockType::BLOCK)
        : blockPos(pos), id(block_id), blockEntiy(entity),
          stateCode(state_code), blockType(type) {}

    /**
    * @brief 检查方块ID是否有效。
    * 
    * @return true 如果ID有效，否则返回false。
    */
    bool isIDVailed() const { return id.has_value(); }

    /**
    * @brief 检查方块实体是否有效。
    * 
    * @return true 如果实体有效，否则返回false。
    */
    bool isEntityVailed() const { return blockEntiy.has_value(); }

    Vec2i blockPos;                 ///< 方块位置
    optional_id id;                 ///< 方块id
    optional_entity blockEntiy;     ///< 方块实体
    state stateCode;                ///< 方块状态码
    BlockType blockType;            ///< 方块类型
};

namespace cocos2d
{
    class Vec2;
}

class Vec2i;
class BlockLayer;
class AssetManager;

/**
* @brief 方块世界访问中心，提供方块的读写接口。
* 
* 本类保证其使用方法都是安全的，不会引发未定义行为。
* 本类型是基础互动的入口，提供方块的读取，放置，破坏和交互接口。
* - 如果试图触发对不存在方块的操作，函数会返回失败，同时事件不会被发送。
* - 对于外部系统，所有方块的读写都应通过本类进行。
* 由于本类依赖于方块层，因此在使用本类之前，必须确保方块层已经初始化完成。
* 
* @note 本类不直接参与任何逻辑，仅提供接口。
* @note 本类不保证方块的存在性，调用方应自行检查返回值。
*/
class BlockWorld
{
public:
    BlockWorld(entt::registry& registry, entt::dispatcher& dispatcher);
    ~BlockWorld();

    /**
    * @brief 获取指定位置的方块状态。
    * 
    * @param pos 方块在世界中的位置
    * @return 方块状态
    */
    BlockState getBlockAtBlockPos(const Vec2i& BlockPos) const;

    /**
    * @brief 获取指定世界位置的方块状态。
    * 
    * @param pos 世界坐标
    * @return 方块状态
    */
    BlockState getBlockAtWorldPos(const cocos2d::Vec2& worldPos) const;

    /**
    * @brief 尝试与指定位置的方块交互。
    * 
    * @param pos 方块坐标
    * @param interactor 交互者实体
    * @return 是否成功交互
    */
    bool TryInteract(const Vec2i& pos, entt::entity interactor);

    /**
    * @brief 尝试与指定位置的方块交互。
    * 
    * @param pos 世界坐标
    * @param interactor 交互者实体
    * @return 是否成功交互
    */
    bool TryInteractAtWorldPos(const cocos2d::Vec2& pos, entt::entity interactor);

    /**
    * @brief 尝试破坏指定位置的方块。
    * 
    * @param pos 方块坐标
    * @param destroyer 破坏者实体
    * @return 是否成功破坏
    */
    bool TryDestroy(const Vec2i& pos, entt::entity destroyer);

    /**
    * @brief 尝试破坏指定位置的方块。
    * 
    * @param pos 世界坐标
    * @param destroyer 破坏者实体
    * @return 是否成功破坏
    */
    bool TryDestroyAtWorldPos(const cocos2d::Vec2& pos, entt::entity destroyer);

    /**
    * @brief 尝试放置方块到指定位置。
    * 
    * @param pos 方块坐标
    * @param block_id 要放置的方块类型ID
    * @param block_state 要放置的方块状态
    * @param placer 放置者实体
    * @return 是否成功放置
    */
    bool TryPlace(const Vec2i& pos, entt::id_type block_id, state block_state, entt::entity placer);

    /**
    * @brief 尝试放置方块到指定位置。
    * 
    * @param pos 世界坐标
    * @param block_id 要放置的方块类型ID
    * @param block_state 要放置的方块状态
    * @param placer 放置者实体
    * @return 是否成功放置
    */
    bool TryPlaceAtWorldPos(const cocos2d::Vec2& pos, entt::id_type block_id, state block_state, entt::entity placer);
private:
    entt::registry& _registry;  ///< 世界组件总线
    entt::dispatcher& _dispatcher; ///< 世界事件总线
    const BlockLayer& _blockLayer; ///< 方块层
    AssetManager& _assetManager; ///< 资源管理器
};