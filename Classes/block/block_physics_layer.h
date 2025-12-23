#pragma once
#include <unordered_map>
#include "utils/vec2i.h"
#include "cocos2d.h"

/*
* @brief 方块物理世界的高级容器。
* 
* 存储启用了碰撞的方块坐标和对应的它们的碰撞形状标签。
* 
* 不直接参与逻辑。
*/
class BlockPhysicsLayer
{
public:
    BlockPhysicsLayer() = default;
    ~BlockPhysicsLayer() = default;

    void addPhysicsShapeTag(const Vec2i& pos, int tag);
    void removePhysicsShapeTag(const Vec2i& pos);
    bool hasPhysicsShapeTag(const Vec2i& pos) const;
    int getPhysicsBodyTag(const Vec2i& pos) const;
    std::unordered_map<Vec2i, int>& getPhysicsBodyTags();
private:
    std::unordered_map<Vec2i, int> _physicsBodies;
};