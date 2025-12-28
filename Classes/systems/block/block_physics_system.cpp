#include "core/consts.h"
#include "core/world.h"
#include "core/assets_manager.h"
#include "components/block/block_component.h"
#include "systems/block_layer/block_physics_layer.h"
#include "systems/block_layer/block_layer.h"
#include "utils/tools.h"
#include "cocos2d.h"
#include "block_physics_system.h"
#include "debug_system.h"
#include <cmath>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>

#define PHYSICS_TICKET_DEBUG 1
#define PHYSICS_LOG  0

BlockPhysicsSystem::BlockPhysicsSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : ISystem(registry, dispatcher),
    _assetManager(_registry.ctx().get<AssetManager>()),
    _blockLayer(_registry.ctx().get<BlockWorld>().getLayer(LayerType::BLOCK)),
    _physicsLayer(_registry.ctx().get<BlockPhysicsLayer>())
{
#if PHYSICS_LOG
    CCLOG("[BlockPhyicsSystem] construct begin");
#endif // PHYSICS_LOG

    _physicsNode = cocos2d::Node::create();
    _physicsNode->setPosition(cocos2d::Vec2(0.5f, 0.5f) * BLOCK_SIZE);
    _registry.ctx().get<WorldScene>()->addChild(_physicsNode);
    _body = cocos2d::PhysicsBody::create();
    _body->setDynamic(false);
    _physicsNode->addComponent(_body);

    _registry.ctx().insert_or_assign(BlockPhysicsWorldRef{ _physicsNode, _body });

#if PHYSICS_LOG 
    CCLOG("[BlockPhyicsSystem] construct end");
#endif // 

}

BlockPhysicsSystem::~BlockPhysicsSystem() = default;

void BlockPhysicsSystem::update(float delta)
{
    auto desired = collectDesiredShapes();

    std::vector<BlockPhysicsShapeKey> toRemove;
    for (const auto& kv : _physicsLayer.getPhysicsBodyTags())
    {
        if (desired.find(kv.first) == desired.end())
        {
            toRemove.push_back(kv.first);
        }
    }

    std::vector<BlockPhysicsShapeKey> toAdd;
    toAdd.reserve(desired.size());
    for (const auto& key : desired)
    {
        if (!_physicsLayer.hasPhysicsShapeTag(key))
        {
            toAdd.push_back(key);
        }
    }

    removeAll(toRemove);
    addAll(toAdd);
}

std::unordered_set<BlockPhysicsShapeKey, BlockPhysicsShapeKeyHash> BlockPhysicsSystem::collectDesiredShapes()
{
#if PHYSICS_TICKET_DEBUG
    auto drawNode = DebugSystem::getDrawNode(_registry, "phy ticket");
    drawNode->clear();
#endif

    struct ColumnKey {
        int x = 0;
        bool projectilePass = false;
        bool operator==(const ColumnKey& other) const { return x == other.x && projectilePass == other.projectilePass; }
    };
 
    struct ColumnKeyHash {
        std::size_t operator()(const ColumnKey& k) const noexcept {
            std::size_t h1 = std::hash<int>()(k.x);
            std::size_t h2 = std::hash<bool>()(k.projectilePass);
            return h1 ^ (h2 << 1);
        }
    };

    auto queryCollisionMeta = [this](const Vec2i& blockPos, bool& projectilePass, cocos2d::Vec2& collisionShape, cocos2d::Vec2& collisionOffset) -> bool {
        if (!hasCollision(blockPos)) {
            return false;
        }
 
        auto blockState = _blockLayer.getBlockAtBlockPos(blockPos);
        const auto& config = _assetManager.getBlockConfig(blockState.id.value());
 
        projectilePass = false;
        if (auto* origin = config.getOrigin(); origin && origin->HasMember("collision") && (*origin)["collision"].IsObject()) {
            const auto& collisionObj = (*origin)["collision"];
            if (collisionObj.HasMember("projectilePass") && collisionObj["projectilePass"].IsBool()) {
                projectilePass = collisionObj["projectilePass"].GetBool();
            }
        }
 
        collisionShape = cocos2d::Vec2(BLOCK_SIZE, BLOCK_SIZE);
        collisionOffset = cocos2d::Vec2::ZERO;
        if (auto* origin = config.getOrigin(); origin && origin->HasMember("collision") && (*origin)["collision"].IsObject()) {
            const auto& collisionObj = (*origin)["collision"];
            if (collisionObj.HasMember("shape") && collisionObj["shape"].IsArray() && collisionObj["shape"].Size() >= 2) {
                const auto& shapeArr = collisionObj["shape"];
                if (shapeArr[0].IsNumber() && shapeArr[1].IsNumber()) {
                    collisionShape.x = shapeArr[0].GetFloat() * BLOCK_SIZE;
                    collisionShape.y = shapeArr[1].GetFloat() * BLOCK_SIZE;
                }
            }
            if (collisionObj.HasMember("offset") && collisionObj["offset"].IsArray() && collisionObj["offset"].Size() >= 2) {
                const auto& offsetArr = collisionObj["offset"];
                if (offsetArr[0].IsNumber() && offsetArr[1].IsNumber()) {
                    collisionOffset.x = offsetArr[0].GetFloat() * BLOCK_SIZE;
                    collisionOffset.y = offsetArr[1].GetFloat() * BLOCK_SIZE;
                }
            }
        }
 
        return true;
    };

    std::unordered_map<ColumnKey, std::vector<int>, ColumnKeyHash> columns;
    std::unordered_set<Vec2i, Vec2iHash> singleBlocks;
    std::unordered_map<Vec2i, bool> singleProjectilePass;

    auto view = _registry.view<Position, PhysicsTicket>();
    view.each([&](Position& worldPos, PhysicsTicket& ticket)
        {
            Vec2i blockUpperLeft = getUpperLeft(worldPos, ticket);
            Vec2i blockLowerRight = getLowerRight(worldPos, ticket);

            for (int y = blockUpperLeft.y; y >= blockLowerRight.y; y--)
            {
                for (int x = blockUpperLeft.x; x <= blockLowerRight.x; x++)
                {
                    Vec2i blockPos = Vec2i(x, y);
                    bool projectilePass = false;
                    cocos2d::Vec2 collisionShape;
                    cocos2d::Vec2 collisionOffset;
                    if (!queryCollisionMeta(blockPos, projectilePass, collisionShape, collisionOffset)) {
                        continue;
                    }
 
                    const bool mergeable = (std::fabs(collisionShape.x - BLOCK_SIZE) < 0.01f) &&
                        (std::fabs(collisionShape.y - BLOCK_SIZE) < 0.01f) &&
                        (std::fabs(collisionOffset.x) < 0.01f) &&
                        (std::fabs(collisionOffset.y) < 0.01f);
 
                    if (mergeable) {
                        columns[ColumnKey{ x, projectilePass }].push_back(y);
                    }
                    else {
                        singleBlocks.insert(blockPos);
                        singleProjectilePass[blockPos] = projectilePass;
                    }
                }
            }

#if PHYSICS_TICKET_DEBUG
            drawNode->drawRect(
                (blockUpperLeft + Vec2i(0, 1)) * BLOCK_SIZE,
                (blockLowerRight + Vec2i(1, 0)) * BLOCK_SIZE,
                cocos2d::Color4F::YELLOW);
#endif
        });

    std::unordered_set<BlockPhysicsShapeKey, BlockPhysicsShapeKeyHash> desired;

    auto isMergeableAt = [&](int x, int y, bool projectilePassWanted) -> bool {
        bool projectilePass = false;
        cocos2d::Vec2 collisionShape;
        cocos2d::Vec2 collisionOffset;
        if (!queryCollisionMeta(Vec2i(x, y), projectilePass, collisionShape, collisionOffset)) {
            return false;
        }
        if (projectilePass != projectilePassWanted) {
            return false;
        }
        return (std::fabs(collisionShape.x - BLOCK_SIZE) < 0.01f) &&
            (std::fabs(collisionShape.y - BLOCK_SIZE) < 0.01f) &&
            (std::fabs(collisionOffset.x) < 0.01f) &&
            (std::fabs(collisionOffset.y) < 0.01f);
    };

    for (auto& kv : columns)
    {
        auto& ys = kv.second;
        std::sort(ys.begin(), ys.end());
        ys.erase(std::unique(ys.begin(), ys.end()), ys.end());
        if (ys.empty()) continue;

        struct Interval { int y0 = 0; int y1 = 0; };
        std::vector<Interval> intervals;
        intervals.reserve(8);

        int runStart = ys.front();
        int prev = ys.front();
        for (size_t i = 1; i < ys.size(); ++i)
        {
            const int y = ys[i];
            if (y == prev + 1) {
                prev = y;
                continue;
            }
            intervals.push_back(Interval{ runStart, prev });
            runStart = prev = y;
        }
        intervals.push_back(Interval{ runStart, prev });

        // Expand a bit beyond the ticket range to avoid artificial split faces near the actor.
        // Then merge overlapping expanded intervals to avoid creating internal faces again.
        constexpr int expandBlocks = 1;
        for (auto& it : intervals)
        {
            for (int step = 0; step < expandBlocks; ++step) {
                if (!isMergeableAt(kv.first.x, it.y0 - 1, kv.first.projectilePass)) break;
                --it.y0;
            }
            for (int step = 0; step < expandBlocks; ++step) {
                if (!isMergeableAt(kv.first.x, it.y1 + 1, kv.first.projectilePass)) break;
                ++it.y1;
            }
        }

        std::sort(intervals.begin(), intervals.end(), [](const Interval& a, const Interval& b) {
            if (a.y0 != b.y0) return a.y0 < b.y0;
            return a.y1 < b.y1;
        });

        std::vector<Interval> merged;
        merged.reserve(intervals.size());
        for (const auto& it : intervals)
        {
            if (merged.empty() || it.y0 > merged.back().y1 + 1) {
                merged.push_back(it);
            } else {
                merged.back().y1 = std::max(merged.back().y1, it.y1);
            }
        }

        for (const auto& it : merged)
        {
            desired.insert(BlockPhysicsShapeKey{ BlockPhysicsShapeKey::Kind::VerticalRun, kv.first.x, it.y0, it.y1, kv.first.projectilePass });
        }
    }

    for (const auto& pos : singleBlocks)
    {
        const bool projectilePass = singleProjectilePass[pos];

        desired.insert(BlockPhysicsShapeKey{ BlockPhysicsShapeKey::Kind::Tile, pos.x, pos.y, pos.y, projectilePass });
    }

    return desired;
}

void BlockPhysicsSystem::addAll(const std::vector<BlockPhysicsShapeKey>& allAdded)
{
    for (const auto& key : allAdded)
    {
        if (_physicsLayer.hasPhysicsShapeTag(key)) {
            continue;
        }
 
        cocos2d::PhysicsShapeBox* shape = nullptr;
        if (key.kind == BlockPhysicsShapeKey::Kind::Tile) {
            shape = createBoxAtBlockPos(Vec2i(key.x, key.y0));
        }
        else {
            shape = createVerticalRunBox(key.x, key.y0, key.y1, key.projectilePass);
        }
 
        if (!shape) {
            continue;
        }
 
        const int tag = _physicsLayer.addPhysicsShapeTag(key);
        shape->setTag(tag);
        _body->addShape(shape);
    }
}

void BlockPhysicsSystem::removeAll(const std::vector<BlockPhysicsShapeKey>& allRemoved)
{
    for (const auto& key : allRemoved)
    {
        if (!_physicsLayer.hasPhysicsShapeTag(key)) {
            continue;
        }
        const int tag = _physicsLayer.getPhysicsBodyTag(key);
        _body->removeShape(tag);
        _physicsLayer.removePhysicsShapeTag(key);
    }
}

bool BlockPhysicsSystem::isInside(const Vec2i& blockPos, const Vec2i& blockUpperLeft, const Vec2i& blockLowerRight)
{
    return blockPos.x >= blockUpperLeft.x &&
        blockPos.x <= blockLowerRight.x &&
        blockPos.y <= blockUpperLeft.y &&
        blockPos.y >= blockLowerRight.y;
}

bool BlockPhysicsSystem::hasCollision(const Vec2i& blockPos)
{
    auto blockState = _blockLayer.getBlockAtBlockPos(blockPos);
    if (!blockState.id.has_value()) return false;
    const auto& config = _assetManager.getBlockConfig(blockState.id.value());

    if (auto* origin = config.getOrigin()) {
        if (origin->HasMember("collision") && (*origin)["collision"].IsObject()) {
            const auto& collisionObj = (*origin)["collision"];
            if (collisionObj.HasMember("enable") && collisionObj["enable"].IsBool()) {
                return collisionObj["enable"].GetBool();
            }
        }
    }

    const auto& raw = config.getConfig();
    if (raw.HasMember("collision") && raw["collision"].IsBool()) {
        return raw["collision"].GetBool();
    }

    return false;
}

Vec2i BlockPhysicsSystem::getUpperLeft(const Position& worldPos, const PhysicsTicket& ticket)
{
    Vec2i upperleft =
        worldPos.getPostion() + ticket.offset + Vec2i(-ticket.size.x, ticket.size.y) / 2;
    return _blockLayer.worldPosToBlockPos(upperleft);
}

Vec2i BlockPhysicsSystem::getLowerRight(const Position& worldPos, const PhysicsTicket& ticket)
{
    Vec2i lowerright =
        worldPos.getPostion() + ticket.offset + Vec2i(ticket.size.x, -ticket.size.y) / 2;
    return _blockLayer.worldPosToBlockPos(lowerright);
}

cocos2d::PhysicsShapeBox* BlockPhysicsSystem::createBoxAtBlockPos(const Vec2i& blockPos)
{
    if (!hasCollision(blockPos)) {
        return nullptr;
    }

    auto blockState = _blockLayer.getBlockAtBlockPos(blockPos);

    constexpr int TERRAIN_CATEGORY = 0x0001;
    constexpr int PROJECTILE_CATEGORY = 0x0004;

    const auto& config = _assetManager.getBlockConfig(blockState.id.value());
    bool projectilePass = false;
    if (auto* origin = config.getOrigin(); origin && origin->HasMember("collision") && (*origin)["collision"].IsObject()) {
        const auto& collisionObj = (*origin)["collision"];
        if (collisionObj.HasMember("projectilePass") && collisionObj["projectilePass"].IsBool()) {
            projectilePass = collisionObj["projectilePass"].GetBool();
        }
    }

    cocos2d::Vec2 collisionShape = cocos2d::Vec2(BLOCK_SIZE, BLOCK_SIZE);
    cocos2d::Vec2 collisionOffset = cocos2d::Vec2::ZERO;
    if (auto* origin = config.getOrigin(); origin && origin->HasMember("collision") && (*origin)["collision"].IsObject()) {
        const auto& collisionObj = (*origin)["collision"];
        if (collisionObj.HasMember("shape") && collisionObj["shape"].IsArray() && collisionObj["shape"].Size() >= 2) {
            const auto& shapeArr = collisionObj["shape"];
            if (shapeArr[0].IsNumber() && shapeArr[1].IsNumber()) {
                collisionShape.x = shapeArr[0].GetFloat() * BLOCK_SIZE;
                collisionShape.y = shapeArr[1].GetFloat() * BLOCK_SIZE;
            }
        }
        if (collisionObj.HasMember("offset") && collisionObj["offset"].IsArray() && collisionObj["offset"].Size() >= 2) {
            const auto& offsetArr = collisionObj["offset"];
            if (offsetArr[0].IsNumber() && offsetArr[1].IsNumber()) {
                collisionOffset.x = offsetArr[0].GetFloat() * BLOCK_SIZE;
                collisionOffset.y = offsetArr[1].GetFloat() * BLOCK_SIZE;
            }
        }
    }

    auto box = cocos2d::PhysicsShapeBox::create(
        { collisionShape.x, collisionShape.y },
        { 0.1f, 0.0f, 1.0f },
        cocos2d::Vec2(blockPos.x * BLOCK_SIZE + collisionOffset.x, blockPos.y * BLOCK_SIZE + collisionOffset.y));

#if PHYSICS_LOG
    CCLOG("create box at %d %d", blockPos.x, blockPos.y);
#endif
    box->setCategoryBitmask(TERRAIN_CATEGORY);
    box->setContactTestBitmask(0xFFFFFFFF);
    if (projectilePass) {
        box->setCollisionBitmask(0xFFFFFFFF & ~PROJECTILE_CATEGORY);
    } else {
        box->setCollisionBitmask(0xFFFFFFFF);
    }
    return box;
}

cocos2d::PhysicsShapeBox* BlockPhysicsSystem::createVerticalRunBox(int x, int y0, int y1, bool projectilePass)
{
    constexpr int TERRAIN_CATEGORY = 0x0001;
    constexpr int PROJECTILE_CATEGORY = 0x0004;

    const int minY = std::min(y0, y1);
    const int maxY = std::max(y0, y1);
    const int len = (maxY - minY + 1);
    const float height = static_cast<float>(len) * BLOCK_SIZE;
    const float centerY = (static_cast<float>(minY + maxY) * 0.5f) * BLOCK_SIZE;

    auto box = cocos2d::PhysicsShapeBox::create(
        { static_cast<float>(BLOCK_SIZE), height },
        { 0.1f, 0.0f, 1.0f },
        cocos2d::Vec2(static_cast<float>(x) * BLOCK_SIZE, centerY));

    box->setCategoryBitmask(TERRAIN_CATEGORY);
    box->setContactTestBitmask(0xFFFFFFFF);
    if (projectilePass) {
        box->setCollisionBitmask(0xFFFFFFFF & ~PROJECTILE_CATEGORY);
    } else {
        box->setCollisionBitmask(0xFFFFFFFF);
    }
    return box;
}