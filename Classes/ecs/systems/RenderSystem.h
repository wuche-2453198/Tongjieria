#ifndef __ECS_SYSTEM_RENDERSYSTEM_H__
#define __ECS_SYSTEM_RENDERSYSTEM_H__

#include "ISystemEntt.h"
#include "SystemPriority.h"
#include "../AllComponents.h"
#include <entt/entt.hpp>

namespace ecs {

/**
 * @brief 渲染系统 - 管理Sprite的创建、更新和销毁（新解耦架构）
 * 
 * 职责：
 * - 根据RenderComponent创建Sprite
 * - 同步Transform到Sprite位置
 * - 同步RenderComponent属性到Sprite
 * - 销毁实体时清理Sprite
 * 
 * 工作流程：
 * 1. 检测新实体（有RenderComponent但无SpriteStateComponent）
 * 2. 创建Sprite并添加SpriteStateComponent
 * 3. 每帧同步Transform和RenderComponent到Sprite
 * 
 * 设计优势：
 * - 组件不持有Cocos2d对象，易于测试
 * - 集中管理Sprite生命周期
 * - 支持热重载和序列化
 */
class RenderSystem : public ISystemEntt {
public:
    const char* getName() const override { return "RenderSystem"; }
    int getPriority() const override { return SystemPriority::RENDER; }
    
    void update(float delta) override;
    
private:
    // 创建精灵（首次渲染）
    void createSprites();
    
    // 同步Transform到Sprite位置
    void syncTransformToSprite();
    
    // 同步RenderComponent属性到Sprite
    void syncRenderProperties();
    
    // 清理已销毁实体的Sprite
    void cleanupDestroyedSprites();
};

/**
 * @brief 动画系统 - 处理帧动画播放（新解耦架构）
 * 
 * 职责：
 * - 更新AnimationComponent的帧计时器
 * - 根据帧序列切换SpriteFrame
 * - 处理循环和停止逻辑
 * - 支持基于AI状态的动画切换（通过AnimationStateComponent）
 */
class AnimationSystem : public ISystemEntt {
public:
    const char* getName() const override { return "AnimationSystem"; }
    int getPriority() const override { return SystemPriority::ANIMATION; }
    
    void update(float delta) override;
    
private:
    // 更新帧动画
    void updateFrameAnimation(float delta);
    
    // 更新状态驱动的动画切换
    void updateStateDrivenAnimation();
    
    // 应用动画状态数据到AnimationComponent
    void applyAnimationStateData(AnimationComponent& anim, const AnimationStateData& stateData);
};

/**
 * @brief Sprite销毁监听器 - 清理组件时释放Sprite
 * 
 * 使用方法：在Scene初始化时注册
 * @code
 * SpriteDestructionObserver::registerToRegistry(registry);
 * @endcode
 */
class SpriteDestructionObserver {
public:
    static void registerToRegistry(entt::registry& registry);
    
private:
    static void onSpriteStateDestroy(entt::registry& registry, entt::entity entity);
};

} // namespace ecs

#endif // __ECS_SYSTEM_RENDERSYSTEM_H__
