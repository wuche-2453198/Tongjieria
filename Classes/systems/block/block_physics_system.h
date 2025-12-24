#pragma once
#include "entt/entt.hpp"
#include "block_system_manager.h"

namespace cocos2d
{
    class PhysicsBody;
    class PhysicsShapeBox;
    class Node;
}

class BlockPhysicsLayer;
class AssetManager;
class Position;
class PhysicsTicket;

/**
* @brief 处理区块的物理属性。
*
* 该类会自动处理脏标记，动态创建物理形状，维护物理形状索引表。
*
* 任何有**位置**和**物理票**的实体才会触发物理形体更新。
* - 在物理票内部的方块，会生成物理形体。
* - 方块在离开物理票的时候，会移除物理形体。
* - 物理形体会根据方块的配置生成。
* 使用者可以合理设置物理票的偏移来优化性能，如根据速度设置偏移，省略掉一些不必要的物理形体。
*/
class BlockPhysicsSystem : public ISystem
{
public:
    BlockPhysicsSystem(entt::registry& registry, entt::dispatcher& dispatcher);
    ~BlockPhysicsSystem();
    void update(float delta);
private:
    std::vector<Vec2i> getAllAddIn();
    std::vector<Vec2i> getAllRemoveOut();
    void addAll(std::vector<Vec2i> allAdded);
    void removeAll(std::vector<Vec2i> allRemoved);
    void updateDirtyBlock();
    bool isInside(const Vec2i& blockPos, const Vec2i& blockUpperLeft, const Vec2i& blockLowerRight);
    bool hasCollision(const Vec2i& blockPos);

    /**
    * @brief 获取区块左上角的坐标。
    *
    * @param worldPos 世界坐标
    * @param ticket 物理票
    */
    Vec2i getUpperLeft(const Position& worldPos, const PhysicsTicket& ticket);

    /**
    * @brief 获取区块右下角的坐标。
    *
    * @param worldPos 世界坐标
    * @param ticket 物理票
    */
    Vec2i getLowerRight(const Position& worldPos, const PhysicsTicket& ticket);

    /**
    * @brief 在区块的坐标上创建一个物理形状。
    *
    * @param blockPos 方块坐标
    * @return 物理形状，如果创建失败或方块没有碰撞，返回nullptr
    */
    cocos2d::PhysicsShapeBox* createBoxAtBlockPos(const Vec2i& blockPos);
    cocos2d::Node* _physicsNode = nullptr;
    cocos2d::PhysicsBody* _body = nullptr;

    AssetManager& _assetManager;        ///< 资源管理器
    BlockLayer& _blockLayer;            ///< 方块层
    BlockPhysicsLayer& _physicsLayer;   ///< 物理层
};