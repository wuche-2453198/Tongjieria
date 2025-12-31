#ifndef __ECS_SYSTEM_DEBUFFSYSTEMENTT_H__
#define __ECS_SYSTEM_DEBUFFSYSTEMENTT_H__

#include "systems/core/ISystemEntt.h"
#include "systems/core/SystemPriority.h"
#include "components/AllComponents.h"
#include "cocos2d.h"

namespace ecs {

/**
 * @brief 减益效果系统 - 处理冷冻、冰冻和中毒减益的计时
 */
class DebuffSystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "DebuffSystem"; }
    int getPriority() const override { return SystemPriority::AI - 5; }

    void update(float delta) override {
        auto view = _registry->view<DebuffComponent>();
        
        view.each([delta, this](auto entity, DebuffComponent& debuff) {
            // 更新冷冻计时器
            if (debuff.hasChillDebuff) {
                debuff.chillTimer += delta;
                if (debuff.chillTimer >= debuff.chillDuration) {
                    debuff.hasChillDebuff = false;
                    CCLOG("Entity %u: Chill debuff expired", entt::to_integral(entity));
                }
            }
            
            // 更新冰冻计时器
            if (debuff.hasFreezeDebuff) {
                debuff.freezeTimer += delta;
                if (debuff.freezeTimer >= debuff.freezeDuration) {
                    debuff.hasFreezeDebuff = false;
                    CCLOG("Entity %u: Freeze debuff expired", entt::to_integral(entity));
                }
            }
            
            // 更新中毒计时器和伤害
            if (debuff.hasPoisonDebuff) {
                debuff.poisonTimer += delta;
                debuff.poisonTickTimer += delta;
                
                // 每秒造成一次伤害
                if (debuff.poisonTickTimer >= 1.0f) {
                    debuff.poisonTickTimer -= 1.0f;
                    
                    // 对实体造成毒素伤害
                    if (auto* health = _registry->try_get<HealthComponent>(entity)) {
                        health->takeDamage(debuff.poisonDamagePerSecond);
                        CCLOG("Entity %u: Poison tick %.1f damage (%.1fs remaining)", 
                              entt::to_integral(entity), 
                              debuff.poisonDamagePerSecond,
                              debuff.poisonDuration - debuff.poisonTimer);
                    }
                }
                
                if (debuff.poisonTimer >= debuff.poisonDuration) {
                    debuff.hasPoisonDebuff = false;
                    CCLOG("Entity %u: Poison debuff expired", entt::to_integral(entity));
                }
            }
        });
    }
};

} // namespace ecs

#endif // __ECS_SYSTEM_DEBUFFSYSTEMENTT_H__
