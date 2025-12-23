#include "block_physics_layer.h"

void BlockPhysicsLayer::addPhysicsShapeTag(const Vec2i& pos, int tag)
{
    assert(!hasPhysicsShapeTag(pos));
    _physicsBodies[pos] = tag;
}

void BlockPhysicsLayer::removePhysicsShapeTag(const Vec2i& pos)
{
    assert(hasPhysicsShapeTag(pos));
    _physicsBodies.erase(pos);
}

bool BlockPhysicsLayer::hasPhysicsShapeTag(const Vec2i& pos) const
{
    return _physicsBodies.find(pos) != _physicsBodies.end();
}

int BlockPhysicsLayer::getPhysicsBodyTag(const Vec2i& pos) const
{
    return _physicsBodies.at(pos);
}

std::unordered_map<Vec2i, int>& BlockPhysicsLayer::getPhysicsBodyTags()
{
    return _physicsBodies;
}

