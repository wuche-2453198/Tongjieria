#ifndef __ECS_COMPONENT_H__
#define __ECS_COMPONENT_H__

#include "Entity.h"
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

namespace ecs {

/**
 * @brief 组件基类 - 所有组件必须继承此类
 *
 * 组件应该是纯数据结构（POD-like），不包含复杂逻辑
 * 逻辑应该放在System中处理
 */
struct IComponent {
  virtual ~IComponent() = default;

  /**
   * @brief 组件是否激活
   */
  bool enabled = true;
};

/**
 * @brief 组件池接口 - 用于类型擦除
 */
class IComponentPool {
public:
  virtual ~IComponentPool() = default;
  virtual void remove(EntityId entity) = 0;
  virtual bool has(EntityId entity) const = 0;
  virtual void clear() = 0;
  virtual size_t size() const = 0;
};

/**
 * @brief 组件池 - 存储特定类型的所有组件实例
 *
 * 使用稀疏数组实现，以实体ID为索引
 * @tparam T 组件类型
 */
template <typename T> class ComponentPool : public IComponentPool {
  static_assert(std::is_base_of<IComponent, T>::value,
                "Component must inherit from IComponent");

public:
  ComponentPool() = default;

  /**
   * @brief 为实体添加组件
   * @param entity 实体ID
   * @param args 构造参数
   * @return 组件引用
   */
  template <typename... Args> T &add(EntityId entity, Args &&...args) {
    auto [it, inserted] =
        _components.try_emplace(entity, std::forward<Args>(args)...);
    if (inserted) {
      _entities.push_back(entity);
    }
    return it->second;
  }

  /**
   * @brief 获取实体的组件
   * @param entity 实体ID
   * @return 组件指针，如果不存在返回nullptr
   */
  T *get(EntityId entity) {
    auto it = _components.find(entity);
    return it != _components.end() ? &it->second : nullptr;
  }

  const T *get(EntityId entity) const {
    auto it = _components.find(entity);
    return it != _components.end() ? &it->second : nullptr;
  }

  /**
   * @brief 移除实体的组件
   */
  void remove(EntityId entity) override {
    if (_components.erase(entity) > 0) {
      _entities.erase(std::remove(_entities.begin(), _entities.end(), entity),
                      _entities.end());
    }
  }

  /**
   * @brief 检查实体是否有该组件
   */
  bool has(EntityId entity) const override {
    return _components.find(entity) != _components.end();
  }

  /**
   * @brief 清空所有组件
   */
  void clear() override {
    _components.clear();
    _entities.clear();
  }

  /**
   * @brief 获取组件数量
   */
  size_t size() const override { return _components.size(); }

  /**
   * @brief 获取所有拥有该组件的实体
   */
  const std::vector<EntityId> &getEntities() const { return _entities; }

  /**
   * @brief 遍历所有组件
   */
  template <typename Func> void forEach(Func &&func) {
    for (auto &[entity, component] : _components) {
      if (component.enabled) {
        func(entity, component);
      }
    }
  }

  /**
   * @brief 遍历所有组件（包括禁用的）
   */
  template <typename Func> void forEachAll(Func &&func) {
    for (auto &[entity, component] : _components) {
      func(entity, component);
    }
  }

private:
  std::unordered_map<EntityId, T> _components;
  std::vector<EntityId> _entities; // 用于快速遍历
};

} // namespace ecs

#endif // __ECS_COMPONENT_H__
