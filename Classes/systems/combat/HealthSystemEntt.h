#ifndef __ECS_SYSTEM_HEALTHSYSTEMENTT_H__
#define __ECS_SYSTEM_HEALTHSYSTEMENTT_H__

#include "systems/core/ISystemEntt.h"
#include "systems/core/SystemPriority.h"
#include "components/AllComponents.h"

namespace ecs {

/**
 * @brief 生命值系统 - 处理无敌时间和死亡
 */
class HealthSystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "HealthSystem"; }
    int getPriority() const override { return SystemPriority::COLLISION + 50; }

    void update(float delta) override {
        auto view = _registry->view<HealthComponent>();
        
        view.each([delta](auto entity, HealthComponent& health) {
            // 更新无敌时间
            if (health.invincibleTimer > 0) {
                health.invincibleTimer -= delta;
                if (health.invincibleTimer < 0.0f) {
                    health.invincibleTimer = 0.0f;
                }
            }
        });
    }
};

} // namespace ecs

#endif // __ECS_SYSTEM_HEALTHSYSTEMENTT_H__
