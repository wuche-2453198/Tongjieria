#ifndef __ECS_ENTITY_HANDLE_H__
#define __ECS_ENTITY_HANDLE_H__

#include "Entity.h"
#include <functional>
#include <type_traits>

namespace ecs {

// 前向声明
class World;

/**
 * @brief 实体句柄 - 提供便捷的实体操作接口
 *
 * EntityHandle 是对 EntityId 的封装，提供：
 * - 链式调用添加/获取组件
 * - 自动验证实体有效性
 * - 更直观的 API
 *
 * 使用示例:
 * @code
 * EntityHandle entity = world.entity();
 * entity.add<TransformComponent>(100, 200)
 *       .add<HealthComponent>(100)
 *       .add<EnemyTag>("Slime")
 *       .setTag("Monster");
 *
 * if (auto* health = entity.get<HealthComponent>()) {
 *     health->takeDamage(10);
 * }
 * @endcode
 */
class EntityHandle {
public:
  EntityHandle() : _world(nullptr), _id(INVALID_ENTITY) {}
  EntityHandle(World *world, EntityId id) : _world(world), _id(id) {}

  // ==================== 基础操作 ====================

  /**
   * @brief 获取实体ID
   */
  EntityId id() const { return _id; }

  /**
   * @brief 获取所属世界
   */
  World *world() const { return _world; }

  /**
   * @brief 检查实体是否有效
   */
  bool valid() const;

  /**
   * @brief 隐式转换为 EntityId
   */
  operator EntityId() const { return _id; }

  /**
   * @brief 布尔转换 - 检查有效性
   */
  explicit operator bool() const { return valid(); }

  // ==================== 组件操作 ====================

  /**
   * @brief 添加组件 (链式调用)
   * @tparam T 组件类型
   * @param args 构造参数
   * @return 自身引用，支持链式调用
   */
  template <typename T, typename... Args> EntityHandle &add(Args &&...args);

  /**
   * @brief 获取组件
   * @return 组件指针，不存在返回 nullptr
   */
  template <typename T> T *get();

  template <typename T> const T *get() const;

  /**
   * @brief 检查是否有指定组件
   */
  template <typename T> bool has() const;

  /**
   * @brief 检查是否有所有指定组件
   */
  template <typename... Ts> bool hasAll() const;

  /**
   * @brief 移除组件
   */
  template <typename T> EntityHandle &remove();

  /**
   * @brief 如果有组件则执行操作
   * @param func 回调函数 (T&) -> void
   * @return 自身引用
   */
  template <typename T, typename Func> EntityHandle &ifHas(Func &&func);

  // ==================== 标签操作 ====================

  /**
   * @brief 设置标签
   */
  EntityHandle &setTag(const std::string &tag);

  /**
   * @brief 获取标签
   */
  const std::string &getTag() const;

  // ==================== 生命周期 ====================

  /**
   * @brief 销毁实体 (延迟到帧末)
   */
  void destroy();

  /**
   * @brief 立即销毁实体
   */
  void destroyImmediate();

  // ==================== 比较操作 ====================

  bool operator==(const EntityHandle &other) const { return _id == other._id; }
  bool operator!=(const EntityHandle &other) const { return _id != other._id; }
  bool operator==(EntityId other) const { return _id == other; }
  bool operator!=(EntityId other) const { return _id != other; }

private:
  World *_world;
  EntityId _id;
};

/**
 * @brief 实体构建器 - 链式创建实体并配置组件
 *
 * 使用示例:
 * @code
 * auto entity = EntityBuilder(world)
 *     .withTag("Player")
 *     .with<TransformComponent>(100, 200)
 *     .with<HealthComponent>(100)
 *     .configure<MovementComponent>([](MovementComponent& m) {
 *         m.moveSpeed = 150.0f;
 *         m.jumpForce = 400.0f;
 *     })
 *     .build();
 * @endcode
 */
class EntityBuilder {
public:
  explicit EntityBuilder(World &world);

  /**
   * @brief 设置实体标签
   */
  EntityBuilder &withTag(const std::string &tag);

  /**
   * @brief 添加组件
   */
  template <typename T, typename... Args> EntityBuilder &with(Args &&...args);

  /**
   * @brief 添加组件并配置
   * @param configurator 配置函数 (T&) -> void
   */
  template <typename T, typename Func>
  EntityBuilder &configure(Func &&configurator);

  /**
   * @brief 添加组件（如果条件为真）
   */
  template <typename T, typename... Args>
  EntityBuilder &withIf(bool condition, Args &&...args);

  /**
   * @brief 从另一个实体复制组件
   */
  template <typename T> EntityBuilder &copyFrom(EntityHandle source);

  /**
   * @brief 应用预设配置
   * @param preset 预设函数 (EntityBuilder&) -> void
   */
  template <typename Func> EntityBuilder &apply(Func &&preset);

  /**
   * @brief 完成构建，返回实体句柄
   */
  EntityHandle build();

  /**
   * @brief 完成构建，返回实体ID
   */
  EntityId buildId();

private:
  World &_world;
  EntityId _entity;
  std::string _tag;
};

} // namespace ecs

#endif // __ECS_ENTITY_HANDLE_H__
