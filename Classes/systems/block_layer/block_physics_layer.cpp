#include "block_physics_layer.h"

int BlockPhysicsLayer::addPhysicsShapeTag(const BlockPhysicsShapeKey& key)
{
    auto it = _physicsBodies.find(key);
    if (it != _physicsBodies.end()) {
        return it->second;
    }
    const int tag = _nextTag++;
    _physicsBodies.emplace(key, tag);
    return tag;
}

void BlockPhysicsLayer::removePhysicsShapeTag(const BlockPhysicsShapeKey& key)
{
    CCASSERT(hasPhysicsShapeTag(key), "不存在该位置的物理形状标记");
    _physicsBodies.erase(key);
}

bool BlockPhysicsLayer::hasPhysicsShapeTag(const BlockPhysicsShapeKey& key) const
{
    return _physicsBodies.find(key) != _physicsBodies.end();
}

int BlockPhysicsLayer::getPhysicsBodyTag(const BlockPhysicsShapeKey& key) const
{
    return _physicsBodies.at(key);
}

std::unordered_map<BlockPhysicsShapeKey, int, BlockPhysicsShapeKeyHash>& BlockPhysicsLayer::getPhysicsBodyTags()
{
    return _physicsBodies;
}
