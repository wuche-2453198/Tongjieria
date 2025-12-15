#ifndef __ECS_SYSTEM_H__
#define __ECS_SYSTEM_H__

#include "Entity.h"
#include <string>

namespace ecs {

// 前向声明
class World;

/**
 * @brief 系统基类 - 所有系统必须继承此类
 *
 * 系统包含处理组件的逻辑
 * 每个系统应该只关注特定类型的组件组合
 */
class ISystem {
public:
  virtual ~ISystem() = default;

  /**
   * @brief 系统初始化 - 在添加到世界时调用
   * @param world 世界引用
   */
  virtual void init(World *world) {
    _world = world;
    _enabled = true;
  }

  /**
   * @brief 每帧更新
   * @param delta 帧间隔时间
   */
  virtual void update(float delta) = 0;

  /**
   * @brief 固定时间步更新 (用于物理等)
   * @param fixedDelta 固定时间步
   */
  virtual void fixedUpdate(float fixedDelta) {}

  /**
   * @brief 渲染后更新 (用于UI等)
   * @param delta 帧间隔时间
   */
  virtual void lateUpdate(float delta) {}

  /**
   * @brief 系统销毁时调用
   */
  virtual void shutdown() {}

  /**
   * @brief 获取系统名称 (用于调试)
   */
  virtual const char *getName() const = 0;

  /**
   * @brief 获取系统优先级 (数字越小越先执行)
   */
  virtual int getPriority() const { return 0; }

  /**
   * @brief 系统是否启用
   */
  bool isEnabled() const { return _enabled; }
  void setEnabled(bool enabled) { _enabled = enabled; }

protected:
  World *_world = nullptr;
  bool _enabled = true;
};

/**
 * @brief 系统优先级常量
 */
namespace SystemPriority {
constexpr int INPUT = -1000;   // 输入处理
constexpr int AI = -500;       // AI决策
constexpr int PHYSICS = 0;     // 物理模拟
constexpr int MOVEMENT = 100;  // 移动处理
constexpr int COLLISION = 200; // 碰撞检测
constexpr int ANIMATION = 500; // 动画更新
constexpr int RENDER = 1000;   // 渲染
constexpr int UI = 2000;       // UI更新
} // namespace SystemPriority

} // namespace ecs

#endif // __ECS_SYSTEM_H__
