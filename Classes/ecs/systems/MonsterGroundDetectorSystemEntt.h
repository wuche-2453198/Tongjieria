#ifndef __ECS_SYSTEM_MONSTERGROUNDDETECTORSYSTEMENTT_H__
#define __ECS_SYSTEM_MONSTERGROUNDDETECTORSYSTEMENTT_H__

#include "ISystemEntt.h"
#include "SystemPriority.h"
#include "../AllComponents.h"
#include <cmath>

namespace ecs {

/**
 * @brief 怪物地面检测系统 - 根据物理体速度判断地面状态
 */
class MonsterGroundDetectorSystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "MonsterGroundDetectorSystem"; }
    int getPriority() const override { return SystemPriority::PHYSICS + 10; }

    void update(float delta) override {
        auto view = _registry->view<GroundDetectorComponent, SpriteStateComponent>();
        
        view.each([](auto entity, GroundDetectorComponent& ground,
                    SpriteStateComponent& state) {
            if (!state.spriteCreated || !state.spriteHandle) return;
            auto* sprite = static_cast<cocos2d::Sprite*>(state.spriteHandle);
            auto* body = sprite->getPhysicsBody();
            if (!body) return;
            cocos2d::Vec2 velocity = body->getVelocity();

            // 判断是否静止
            ground.isStill = std::abs(velocity.x) < ground.stillThreshold &&
                           std::abs(velocity.y) < ground.stillThreshold;
        });
    }
};

} // namespace ecs

#endif // __ECS_SYSTEM_MONSTERGROUNDDETECTORSYSTEMENTT_H__
