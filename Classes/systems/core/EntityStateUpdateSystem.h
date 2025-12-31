#ifndef __ECS_SYSTEM_ENTITYSTATEUPDATESYSTEM_H__
#define __ECS_SYSTEM_ENTITYSTATEUPDATESYSTEM_H__

#include "systems/core/ISystemEntt.h"
#include "systems/core/SystemPriority.h"
#include "components/core/EntityStateFlags.h"
#include "components/core/TransformComponent.h"
#include "components/npc/EnemyTag.h"
#include "cocos2d.h"
#include <cmath>

namespace ecs {

/**
 * @brief 实体状态更新系统 - 计算并更新实体的运行时状态
 * 
 * 该系统负责：
 * - 计算实体到玩家的距离
 * - 更新 isOnScreen 状态（基于屏幕可见区域）
 * - 更新 isIdle 状态（基于速度和行为）
 * - 更新 lastUpdateFrame（用于降频更新）
 * 
 * 优先级：STATE (400) - 在 AI 和 MOVEMENT 之后，ANIMATION 之前
 * 
 * Requirements: 5.1, 5.5, 5.6
 */
class EntityStateUpdateSystem : public ISystemEntt {
public:
    const char* getName() const override { return "EntityStateUpdateSystem"; }
    int getPriority() const override { return SystemPriority::STATE; }

    /**
     * @brief 设置玩家实体（用于距离计算）
     * @param playerEntity 玩家实体ID
     */
    void setPlayerEntity(entt::entity playerEntity) {
        _playerEntity = playerEntity;
    }

    /**
     * @brief 设置屏幕边界扩展（用于离屏判断）
     * @param margin 屏幕边界外的额外距离（像素）
     */
    void setScreenMargin(float margin) {
        _screenMargin = margin;
    }

    /**
     * @brief 设置空闲速度阈值
     * @param threshold 低于此速度视为空闲
     */
    void setIdleSpeedThreshold(float threshold) {
        _idleSpeedThreshold = threshold;
    }

    void update(float delta) override {
        if (!_registry) return;

        // 增加帧计数器
        _frameCounter++;

        // 获取玩家位置
        cocos2d::Vec2 playerPos = cocos2d::Vec2::ZERO;
        bool hasPlayer = false;
        
        if (_registry->valid(_playerEntity)) {
            auto* playerTransform = _registry->try_get<TransformComponent>(_playerEntity);
            if (playerTransform) {
                playerPos = playerTransform->position;
                hasPlayer = true;
            }
        }

        // 获取屏幕尺寸和可见区域
        auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
        auto origin = cocos2d::Director::getInstance()->getVisibleOrigin();
        
        // 计算扩展后的屏幕边界（用于离屏判断）
        float screenLeft = origin.x - _screenMargin;
        float screenRight = origin.x + visibleSize.width + _screenMargin;
        float screenBottom = origin.y - _screenMargin;
        float screenTop = origin.y + visibleSize.height + _screenMargin;

        // 更新所有带有 EntityStateFlags 和 TransformComponent 的实体
        auto view = _registry->view<EntityStateFlags, TransformComponent>();
        
        view.each([&](auto entity, EntityStateFlags& stateFlags, TransformComponent& transform) {
            // 1. 计算到玩家的距离
            if (hasPlayer) {
                float dx = transform.position.x - playerPos.x;
                float dy = transform.position.y - playerPos.y;
                stateFlags.distanceToPlayer = std::sqrt(dx * dx + dy * dy);
            } else {
                stateFlags.distanceToPlayer = 0.0f;
            }

            // 2. 更新 isOnScreen 状态
            stateFlags.isOnScreen = isPositionOnScreen(transform.position, 
                                                        screenLeft, screenRight, 
                                                        screenBottom, screenTop);

            // 3. 更新 isIdle 状态（基于速度）
            float speedSq = transform.velocity.lengthSquared();
            stateFlags.isIdle = (speedSq < _idleSpeedThreshold * _idleSpeedThreshold);

            // 4. 记录当前帧号（用于降频更新判断）
            // 注意：lastUpdateFrame 由各个系统在实际更新时设置
        });
    }

    /**
     * @brief 获取当前帧计数器
     * @return 当前帧号
     */
    int getFrameCounter() const { return _frameCounter; }

private:
    entt::entity _playerEntity = entt::null;
    float _screenMargin = 100.0f;        // 屏幕边界扩展（像素）
    float _idleSpeedThreshold = 5.0f;    // 空闲速度阈值
    int _frameCounter = 0;               // 帧计数器

    /**
     * @brief 判断位置是否在屏幕可见区域内
     */
    bool isPositionOnScreen(const cocos2d::Vec2& pos, 
                            float left, float right, 
                            float bottom, float top) const {
        return pos.x >= left && pos.x <= right && 
               pos.y >= bottom && pos.y <= top;
    }
};

} // namespace ecs

#endif // __ECS_SYSTEM_ENTITYSTATEUPDATESYSTEM_H__
