#ifndef __ECS_COMPONENT_SPRITESTATECOMPONENT_H__
#define __ECS_COMPONENT_SPRITESTATECOMPONENT_H__

namespace ecs {

/**
 * @brief 精灵状态组件 - 记录精灵的运行时状态（新解耦架构）
 * 
 * 由RenderSystem内部使用，业务逻辑不应直接访问
 * 
 * 用途：
 * - 标记Sprite是否已创建
 * - 保存Sprite句柄供RenderSystem使用
 * - 避免重复创建Sprite
 */
struct SpriteStateComponent {
    bool spriteCreated = false;        // 精灵是否已创建
    void* spriteHandle = nullptr;      // 精灵句柄（由SpriteManager管理）
    
    SpriteStateComponent() = default;
};

} // namespace ecs

#endif // __ECS_COMPONENT_SPRITESTATECOMPONENT_H__
