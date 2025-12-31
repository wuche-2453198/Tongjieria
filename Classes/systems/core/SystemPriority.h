#ifndef __ECS_SYSTEM_PRIORITY_H__
#define __ECS_SYSTEM_PRIORITY_H__

namespace ecs {

/**
 * @brief 系统执行优先级（数值越小越先执行）
 * 
 * 优先级分层设计：
 * - INPUT (0): 输入处理，最高优先级
 * - PHYSICS (100): 物理模拟
 * - COLLISION (150): 碰撞检测
 * - AI (200): AI 和逻辑处理
 * - MOVEMENT (300): 移动处理
 * - STATE (400): 状态更新（实体状态标记等）
 * - ANIMATION (450): 动画更新
 * - LIFETIME (500): 生命周期管理
 * - RENDER (600): 渲染（最低优先级）
 * - CLEANUP (700): 清理工作
 */
namespace SystemPriority {
// 输入处理（最高优先级）
constexpr int INPUT = 0;

// 物理模拟
constexpr int PHYSICS = 100;
constexpr int COLLISION = 150;

// AI 和逻辑
constexpr int AI = 200;
constexpr int MOVEMENT = 300;

// 状态更新
constexpr int STATE = 400;
constexpr int ANIMATION = 450;

// 生命周期管理
constexpr int LIFETIME = 500;

// 渲染（最低优先级）
constexpr int RENDER = 600;
constexpr int CLEANUP = 700;
}

} // namespace ecs

#endif // __ECS_SYSTEM_PRIORITY_H__
