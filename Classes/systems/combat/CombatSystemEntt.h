#ifndef __ECS_SYSTEM_COMBATSYSTEMENTT_H__
#define __ECS_SYSTEM_COMBATSYSTEMENTT_H__

#include "systems/core/ISystemEntt.h"
#include "systems/core/SystemPriority.h"
#include "components/AllComponents.h"

namespace ecs {

/**
 * @brief 战斗系统 - 处理攻击冷却
 */
class CombatSystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "CombatSystem"; }
    int getPriority() const override { return SystemPriority::COLLISION + 100; }

    void update(float delta) override {
        auto view = _registry->view<CombatComponent>();
        
        view.each([delta](auto entity, CombatComponent& combat) {
            if (combat.attackTimer > 0) {
                combat.attackTimer -= delta;
                if (combat.attackTimer < 0.0f) {
                    combat.attackTimer = 0.0f;
                }
            }
        });
    }
};

} // namespace ecs

#endif // __ECS_SYSTEM_COMBATSYSTEMENTT_H__
