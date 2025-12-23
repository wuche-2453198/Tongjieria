#ifndef __ECS_COMPONENT_INITIALVELOCITYCOMPONENT_H__
#define __ECS_COMPONENT_INITIALVELOCITYCOMPONENT_H__

#include "cocos2d.h"

namespace ecs {

/**
 * @brief 初始速度组件 - 用于在物理体创建后立即设置速度
 * 
 * 用途：
 * - 投射物等需要在创建时立即设置速度的实体
 * - RenderSystem创建物理体后会检查此组件并应用速度
 * - 应用后自动移除此组件
 */
struct InitialVelocityComponent {
    cocos2d::Vec2 velocity = cocos2d::Vec2::ZERO;
    bool applied = false;
    
    InitialVelocityComponent() = default;
    InitialVelocityComponent(const cocos2d::Vec2& vel) : velocity(vel) {}
};

} // namespace ecs

#endif // __ECS_COMPONENT_INITIALVELOCITYCOMPONENT_H__
