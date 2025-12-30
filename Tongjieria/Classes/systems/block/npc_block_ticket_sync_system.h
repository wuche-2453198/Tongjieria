#pragma once

#include "block_system_manager.h"
#include "components/AllComponents.h"
#include "components/block/block_component.h"
#include "core/assets_manager.h"

#include <algorithm>
#include <cmath>

class NpcBlockTicketSyncSystem : public ISystem
{
public:
    NpcBlockTicketSyncSystem(entt::registry& registry, entt::dispatcher& dispatcher)
        : ISystem(registry, dispatcher) {}

    void update(float delta) override
    {
        auto& assetManager = _registry.ctx().get<AssetManager>();
        auto& blockLayer = _blockWorld.getLayer(LayerType::BLOCK);

        auto hasSolidCollisionAt = [&](const Vec2i& blockPos) -> bool {
            auto blockState = blockLayer.getBlockAtBlockPos(blockPos);
            if (!blockState.id.has_value()) {
                return false;
            }

            const auto& config = assetManager.getBlockConfig(blockState.id.value());

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
        };

        auto isEmbeddedInSolid = [&](const cocos2d::Vec2& worldPos, const ecs::PhysicsBodyComponent& physics) -> bool {
            const cocos2d::Vec2 center = worldPos + physics.offset;
            const Vec2i centerB = BlockLayer::worldPosToBlockPos(center);
            return hasSolidCollisionAt(centerB);
        };

        auto view = _registry.view<ecs::TransformComponent, ecs::PhysicsBodyComponent>();
        view.each([this, delta, &isEmbeddedInSolid](entt::entity entity, const ecs::TransformComponent& transform, const ecs::PhysicsBodyComponent& physics) {
            if (auto* pooled = _registry.try_get<ecs::PooledEntity>(entity)) {
                if (!pooled->inUse) {
                    if (_registry.all_of<Position>(entity)) {
                        _registry.remove<Position>(entity);
                    }
                    if (_registry.all_of<PhysicsTicket>(entity)) {
                        _registry.remove<PhysicsTicket>(entity);
                    }
                    if (_registry.all_of<LoadingTicket>(entity)) {
                        _registry.remove<LoadingTicket>(entity);
                    }
                    return;
                }
            }

            constexpr int NPC_CATEGORY = 0x0002;
            const bool isProjectile = _registry.all_of<ecs::ProjectileComponent>(entity);
            const bool isPlayer = _registry.all_of<ecs::PlayerTag>(entity);
            const bool isNpcByMask = ((physics.categoryBitmask & NPC_CATEGORY) != 0);
            const bool isNpcByComponents = _registry.any_of<ecs::AggroComponent, ecs::JumpMovementComponent, ecs::GroundDetectorComponent>(entity);
            const bool isActor = isPlayer || isNpcByMask || isNpcByComponents;

            if (!isActor && !isProjectile) {
                return;
            }

            cocos2d::Vec2 ticketSize = cocos2d::Vec2::ZERO;
            if (physics.shape == ecs::PhysicsBodyComponent::BodyShape::Circle) {
                float d = std::max(0.0f, physics.radius * 2.0f);
                ticketSize = cocos2d::Vec2(d, d);
            } else {
                ticketSize = cocos2d::Vec2(std::max(0.0f, physics.width), std::max(0.0f, physics.height));
            }

            const float padding = 32.0f;
            ticketSize.x = std::max(ticketSize.x + padding, 16.0f);
            ticketSize.y = std::max(ticketSize.y + padding, 16.0f);

            cocos2d::Vec2 worldPos = transform.position;
            cocos2d::Vec2 velocity = transform.velocity;
            if (auto* state = _registry.try_get<ecs::SpriteStateComponent>(entity)) {
                if (state->spriteCreated && state->spriteHandle) {
                    auto* sprite = static_cast<cocos2d::Sprite*>(state->spriteHandle);
                    if (sprite) {
                        if (auto* body = sprite->getPhysicsBody()) {
                            worldPos = sprite->getPosition();
                            velocity = body->getVelocity();
                        }
                    }
                }
            }

            {
                cocos2d::Vec2 resolvedPos = worldPos;
                constexpr int maxLiftSteps = 256;
                int steps = 0;
                while (steps < maxLiftSteps && isEmbeddedInSolid(resolvedPos, physics)) {
                    resolvedPos.y += static_cast<float>(BLOCK_SIZE);
                    ++steps;
                }

                if (steps > 0) {
                    worldPos = resolvedPos;
                    velocity = cocos2d::Vec2::ZERO;

                    if (auto* state = _registry.try_get<ecs::SpriteStateComponent>(entity)) {
                        if (state->spriteCreated && state->spriteHandle) {
                            auto* sprite = static_cast<cocos2d::Sprite*>(state->spriteHandle);
                            if (sprite) {
                                sprite->setPosition(worldPos);
                                if (auto* body = sprite->getPhysicsBody()) {
                                    body->setVelocity(cocos2d::Vec2::ZERO);
                                }
                            }
                        }
                    }

                    if (_registry.all_of<ecs::TransformComponent>(entity)) {
                        auto& t = _registry.get<ecs::TransformComponent>(entity);
                        t.position = worldPos;
                        t.velocity = cocos2d::Vec2::ZERO;
                    }
                }
            }

            cocos2d::Vec2 deltaMove = velocity * delta;
            const float maxSweep = 512.0f;
            deltaMove.x = std::clamp(deltaMove.x, -maxSweep, maxSweep);
            deltaMove.y = std::clamp(deltaMove.y, -maxSweep, maxSweep);

            cocos2d::Vec2 ticketOffset = physics.offset + deltaMove * 0.5f;
            ticketSize.x += std::fabs(deltaMove.x);
            ticketSize.y += std::fabs(deltaMove.y);

            if (_registry.all_of<Position>(entity)) {
                _registry.get<Position>(entity).setPosition(worldPos);
            } else {
                _registry.emplace<Position>(entity, worldPos);
            }

            if (_registry.all_of<PhysicsTicket>(entity)) {
                auto& ticket = _registry.get<PhysicsTicket>(entity);
                ticket.size = ticketSize;
                ticket.offset = ticketOffset;
            } else {
                _registry.emplace<PhysicsTicket>(entity, ticketSize, ticketOffset);
            }

            if (isActor) {
                constexpr unsigned int radius = 4;
                if (_registry.all_of<LoadingTicket>(entity)) {
                    auto& ticket = _registry.get<LoadingTicket>(entity);
                    ticket.radius = radius;
                    ticket.is_permanent = false;
                    ticket.doration = -1;
                    ticket.entity_id = entity;
                } else {
                    _registry.emplace<LoadingTicket>(entity, entity, radius, false);
                }
            }
        });
    }
};
