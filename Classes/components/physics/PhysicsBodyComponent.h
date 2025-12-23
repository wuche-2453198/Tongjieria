#ifndef __ECS_COMPONENT_PHYSICSBODYCOMPONENT_H__
#define __ECS_COMPONENT_PHYSICSBODYCOMPONENT_H__

#include "cocos2d.h"

namespace ecs {

/**
 * @brief 物理体配置组件 - 存储物理体创建参数（新解耦架构）
 * 
 * 用途：
 * - 告诉RenderSystem如何创建物理体
 * - 纯数据，不持有PhysicsBody指针
 * - 物理体由RenderSystem创建并附加到Sprite
 */
struct PhysicsBodyComponent {
    enum class BodyShape {
        Box,
        Circle
    };
    
    BodyShape shape = BodyShape::Box;
    float width = 40.0f;
    float height = 40.0f;
    float radius = 20.0f;  // 用于圆形
    cocos2d::Vec2 offset = cocos2d::Vec2::ZERO;  // 物理体偏移
    
    // 物理材质
    float density = 1.0f;
    float restitution = 0.0f;  // 弹性
    float friction = 0.5f;     // 摩擦力
    
    // 物理属性
    bool dynamic = true;
    bool rotationEnabled = false;
    bool gravityEnabled = true;
    float velocityLimit = 500.0f;
    float linearDamping = 0.0f;
    float angularDamping = 0.0f;
    
    // 碰撞配置
    int categoryBitmask = 0x0002;      // 默认敌人类别
    int contactTestBitmask = 0xFFFFFFFF;
    int collisionBitmask = 0xFFFFFFFF;
    int group = 0;
    
    PhysicsBodyComponent() = default;
};

} // namespace ecs

#endif // __ECS_COMPONENT_PHYSICSBODYCOMPONENT_H__
