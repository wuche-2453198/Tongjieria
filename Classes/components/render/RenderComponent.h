#ifndef __ECS_COMPONENT_RENDERCOMPONENT_H__
#define __ECS_COMPONENT_RENDERCOMPONENT_H__

#include "cocos2d.h"
#include <string>

namespace ecs {

/**
 * @brief 纯数据渲染组件 - 不持有Cocos2d对象（新解耦架构）
 * 
 * 设计原则：
 * - 只包含渲染所需的数据
 * - 不持有Cocos2d指针
 * - 可序列化、可测试
 * - 由RenderSystem负责创建和管理实际的Sprite对象
 */
struct RenderComponent {
    std::string spriteResourceId;      // 精灵资源ID（用于SpriteManager查找）
    cocos2d::Vec2 renderOffset;        // 渲染偏移（相对于Transform位置）
    float scale = 1.0f;                // 缩放
    bool flipX = false;                // 水平翻转
    bool flipY = false;                // 垂直翻转
    int zOrder = 0;                    // 渲染层级
    bool visible = true;               // 是否可见
    cocos2d::Color3B color = cocos2d::Color3B::WHITE;  // 颜色
    uint8_t opacity = 255;             // 不透明度
    float rotation = 0.0f;             // 旋转角度
    bool enableSync = true;            // 是否启用属性同步（禁用时允许Cocos2d Action控制）
    
    RenderComponent() = default;
    RenderComponent(const std::string& resId) : spriteResourceId(resId) {}
};

} // namespace ecs

#endif // __ECS_COMPONENT_RENDERCOMPONENT_H__
