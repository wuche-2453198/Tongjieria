#ifndef __ECS_SYSTEM_SLOWFALLSYSTEMENTT_H__
#define __ECS_SYSTEM_SLOWFALLSYSTEMENTT_H__

#include "ISystemEntt.h"
#include "SystemPriority.h"
#include "../AllComponents.h"
#include <cmath>

namespace ecs {

/**
 * @brief 缓降系统 - 处理伞史莱姆等下落时的空气阻力
 * 
 * 当实体下落时，限制其最大下落速度，模拟撑伞的效果
 */
class SlowFallSystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "SlowFallSystem"; }
    int getPriority() const override { return SystemPriority::PHYSICS + 1; }

    void update(float delta) override {
        auto view = _registry->view<SlowFallComponent, SpriteStateComponent>();
        
        view.each([](auto entity, SlowFallComponent& slowFall,
                    SpriteStateComponent& state) {
            if (!slowFall.isActive || !state.spriteCreated || !state.spriteHandle)
                return;
            
            auto* sprite = static_cast<cocos2d::Sprite*>(state.spriteHandle);
            auto* body = sprite->getPhysicsBody();
            if (!body)
                return;
            
            cocos2d::Vec2 velocity = body->getVelocity();
            
            // 只在下落时应用缓降效果（velocity.y < 0 表示向下）
            if (velocity.y < 0) {
                bool modified = false;
                
                // 垂直方向：限制最大下落速度
                float maxFall = -slowFall.maxFallSpeed;
                if (velocity.y < maxFall) {
                    // 应用阻尼，逐渐减速到最大下落速度
                    velocity.y = velocity.y * slowFall.fallDamping;
                    if (velocity.y < maxFall) {
                        velocity.y = maxFall;
                    }
                    modified = true;
                }
                
                // 水平方向：施加空气阻力
                if (std::abs(velocity.x) > 10.0f) {
                    velocity.x = velocity.x * slowFall.horizontalDamping;
                    modified = true;
                }
                
                if (modified) {
                    body->setVelocity(velocity);
                }
            }
        });
    }
};

} // namespace ecs

#endif // __ECS_SYSTEM_SLOWFALLSYSTEMENTT_H__
