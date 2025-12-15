#ifndef __ECS_H__
#define __ECS_H__

/**
 * @file ECS.h
 * @brief 轻量级ECS框架 - 统一包含头文件
 *
 * 设计原则:
 * 1. Entity - 只是一个ID，不包含任何数据
 * 2. Component - 纯数据结构，不包含逻辑
 * 3. System - 处理特定组件组合的逻辑
 * 4. World - 管理所有实体、组件和系统
 *
 * 使用示例:
 * @code
 * #include "ecs/ECS.h"
 * using namespace ecs;
 *
 * // 创建世界
 * World world;
 *
 * // 添加系统
 * world.addSystem<SlimeRenderSystem>();
 * world.addSystem<JumpMovementSystem>();
 *
 * // 创建实体
 * EntityId player = world.createEntity("Player");
 * world.addComponent<TransformComponent>(player, 100.0f, 200.0f);
 * world.addComponent<PlayerTag>(player);
 *
 * // 每帧更新
 * world.update(deltaTime);
 */

// 核心类型
#include "Component.h"
#include "Entity.h"
#include "EntityHandle.h"
#include "System.h"
#include "World.h"

// 组件系统
#include "Components.h"
#include "SpriteComponent.h"

// 游戏系统
#include "Systems.h"

namespace ecs {

// ==================== 便捷工厂函数 ====================

/**
 * @brief 创建全局世界单例 (可选使用)
 */
class GameWorld {
public:
  static World &getInstance() {
    static World instance;
    return instance;
  }

  // 禁止拷贝
  GameWorld(const GameWorld &) = delete;
  GameWorld &operator=(const GameWorld &) = delete;

private:
  GameWorld() = default;
};

// EntityBuilder 已移动到 EntityHandle.h
// 使用示例:
// @code
// EntityBuilder(world)
//     .withTag("Player")
//     .with<TransformComponent>(100, 200)
//     .with<HealthComponent>(100)
//     .build();
// @endcode

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
