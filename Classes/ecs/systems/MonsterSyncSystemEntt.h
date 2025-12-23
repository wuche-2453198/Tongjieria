#ifndef __ECS_SYSTEM_MONSTERSYNCSYSTEMENTT_H__
#define __ECS_SYSTEM_MONSTERSYNCSYSTEMENTT_H__

#include "ISystemEntt.h"
#include "SystemPriority.h"
#include "../AllComponents.h"

namespace ecs {

/**
 * @brief 怪物同步系统 - 同步Transform位置到精灵（从物理体读取）
 */
class MonsterSyncSystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "MonsterSyncSystem"; }
    int getPriority() const override { return SystemPriority::RENDER - 10; }

    void update(float delta) override {
        auto view = _registry->view<TransformComponent, SpriteStateComponent>();
        
        view.each([this, delta](auto entity, TransformComponent& transform,
                               SpriteStateComponent& state) {
            if (!state.spriteCreated || !state.spriteHandle)
                return;

            auto* sprite = static_cast<cocos2d::Sprite*>(state.spriteHandle);
            auto* body = sprite->getPhysicsBody();
            if (!body)
                return;

            // 从精灵位置同步到Transform（物理体驱动）
            transform.position = body->getPosition();
        });
    }
};

} // namespace ecs

#endif // __ECS_SYSTEM_MONSTERSYNCSYSTEMENTT_H__
