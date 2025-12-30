#pragma once

#include "components/AllComponents.h"
#include "core/assets_manager.h"
#include "core/block_world.h"
#include "core/consts.h"
#include "core/factory/monster/MonsterMasterFactory.h"
#include "components/block/block_component.h"
#include "systems/block_layer/block_layer.h"
#include "systems/core/ISystemEntt.h"
#include "systems/core/SystemPriority.h"

#include <algorithm>
#include <cmath>
#include <random>
#include <string>
#include <vector>

namespace ecs {

class MonsterSpawnSystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "MonsterSpawnSystem"; }
    int getPriority() const override { return SystemPriority::AI - 20; }

    void setRelaxedSpawnSearchEnabled(bool enabled) { _relaxedSpawnSearchEnabled = enabled; }
    void setRelaxedSpawnSearchRangeTiles(int halfWidthTiles, int halfHeightTiles) {
        _relaxedSpawnSearchHalfWidthTiles = std::max(0, halfWidthTiles);
        _relaxedSpawnSearchHalfHeightTiles = std::max(0, halfHeightTiles);
    }
    void setRelaxedSpawnMarginTiles(float marginTiles) { _relaxedSpawnMarginTiles = std::max(0.0f, marginTiles); }
    void setRelaxedTreatMissingChunksAsClear(bool enabled) { _relaxedTreatMissingChunksAsClear = enabled; }

    void update(float delta) override {
        if (!_registry) {
            return;
        }

        entt::entity player = entt::null;
        TransformComponent* playerTransform = nullptr;
        {
            auto playerView = _registry->view<PlayerTag, TransformComponent>();
            for (auto entity : playerView) {
                player = entity;
                playerTransform = &playerView.get<TransformComponent>(entity);
                break;
            }
        }

        if (player == entt::null || !playerTransform) {
            return;
        }

        const cocos2d::Vec2 playerPos = playerTransform->position;
        const Vec2i playerBlockPos = BlockLayer::worldPosToBlockPos(playerPos);

        int activeMobCount = 0;
        std::vector<entt::entity> toDestroy;
        {
            auto mobView = _registry->view<PhysicsBodyComponent, TransformComponent>();
            mobView.each([&](entt::entity entity, const PhysicsBodyComponent& physics, const TransformComponent& transform) {
                if (entity == player) {
                    return;
                }
                if (auto* pooled = _registry->try_get<PooledEntity>(entity)) {
                    if (!pooled->inUse) {
                        return;
                    }
                }
                if (_registry->any_of<PlayerTag>(entity)) {
                    return;
                }
                if (_registry->any_of<ProjectileComponent>(entity)) {
                    return;
                }

                constexpr int NPC_CATEGORY = 0x0002;
                if ((physics.categoryBitmask & NPC_CATEGORY) == 0) {
                    return;
                }

                const float dx = std::fabs(transform.position.x - playerPos.x);
                const float dy = std::fabs(transform.position.y - playerPos.y);

                if (dx > kDespawnHalfWidthTiles * BLOCK_SIZE ||
                    dy > kDespawnHalfHeightTiles * BLOCK_SIZE) {
                    toDestroy.push_back(entity);
                    return;
                }

                activeMobCount++;
            });
        }

        for (auto entity : toDestroy) {
            if (_registry->valid(entity)) {
                _registry->destroy(entity);
            }
        }

        if (activeMobCount >= kMaxMobs) {
            return;
        }

        if (!rollSpawnAttempt(delta)) {
            return;
        }

        ensureMonsterIdCache();
        if (_monsterIds.empty()) {
            return;
        }

        for (int attempt = 0; attempt < kSpawnPointAttempts; ++attempt) {
            const std::string& monsterId = pickRandomMonsterId();
            if (monsterId.empty()) {
                continue;
            }

            const auto offsetTiles = randomOffsetInSpawnRing();
            const Vec2i baseBlockPos(playerBlockPos.x + offsetTiles.x, playerBlockPos.y + offsetTiles.y);
            if (!isChunkLoadedAtBlockPos(baseBlockPos)) {
                continue;
            }

            cocos2d::Vec2 spawnPos;
            if (!findSpawnPosInSearchArea(baseBlockPos, playerBlockPos, monsterId, spawnPos)) {
                continue;
            }

            auto& worldScene = _registry->ctx().get<WorldScene>();
            cocos2d::Node* parentNode = worldScene.operator->();

            MonsterMasterFactory::getInstance().createMonster(*_registry, monsterId, spawnPos.x, spawnPos.y, parentNode);
            break;
        }
    }

private:
    struct OffsetTiles {
        int x = 0;
        int y = 0;
    };

    struct SpawnBox;

    static constexpr int kMaxMobs = 8;
    static constexpr int kSpawnRate = 600;

    static constexpr int kInnerX = 62;
    static constexpr int kOuterX = 84;
    static constexpr int kInnerY = 35;
    static constexpr int kOuterY = 47;

    static constexpr float kDespawnHalfWidthTiles = 252.0f;
    static constexpr float kDespawnHalfHeightTiles = 146.75f;

    static constexpr int kSpawnPointAttempts = 20;
    static constexpr float kSpawnMarginTiles = 0.5f;
    static constexpr int kGroundSearchDepthTiles = 64;
    static constexpr float kGroundEpsilon = 0.1f;

    static constexpr int kFootprintSurfaceSearchUpTiles = 8;
    static constexpr int kFootprintSurfaceSearchDownTiles = 24;
    static constexpr int kMaxSpawnLiftTiles = 12;

    bool _relaxedSpawnSearchEnabled = true;
    int _relaxedSpawnSearchHalfWidthTiles = 10;
    int _relaxedSpawnSearchHalfHeightTiles = 6;
    float _relaxedSpawnMarginTiles = 0.0f;
    bool _relaxedTreatMissingChunksAsClear = false;

    std::mt19937 _rng{ std::random_device{}() };
    std::uniform_real_distribution<float> _unitDist{ 0.0f, 1.0f };
    std::vector<std::string> _monsterIds;
    size_t _monsterPickIndex = 0;

    bool rollSpawnAttempt(float delta) {
        if (kSpawnRate <= 0) {
            return false;
        }

        const float frames = std::max(0.0f, delta) * 60.0f;
        if (frames <= 0.0f) {
            return false;
        }

        const float perFrame = 1.0f / static_cast<float>(kSpawnRate);
        const float base = std::clamp(1.0f - perFrame, 0.0f, 1.0f);
        const float p = 1.0f - std::pow(base, frames);
        return _unitDist(_rng) < p;
    }

    void ensureMonsterIdCache() {
        if (!_monsterIds.empty()) {
            return;
        }

        auto ids = MonsterMasterFactory::getInstance().getAllSupportedMonsterIds();
        ids.erase(std::remove_if(ids.begin(), ids.end(), [](const std::string& id) {
            if (id.find("KingSlime") != std::string::npos) {
                return true;
            }
            return false;
        }), ids.end());

        _monsterIds = std::move(ids);
        std::shuffle(_monsterIds.begin(), _monsterIds.end(), _rng);
        _monsterPickIndex = 0;
    }

    const std::string& pickRandomMonsterId() {
        static const std::string empty;
        if (_monsterIds.empty()) {
            return empty;
        }

        if (_monsterPickIndex >= _monsterIds.size()) {
            std::shuffle(_monsterIds.begin(), _monsterIds.end(), _rng);
            _monsterPickIndex = 0;
        }

        return _monsterIds[_monsterPickIndex++];
    }

    OffsetTiles randomOffsetInSpawnRing() {
        std::uniform_int_distribution<int> distX(-kOuterX, kOuterX);
        std::uniform_int_distribution<int> distY(-kOuterY, kOuterY);

        for (int i = 0; i < 64; ++i) {
            const int x = distX(_rng);
            const int y = distY(_rng);
            if (std::abs(x) < kInnerX && std::abs(y) < kInnerY) {
                continue;
            }
            return OffsetTiles{ x, y };
        }

        return OffsetTiles{ kOuterX, 0 };
    }

    bool isChunkLoadedAt(const cocos2d::Vec2& worldPos) {
        auto& bw = _registry->ctx().get<BlockWorld>();
        auto& blockLayer = bw.getLayer(LayerType::BLOCK);
        return blockLayer.hasChunkExistAtWorldPos(worldPos);
    }

    bool isChunkLoadedAtBlockPos(const Vec2i& blockPos) {
        auto& bw = _registry->ctx().get<BlockWorld>();
        auto& blockLayer = bw.getLayer(LayerType::BLOCK);
        return blockLayer.hasChunkExistAtBlockPos(blockPos);
    }

    bool isValidSupportBlock(const Vec2i& blockPos) {
        auto& bw = _registry->ctx().get<BlockWorld>();
        auto& blockLayer = bw.getLayer(LayerType::BLOCK);

        if (!blockLayer.hasChunkExistAtBlockPos(blockPos)) {
            return false;
        }
        if (!hasSolidCollisionAt(blockPos)) {
            return false;
        }
        return true;
    }

    bool findSurfaceBlockNearY(int x, int refY, int searchUpTiles, int searchDownTiles, Vec2i& outSurfaceBlock) {
        auto& bw = _registry->ctx().get<BlockWorld>();
        auto& blockLayer = bw.getLayer(LayerType::BLOCK);

        const int topY = refY + std::max(0, searchUpTiles);
        const int bottomY = refY - std::max(0, searchDownTiles);

        for (int y = topY; y >= bottomY; --y) {
            const Vec2i ground(x, y);
            if (!blockLayer.hasChunkExistAtBlockPos(ground)) {
                continue;
            }
            if (!isValidSupportBlock(ground)) {
                continue;
            }
            const Vec2i above(x, y + 1);
            if (!blockLayer.hasChunkExistAtBlockPos(above)) {
                continue;
            }
            if (hasSolidCollisionAt(above)) {
                continue;
            }
            outSurfaceBlock = ground;
            return true;
        }

        return false;
    }

    bool adjustSupportForFootprintIfNeeded(const Vec2i& baseSupport,
                                          const SpawnBox& physBox,
                                          Vec2i& outAdjustedSupport);

    bool findGroundBlockBelow(const Vec2i& startBlockPos, Vec2i& outGroundBlock) {
        auto& bw = _registry->ctx().get<BlockWorld>();
        auto& blockLayer = bw.getLayer(LayerType::BLOCK);

        for (int i = 0; i < kGroundSearchDepthTiles; ++i) {
            const Vec2i ground(startBlockPos.x, startBlockPos.y - i);
            if (!blockLayer.hasChunkExistAtBlockPos(ground)) {
                continue;
            }

            if (!isValidSupportBlock(ground)) {
                continue;
            }

            const Vec2i above(ground.x, ground.y + 1);
            if (!blockLayer.hasChunkExistAtBlockPos(above)) {
                continue;
            }
            if (hasSolidCollisionAt(above)) {
                continue;
            }

            outGroundBlock = ground;
            return true;
        }

        return false;
    }

    bool findGroundedSpawnPos(const Vec2i& baseBlockPos,
                              const Vec2i& playerBlockPos,
                              const std::string& monsterId,
                              cocos2d::Vec2& outSpawnPos) {
        const MonsterConfig* cfg = MonsterMasterFactory::getInstance().getConfig(monsterId);
        if (!cfg) {
            return false;
        }

        Vec2i groundBlock;
        if (!findGroundBlockBelow(baseBlockPos, groundBlock)) {
            return false;
        }

        const SpawnBox physBox = computeSpawnBox(*cfg);

        Vec2i adjustedSupport = groundBlock;
        if (!adjustSupportForFootprintIfNeeded(groundBlock, physBox, adjustedSupport)) {
            return false;
        }

        const float groundTopY = static_cast<float>(adjustedSupport.y + 1) * BLOCK_SIZE;
        const float spawnX = (static_cast<float>(groundBlock.x) + 0.5f) * BLOCK_SIZE;
        const float spawnY = groundTopY + physBox.size.y * 0.5f - physBox.offset.y + kGroundEpsilon;

        outSpawnPos = cocos2d::Vec2(spawnX, spawnY);

        const Vec2i spawnBlockPos = BlockLayer::worldPosToBlockPos(outSpawnPos);
        const Vec2i spawnOffset(spawnBlockPos.x - playerBlockPos.x, spawnBlockPos.y - playerBlockPos.y);
        if (std::abs(spawnOffset.x) > kOuterX || std::abs(spawnOffset.y) > kOuterY) {
            return false;
        }
        if (std::abs(spawnOffset.x) < kInnerX && std::abs(spawnOffset.y) < kInnerY) {
            return false;
        }

        return isChunkLoadedAt(outSpawnPos);
    }

    bool findSpawnPosInSearchArea(const Vec2i& baseBlockPos,
                                  const Vec2i& playerBlockPos,
                                  const std::string& monsterId,
                                  cocos2d::Vec2& outSpawnPos) {
        if (!_relaxedSpawnSearchEnabled) {
            if (!findGroundedSpawnPos(baseBlockPos, playerBlockPos, monsterId, outSpawnPos)) {
                return false;
            }
            return isAreaClearForMonster(outSpawnPos, monsterId);
        }

        std::vector<int> xOffsets;
        xOffsets.reserve(static_cast<size_t>(_relaxedSpawnSearchHalfWidthTiles) * 2 + 1);
        xOffsets.push_back(0);
        for (int i = 1; i <= _relaxedSpawnSearchHalfWidthTiles; ++i) {
            xOffsets.push_back(i);
            xOffsets.push_back(-i);
        }

        std::vector<int> yOffsets;
        yOffsets.reserve(static_cast<size_t>(_relaxedSpawnSearchHalfHeightTiles) * 2 + 1);
        yOffsets.push_back(0);
        for (int i = 1; i <= _relaxedSpawnSearchHalfHeightTiles; ++i) {
            yOffsets.push_back(i);
            yOffsets.push_back(-i);
        }

        for (int dy : yOffsets) {
            for (int dx : xOffsets) {
                const Vec2i candidateBase(baseBlockPos.x + dx, baseBlockPos.y + dy);
                if (!isChunkLoadedAtBlockPos(candidateBase)) {
                    continue;
                }

                cocos2d::Vec2 candidateSpawn;
                if (!findGroundedSpawnPos(candidateBase, playerBlockPos, monsterId, candidateSpawn)) {
                    continue;
                }
                if (!isAreaClearForMonster(candidateSpawn, monsterId)) {
                    continue;
                }
                outSpawnPos = candidateSpawn;
                return true;
            }
        }

        return false;
    }

    bool getCollisionEnabled(const BlockConfig& config) {
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

    bool hasSolidCollisionAt(const Vec2i& blockPos) {
        auto& bw = _registry->ctx().get<BlockWorld>();
        auto& am = _registry->ctx().get<AssetManager>();
        auto& layer = bw.getLayer(LayerType::BLOCK);

        if (!layer.hasChunkExistAtBlockPos(blockPos)) {
            return false;
        }

        auto handle = layer.getBlockAtBlockPos(blockPos);
        if (!handle.id.has_value()) {
            return false;
        }

        const auto& cfg = am.getBlockConfig(handle.id.value());
        return getCollisionEnabled(cfg);
    }

    struct SpawnBox {
        cocos2d::Vec2 size = cocos2d::Vec2::ZERO;
        cocos2d::Vec2 offset = cocos2d::Vec2::ZERO;
    };

    SpawnBox computeSpawnBox(const MonsterConfig& config) {
        SpawnBox box;

        const float scaledWidth = config.physics.bodyWidth * config.display.scale;
        const float scaledHeight = config.physics.bodyHeight * config.display.scale;
        const float scaledHeightOffset = config.physics.bodyHeightOffset * config.display.scale;

        if (config.type == "DemonEye") {
            const float radius = std::min(scaledWidth, scaledHeight) / 2.0f;
            box.size = cocos2d::Vec2(radius * 2.0f, radius * 2.0f);
            box.offset = cocos2d::Vec2(scaledHeightOffset, 0.0f);
        } else if (config.type == "Antlion") {
            const float radius = std::min(scaledWidth, scaledHeight) / 2.0f;
            box.size = cocos2d::Vec2(radius * 2.0f, radius * 2.0f);
            box.offset = cocos2d::Vec2::ZERO;
        } else if (config.type == "EaterOfSouls") {
            box.size = cocos2d::Vec2(scaledWidth, scaledHeight);
            box.offset = cocos2d::Vec2(0.0f, scaledHeightOffset);
        } else if (config.type == "Vulture") {
            box.size = cocos2d::Vec2(scaledWidth, scaledHeight);
            box.offset = cocos2d::Vec2::ZERO;
        } else {
            float width = scaledWidth;
            if (config.type == "Slime") {
                float w = scaledWidth - 2.0f * config.display.scale;
                if (w < 1.0f) {
                    w = 1.0f;
                }
                width = w;
            }
            box.size = cocos2d::Vec2(width, scaledHeight);
            box.offset = cocos2d::Vec2(0.0f, scaledHeightOffset);
        }

        if (box.size.x < 1.0f) box.size.x = 1.0f;
        if (box.size.y < 1.0f) box.size.y = 1.0f;
        return box;
    }

    bool isAreaClearForMonster(const cocos2d::Vec2& spawnPos, const std::string& monsterId) {
        const MonsterConfig* cfg = MonsterMasterFactory::getInstance().getConfig(monsterId);
        if (!cfg) {
            return false;
        }

        SpawnBox box = computeSpawnBox(*cfg);
        const float marginTiles = _relaxedSpawnSearchEnabled ? _relaxedSpawnMarginTiles : kSpawnMarginTiles;
        const float margin = marginTiles * BLOCK_SIZE;
        const float halfW = box.size.x * 0.5f + margin;
        const float halfHUp = box.size.y * 0.5f + margin;
        const float halfHDown = box.size.y * 0.5f;

        const cocos2d::Vec2 center = spawnPos + box.offset;
        const cocos2d::Vec2 upperLeftWorld = center + cocos2d::Vec2(-halfW, halfHUp);
        const cocos2d::Vec2 lowerRightWorld = center + cocos2d::Vec2(halfW, -halfHDown);

        const Vec2i upperLeftBlock = BlockLayer::worldPosToBlockPos(upperLeftWorld);
        const Vec2i lowerRightBlock = BlockLayer::worldPosToBlockPos(lowerRightWorld);

        auto& bw = _registry->ctx().get<BlockWorld>();
        auto& blockLayer = bw.getLayer(LayerType::BLOCK);

        for (int y = upperLeftBlock.y; y >= lowerRightBlock.y; --y) {
            for (int x = upperLeftBlock.x; x <= lowerRightBlock.x; ++x) {
                const Vec2i bp(x, y);
                if (!blockLayer.hasChunkExistAtBlockPos(bp)) {
                    if (_relaxedSpawnSearchEnabled && _relaxedTreatMissingChunksAsClear) {
                        continue;
                    }
                    return false;
                }
                if (hasSolidCollisionAt(bp)) {
                    return false;
                }
            }
        }

        return true;
    }
};

inline bool MonsterSpawnSystemEntt::adjustSupportForFootprintIfNeeded(const Vec2i& baseSupport,
                                                                      const SpawnBox& physBox,
                                                                      Vec2i& outAdjustedSupport) {
    outAdjustedSupport = baseSupport;
    if (!_relaxedSpawnSearchEnabled) {
        return true;
    }

    const int halfFootprintTiles = std::max(0, static_cast<int>(std::ceil((physBox.size.x * 0.5f) / static_cast<float>(BLOCK_SIZE))));
    int maxSurfaceY = baseSupport.y;
    bool foundAny = false;

    for (int dx = -halfFootprintTiles; dx <= halfFootprintTiles; ++dx) {
        Vec2i surface;
        if (!findSurfaceBlockNearY(baseSupport.x + dx,
                                   baseSupport.y,
                                   kFootprintSurfaceSearchUpTiles,
                                   kFootprintSurfaceSearchDownTiles,
                                   surface)) {
            continue;
        }
        foundAny = true;
        if (surface.y > maxSurfaceY) {
            maxSurfaceY = surface.y;
        }
    }

    if (!foundAny) {
        return false;
    }

    if (maxSurfaceY - baseSupport.y > kMaxSpawnLiftTiles) {
        return false;
    }

    outAdjustedSupport.y = maxSurfaceY;
    return true;
}

} // namespace ecs
