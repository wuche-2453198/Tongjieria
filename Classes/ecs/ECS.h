#ifndef __ECS_H__
#define __ECS_H__

/**
 * @file ECS.h
 * @brief ECS框架统一头文件（使用EnTT库）
 *
 * 设计原则:
 * 1. Entity - 由EnTT管理的实体
 * 2. Component - 纯数据结构，不包含逻辑
 * 3. System - 处理特定组件组合的逻辑（使用ISystemEntt接口）
 *
 * 使用示例:
 * @code
 * #include "ecs/ECS.h"
 * #include "ecs/SystemsEntt.h"
 * #include <entt/entt.hpp>
 *
 * // 创建注册表
 * entt::registry registry;
 * ecs::SystemManagerEntt systemManager;
 * systemManager.setRegistry(&registry);
 *
 * // 添加系统
 * systemManager.addSystem<ecs::SlimeRenderSystemEntt>();
 * systemManager.addSystem<ecs::JumpMovementSystemEntt>();
 *
 * // 创建实体
 * auto entity = registry.create();
 * registry.emplace<ecs::TransformComponent>(entity, 100.0f, 200.0f);
 * registry.emplace<ecs::PlayerTag>(entity);
 *
 * // 每帧更新
 * systemManager.update(deltaTime);
 * @endcode
 */

// 核心类型和组件
#include "Entity.h"
#include "AllComponents.h"

namespace ecs {

// ==================== 预定义碰撞层 ====================

namespace CollisionLayer {
constexpr int NONE = 0;
constexpr int PLAYER = 1 << 0;     // 1
constexpr int ENEMY = 1 << 1;      // 2
constexpr int GROUND = 1 << 2;     // 4
constexpr int WALL = 1 << 3;       // 8
constexpr int PROJECTILE = 1 << 4; // 16
constexpr int ITEM = 1 << 5;       // 32
constexpr int TRIGGER = 1 << 6;    // 64
constexpr int ALL = 0xFFFFFFFF;
} // namespace CollisionLayer

} // namespace ecs

#endif // __ECS_H__
