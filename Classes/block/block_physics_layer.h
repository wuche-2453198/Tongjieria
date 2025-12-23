#pragma once
#include <unordered_map>
#include "utils/vec2i.h"
#include "cocos2d.h"

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