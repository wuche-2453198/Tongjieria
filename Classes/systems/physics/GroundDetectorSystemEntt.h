#ifndef __ECS_SYSTEM_GROUNDDETECTORSYSTEMENTT_H__
#define __ECS_SYSTEM_GROUNDDETECTORSYSTEMENTT_H__

#include "systems/core/ISystemEntt.h"
#include "systems/core/SystemPriority.h"
#include "components/AllComponents.h"
#include <cmath>

namespace ecs {

/**
 * @brief 地面检测系统 - 根据物理体速度判断地面和静止状态
 * 
 * 动态阻尼：地面时高阻尼快速停止，空中低阻尼保持灵活
 */
class GroundDetectorSystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "GroundDetectorSystem"; }
    int getPriority() const override { return SystemPriority::PHYSICS + 10; }

    // 阻尼配置
    static constexpr float GROUND_DAMPING = 8.0f;  // 地面高阻尼，快速停止
    static constexpr float AIR_DAMPING = 0.3f;     // 空中低阻尼，保持灵活

    void update(float delta) override {
        auto view = _registry->view<GroundDetectorComponent, SpriteStateComponent>();
        
        view.each([this](auto entity, GroundDetectorComponent& ground,
                        SpriteStateComponent& state) {
            if (!state.spriteCreated || !state.spriteHandle)
                return;

            auto* sprite = static_cast<cocos2d::Sprite*>(state.spriteHandle);
            auto* body = sprite->getPhysicsBody();
            if (!body)
                return;

            cocos2d::Vec2 velocity = body->getVelocity();
            // 只检查Y轴速度判断是否在地面（允许在地面上水平移动）
            ground.isOnGround = std::abs(velocity.y) < ground.stillThreshold;
            
            // 动态切换阻尼：地面高阻尼防止滑行，空中低阻尼保持跳跃灵活性
            if (body) {
                if (ground.isOnGround) {
                    body->setLinearDamping(GROUND_DAMPING);
                } else {
                    body->setLinearDamping(AIR_DAMPING);
                }
            }
        });
    }
};

} // namespace ecs

#endif // __ECS_SYSTEM_GROUNDDETECTORSYSTEMENTT_H__
