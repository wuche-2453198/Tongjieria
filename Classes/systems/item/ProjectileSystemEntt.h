#ifndef __ECS_SYSTEM_PROJECTILESYSTEMENTT_H__
#define __ECS_SYSTEM_PROJECTILESYSTEMENTT_H__

#include "systems/core/ISystemEntt.h"
#include "systems/core/SystemPriority.h"
#include "components/AllComponents.h"
#include "cocos2d.h"
#include <vector>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace ecs {

/**
 * @brief 投射物系统 - 更新投射物位置、旋转和生命周期
 */
class ProjectileSystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "ProjectileSystem"; }
    int getPriority() const override { return SystemPriority::PHYSICS + 5; }

    void update(float delta) override {
        std::vector<entt::entity> toDestroy;
        
        auto view = _registry->view<ProjectileComponent, SpriteStateComponent, RenderComponent, TransformComponent>();
        
        for (auto entity : view) {
            auto& proj = view.get<ProjectileComponent>(entity);
            auto& state = view.get<SpriteStateComponent>(entity);
            auto& render = view.get<RenderComponent>(entity);
            auto& transform = view.get<TransformComponent>(entity);
            
            // 更新生命周期
            proj.lifetime -= delta;
            if (proj.lifetime <= 0 || proj.hasHit) {
                toDestroy.push_back(entity);
                continue;
            }
            
            // 从物理体同步位置和旋转
            if (state.spriteCreated && state.spriteHandle) {
                auto* sprite = static_cast<cocos2d::Sprite*>(state.spriteHandle);
                auto* body = sprite->getPhysicsBody();
                if (body) {
                    transform.position = sprite->getPosition();
                    cocos2d::Vec2 velocity = body->getVelocity();
                    
                    // 根据速度方向更新旋转
                    if (velocity.lengthSquared() > 1.0f) {
                        float rotAngle = atan2(velocity.y, velocity.x) * 180.0f / M_PI;
                        render.rotation = -rotAngle + 90.0f;
                    }
                }
            }
        }
        
        // 销毁过期的投射物
        for (auto entity : toDestroy) {
            _registry->destroy(entity);
        }
    }
};

} // namespace ecs

#endif // __ECS_SYSTEM_PROJECTILESYSTEMENTT_H__
