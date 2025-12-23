#ifndef __ECS_SYSTEM_SLIMESYNCSYSTEMENTT_H__
#define __ECS_SYSTEM_SLIMESYNCSYSTEMENTT_H__

#include "systems/core/ISystemEntt.h"
#include "systems/core/SystemPriority.h"
#include "components/AllComponents.h"

namespace ecs {

/**
 * @brief 史莱姆同步系统 - 从精灵读取位置到Transform（物理引擎驱动精灵位置）
 */
class SlimeSyncSystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "SlimeSyncSystem"; }
    int getPriority() const override { return SystemPriority::RENDER - 10; }

    void update(float delta) override {
        auto view = _registry->view<TransformComponent, SpriteStateComponent>();
        
        view.each([delta](auto entity, TransformComponent& transform,
                         SpriteStateComponent& state) {
            if (!state.spriteCreated || !state.spriteHandle)
                return;

            auto* sprite = static_cast<cocos2d::Sprite*>(state.spriteHandle);
            // 物理引擎自动更新精灵位置
            transform.position = sprite->getPosition();
        });
    }
};

} // namespace ecs

#endif // __ECS_SYSTEM_SLIMESYNCSYSTEMENTT_H__
