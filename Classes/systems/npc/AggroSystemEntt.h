#ifndef __ECS_SYSTEM_AGGROSYSTEMENTT_H__
#define __ECS_SYSTEM_AGGROSYSTEMENTT_H__

#include "systems/core/ISystemEntt.h"
#include "systems/core/SystemPriority.h"
#include "systems/core/EntityDestructionManager.h"
#include "components/AllComponents.h"
#include "cocos2d.h"

namespace ecs {

/**
 * @brief 仇恨检测系统 - 检测目标并更新仇恨状态
 */
class AggroSystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "AggroSystem"; }
    int getPriority() const override { return SystemPriority::AI - 10; }

    void update(float delta) override {
        entt::entity cachedPlayer = entt::null;
        TransformComponent* cachedPlayerTransform = nullptr;
        {
            auto playerView = _registry->view<PlayerTag, TransformComponent>();
            for (auto entity : playerView) {
                cachedPlayer = entity;
                cachedPlayerTransform = &playerView.get<TransformComponent>(entity);
                break;
            }
        }

        // 遍历所有具有仇恨组件的实体
        auto aggroView = _registry->view<AggroComponent, TransformComponent>();
        
        aggroView.each([this, cachedPlayer, cachedPlayerTransform](entt::entity entity, AggroComponent& aggro, 
                             TransformComponent& transform) {
            // 清除无效或待销毁的目标引用
            if (aggro.targetEntity != INVALID_ENTITY) {
                if (!isEntityReferenceValid(*_registry, aggro.targetEntity)) {
                    // 目标实体无效或待销毁，清除引用
                    aggro.targetEntity = INVALID_ENTITY;
                    aggro.distanceToTarget = 99999.0f;
                    aggro.directionToTarget = cocos2d::Vec2::ZERO;
                    if (aggro.hasAggro) {
                        aggro.hasAggro = false;
                        CCLOG("Entity %u: Target entity invalid or pending destruction, clearing reference",
                              entt::to_integral(entity));
                    }
                }
            }
            
            // EnTT高效查找：根据targetTag直接获取目标实体
            entt::entity target = entt::null;
            if (aggro.targetTag == "Player") {
                target = cachedPlayer;
            } else {
                target = findTargetByTag(aggro.targetTag);
            }
            // 转换entt::entity到EntityId（都是uint32_t）
            aggro.targetEntity = (target == entt::null) ? INVALID_ENTITY : entt::to_integral(target);

            if (target == entt::null) {
                aggro.distanceToTarget = 99999.0f;
                aggro.directionToTarget = cocos2d::Vec2::ZERO;
                if (aggro.hasAggro) {
                    aggro.hasAggro = false;
                    CCLOG("Entity %u: Lost target, exiting aggro", entt::to_integral(entity));
                }
                return;
            }

            // 计算到目标的距离和方向
            TransformComponent* targetTransform = nullptr;
            if (aggro.targetTag == "Player") {
                if (target == cachedPlayer) {
                    targetTransform = cachedPlayerTransform;
                }
            }
            if (!targetTransform) {
                targetTransform = _registry->try_get<TransformComponent>(target);
            }
            if (!targetTransform) {
                return;
            }

            cocos2d::Vec2 diff = targetTransform->position - transform.position;
            aggro.distanceToTarget = diff.length();
            aggro.directionToTarget = diff.getNormalized();

            // 仇恨状态切换
            if (aggro.shouldEnterAggro()) {
                aggro.hasAggro = true;
                CCLOG("Entity %u: Target in range (%.1f <= %.1f), entering aggro",
                      entt::to_integral(entity), aggro.distanceToTarget, aggro.aggroRange);
            } else if (aggro.shouldExitAggro()) {
                aggro.hasAggro = false;
                CCLOG("Entity %u: Target too far (%.1f > %.1f), exiting aggro",
                      entt::to_integral(entity), aggro.distanceToTarget, aggro.deaggroRange);
            }
        });
    }

private:
    /**
     * @brief 根据tag查找目标实体
     */
    entt::entity findTargetByTag(const std::string& tag) {
        // 目前只支持查找玩家
        if (tag == "Player") {
            // 使用PlayerTag组件查找玩家实体
            auto playerView = _registry->view<PlayerTag, TransformComponent>();
            // EnTT 3.x使用迭代器检查是否为空
            for (auto entity : playerView) {
                return entity;  // 返回第一个玩家（O(1)！）
            }
        }
        
        return entt::null;
    }
};

} // namespace ecs

#endif // __ECS_SYSTEM_AGGROSYSTEMENTT_H__
