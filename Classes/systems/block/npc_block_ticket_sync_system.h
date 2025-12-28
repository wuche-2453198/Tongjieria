#pragma once

#include "block_system_manager.h"
#include "components/AllComponents.h"
#include "components/block/block_component.h"

#include <algorithm>
#include <cmath>

class NpcBlockTicketSyncSystem : public ISystem
{
public:
    NpcBlockTicketSyncSystem(entt::registry& registry, entt::dispatcher& dispatcher)
        : ISystem(registry, dispatcher) {}

    void update(float delta) override
    {
        auto view = _registry.view<ecs::TransformComponent, ecs::PhysicsBodyComponent>();
        view.each([this, delta](entt::entity entity, const ecs::TransformComponent& transform, const ecs::PhysicsBodyComponent& physics) {
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
