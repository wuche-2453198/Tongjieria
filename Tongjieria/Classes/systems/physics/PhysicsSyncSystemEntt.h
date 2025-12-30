#ifndef __ECS_SYSTEM_PHYSICSSYNCSYSTEMENTT_H__
#define __ECS_SYSTEM_PHYSICSSYNCSYSTEMENTT_H__

#include "systems/core/ISystemEntt.h"
#include "systems/core/SystemPriority.h"
#include "components/AllComponents.h"

namespace ecs {

/**
 * @brief 统一物理同步系统 - 从物理体同步位置到Transform
 * 
 * 此系统替代了以下重复的系统：
 * - SlimeRenderSystemEntt
 * - SlimeSyncSystemEntt  
 * - MonsterSyncSystemEntt
 * 
 * 职责：
 * - 从物理引擎驱动的Sprite读取位置
 * - 同步到TransformComponent
 * - 统一处理所有带物理体的实体
 * 
 * 优化效果：
 * - 减少系统数量（3个 -> 1个）
 * - 减少每帧遍历次数
 * - 避免重复的位置同步操作
 */
class PhysicsSyncSystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "PhysicsSyncSystem"; }
    
    // 优先级：在渲染之前，在物理之后
    int getPriority() const override { return SystemPriority::RENDER - 10; }

    void update(float delta) override {
        // 遍历所有有Transform和SpriteState的实体
        auto view = _registry->view<TransformComponent, SpriteStateComponent, PhysicsBodyComponent>();
        
        view.each([this](auto entity, TransformComponent& transform, SpriteStateComponent& state, PhysicsBodyComponent&) {
            auto* pooled = _registry->try_get<PooledEntity>(entity);
            if (pooled && !pooled->inUse) {
                return;
            }

            // 跳过无效精灵
            if (!state.spriteCreated || !state.spriteHandle) {
                return;
            }

            auto* sprite = static_cast<cocos2d::Sprite*>(state.spriteHandle);
            
            // 只有带物理体的实体才需要从精灵同步位置
            // （物理引擎会自动更新精灵位置）
            if (sprite->getPhysicsBody()) {
                transform.position = sprite->getPosition();
            }
        });
    }
};

} // namespace ecs

#endif // __ECS_SYSTEM_PHYSICSSYNCSYSTEMENTT_H__
