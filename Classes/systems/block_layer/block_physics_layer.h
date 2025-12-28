#pragma once
#include <unordered_map>
#include <cstdint>
#include <functional>
#include "utils/vec2i.h"
#include "cocos2d.h"

/*
* @brief 方块物理世界的高级容器。
* 
* 存储启用了碰撞的方块坐标和对应的它们的碰撞形状标签。
* 
* 不直接参与逻辑。
*/
struct BlockPhysicsShapeKey
{
public:
    enum class Kind : std::uint8_t {
        Tile = 0,
        VerticalRun = 1,
    };

    Kind kind = Kind::Tile;
    int x = 0;
    int y0 = 0;
    int y1 = 0;
    bool projectilePass = false;

    bool operator==(const BlockPhysicsShapeKey& other) const
    {
        return kind == other.kind &&
            x == other.x &&
            y0 == other.y0 &&
            y1 == other.y1 &&
            projectilePass == other.projectilePass;
    }
};

struct BlockPhysicsShapeKeyHash
{
    std::size_t operator()(const BlockPhysicsShapeKey& k) const noexcept
    {
        std::size_t h = 0;
        auto mix = [&h](std::size_t v) {
            h ^= v + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
        };
        mix(std::hash<int>()(static_cast<int>(k.kind)));
        mix(std::hash<int>()(k.x));
        mix(std::hash<int>()(k.y0));
        mix(std::hash<int>()(k.y1));
        mix(std::hash<bool>()(k.projectilePass));
        return h;
    }
};

class BlockPhysicsLayer
{
public:
    BlockPhysicsLayer() = default;
    ~BlockPhysicsLayer() = default;

    int addPhysicsShapeTag(const BlockPhysicsShapeKey& key);
    void removePhysicsShapeTag(const BlockPhysicsShapeKey& key);
    bool hasPhysicsShapeTag(const BlockPhysicsShapeKey& key) const;
    int getPhysicsBodyTag(const BlockPhysicsShapeKey& key) const;
    std::unordered_map<BlockPhysicsShapeKey, int, BlockPhysicsShapeKeyHash>& getPhysicsBodyTags();
private:
    std::unordered_map<BlockPhysicsShapeKey, int, BlockPhysicsShapeKeyHash> _physicsBodies;
    int _nextTag = 1;
};