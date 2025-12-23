#ifndef __ECS_SYSTEM_SLIMERENDERSYSTEMENTT_H__
#define __ECS_SYSTEM_SLIMERENDERSYSTEMENTT_H__

#include "systems/core/ISystemEntt.h"
#include "systems/core/SystemPriority.h"
#include "components/AllComponents.h"

namespace ecs {

/**
 * @brief 史莱姆渲染系统 - 同步Transform到SlimeSprite
 */
class SlimeRenderSystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "SlimeRenderSystem"; }
    int getPriority() const override { return SystemPriority::RENDER; }

    void update(float delta) override {
        // 新架构：RenderSystem和AnimationSystem已经处理了渲染和动画
        // 这个系统现在只需要从物理体同步位置到Transform
        auto view = _registry->view<TransformComponent, SpriteStateComponent>();
        
        view.each([](auto entity, TransformComponent& transform, SpriteStateComponent& state) {
            if (!state.spriteCreated || !state.spriteHandle)
                return;

            auto* sprite = static_cast<cocos2d::Sprite*>(state.spriteHandle);
            
            // 从物理引擎读取位置到transform
            if (auto* body = sprite->getPhysicsBody()) {
                transform.position = sprite->getPosition();
            }
        });
    }
};

} // namespace ecs

#endif // __ECS_SYSTEM_SLIMERENDERSYSTEMENTT_H__
