#ifndef __ECS_WORLD_H__
#define __ECS_WORLD_H__

#include "Component.h"
#include "Entity.h"
#include "System.h"
#include "EntityHandle.h"
#include <algorithm>
#include <functional>
#include <memory>
#include <queue>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <vector>


namespace ecs {

/**
 * @brief 事件基类
 */
struct IEvent {
  virtual ~IEvent() = default;
};

/**
 * @brief 实体创建事件
 */
struct EntityCreatedEvent : public IEvent {
  EntityId entity;
  EntityCreatedEvent(EntityId e) : entity(e) {}
};

/**
 * @brief 实体销毁事件
 */
struct EntityDestroyedEvent : public IEvent {
  EntityId entity;
  EntityDestroyedEvent(EntityId e) : entity(e) {}
};

/**
 * @brief 组件添加事件
 */
template <typename T> struct ComponentAddedEvent : public IEvent {
  EntityId entity;
  T *component;
  ComponentAddedEvent(EntityId e, T *c) : entity(e), component(c) {}
};

/**
 * @brief 世界类 - ECS框架核心
 *
 * 管理所有实体、组件和系统的生命周期
 * 提供实体创建、组件管理、系统调度等功能
 */
class World {
public:
  World() = default;
  ~World() { shutdown(); }

  // ==================== 实体管理 ====================

  /**
   * @brief 创建新实体并返回句柄
   * @return 实体句柄
   */
  EntityHandle entity() {
    return EntityHandle(this, createEntity());
  }

  /**
   * @brief 创建带标签的实体并返回句柄
   */
  EntityHandle entity(const std::string &tag) {
    return EntityHandle(this, createEntity(tag));
  }

  /**
   * @brief 获取实体句柄
   */
  EntityHandle getHandle(EntityId id) {
    return EntityHandle(this, id);
  }

  /**
   * @brief 创建新实体
   * @return 实体ID
   */
  EntityId createEntity() {
    EntityId entity;
    if (!_recycledIds.empty()) {
      entity = _recycledIds.front();
      _recycledIds.pop();
    } else {
      entity = _nextEntityId++;
    }
    _activeEntities.insert(entity);
    _entityTags[entity] = "";
    emit(EntityCreatedEvent(entity));
    return entity;
  }

  /**
   * @brief 创建带标签的实体
   * @param tag 实体标签
   * @return 实体ID
   */
  EntityId createEntity(const std::string &tag) {
    EntityId entity = createEntity();
    setTag(entity, tag);
    return entity;
  }

  /**
   * @brief 销毁实体 (延迟到帧末执行)
   * @param entity 实体ID
   */
  void destroyEntity(EntityId entity) {
    if (isValid(entity)) {
      _entitiesToDestroy.push_back(entity);
    }
  }

  /**
   * @brief 立即销毁实体
   */
  void destroyEntityImmediate(EntityId entity) {
    if (!isValid(entity))
      return;

    emit(EntityDestroyedEvent(entity));

    // 移除所有组件
    for (auto &[typeId, pool] : _componentPools) {
      pool->remove(entity);
    }

    // 移除标签映射
    auto tagIt = _entityTags.find(entity);
    if (tagIt != _entityTags.end()) {
      if (!tagIt->second.empty()) {
        _tagToEntities[tagIt->second].erase(entity);
      }
      _entityTags.erase(tagIt);
    }

    _activeEntities.erase(entity);
    _recycledIds.push(entity);
  }

  /**
   * @brief 检查实体是否有效
   */
  bool isValid(EntityId entity) const {
    return _activeEntities.find(entity) != _activeEntities.end();
  }

  /**
   * @brief 获取所有活跃实体
   */
  const std::unordered_set<EntityId> &getEntities() const {
    return _activeEntities;
  }

  /**
   * @brief 获取活跃实体数量
   */
  size_t getEntityCount() const { return _activeEntities.size(); }

  // ==================== 标签系统 ====================

  /**
   * @brief 设置实体标签
   */
  void setTag(EntityId entity, const std::string &tag) {
    if (!isValid(entity))
      return;

    // 移除旧标签映射
    auto &oldTag = _entityTags[entity];
    if (!oldTag.empty()) {
      _tagToEntities[oldTag].erase(entity);
    }

    // 设置新标签
    oldTag = tag;
    if (!tag.empty()) {
      _tagToEntities[tag].insert(entity);
    }
  }

  /**
   * @brief 获取实体标签
   */
  const std::string &getTag(EntityId entity) const {
    static const std::string empty;
    auto it = _entityTags.find(entity);
    return it != _entityTags.end() ? it->second : empty;
  }

  /**
   * @brief 通过标签查找实体
   * @return 第一个匹配的实体，如果没找到返回INVALID_ENTITY
   */
  EntityId findEntityByTag(const std::string &tag) const {
    auto it = _tagToEntities.find(tag);
    if (it != _tagToEntities.end() && !it->second.empty()) {
      return *it->second.begin();
    }
    return INVALID_ENTITY;
  }

  /**
   * @brief 通过标签查找所有实体
   */
  std::vector<EntityId> findEntitiesByTag(const std::string &tag) const {
    auto it = _tagToEntities.find(tag);
    if (it != _tagToEntities.end()) {
      return std::vector<EntityId>(it->second.begin(), it->second.end());
    }
    return {};
  }

  // ==================== 组件管理 ====================

  /**
   * @brief 为实体添加组件
   * @tparam T 组件类型
   * @param entity 实体ID
   * @param args 构造参数
   * @return 组件引用
   */
  template <typename T, typename... Args>
  T &addComponent(EntityId entity, Args &&...args) {
    auto &pool = getOrCreatePool<T>();
    T &component = pool.add(entity, std::forward<Args>(args)...);
    return component;
  }

  /**
   * @brief 获取实体的组件
   * @return 组件指针，不存在返回nullptr
   */
  template <typename T> T *getComponent(EntityId entity) {
    auto *pool = getPool<T>();
    return pool ? pool->get(entity) : nullptr;
  }

  template <typename T> const T *getComponent(EntityId entity) const {
    auto *pool = getPool<T>();
    return pool ? pool->get(entity) : nullptr;
  }

  /**
   * @brief 检查实体是否有指定组件
   */
  template <typename T> bool hasComponent(EntityId entity) const {
    auto *pool = getPool<T>();
    return pool && pool->has(entity);
  }

  /**
   * @brief 检查实体是否有所有指定组件
   */
  template <typename T, typename T2, typename... Ts>
  bool hasComponents(EntityId entity) const {
    return hasComponent<T>(entity) && hasComponents<T2, Ts...>(entity);
  }

  template <typename T> bool hasComponents(EntityId entity) const {
    return hasComponent<T>(entity);
  }

  /**
   * @brief 移除实体的组件
   */
  template <typename T> void removeComponent(EntityId entity) {
    auto *pool = getPool<T>();
    if (pool) {
      pool->remove(entity);
    }
  }

  /**
   * @brief 获取组件池 (用于批量操作)
   */
  template <typename T> ComponentPool<T> *getPool() {
    ComponentTypeId typeId = getComponentTypeId<T>();
    auto it = _componentPools.find(typeId);
    if (it != _componentPools.end()) {
      return static_cast<ComponentPool<T> *>(it->second.get());
    }
    return nullptr;
  }

  template <typename T> const ComponentPool<T> *getPool() const {
    ComponentTypeId typeId = getComponentTypeId<T>();
    auto it = _componentPools.find(typeId);
    if (it != _componentPools.end()) {
      return static_cast<const ComponentPool<T> *>(it->second.get());
    }
    return nullptr;
  }

  // ==================== 视图查询 ====================

  /**
   * @brief 遍历拥有指定组件的所有实体
   * @tparam T 组件类型
   * @param func 回调函数 (EntityId, T&)
   */
  template <typename T, typename Func> void forEach(Func &&func) {
    auto *pool = getPool<T>();
    if (pool) {
      pool->forEach(std::forward<Func>(func));
    }
  }

  /**
   * @brief 遍历拥有多个组件的所有实体
   * @tparam T1, T2 组件类型
   * @param func 回调函数 (EntityId, T1&, T2&)
   */
  template <typename T1, typename T2, typename Func> void forEach(Func &&func) {
    auto *pool1 = getPool<T1>();
    auto *pool2 = getPool<T2>();
    if (!pool1 || !pool2)
      return;

    // 选择较小的池进行遍历
    if (pool1->size() <= pool2->size()) {
      pool1->forEach([&](EntityId entity, T1 &c1) {
        if (auto *c2 = pool2->get(entity)) {
          if (c2->enabled) {
            func(entity, c1, *c2);
          }
        }
      });
    } else {
      pool2->forEach([&](EntityId entity, T2 &c2) {
        if (auto *c1 = pool1->get(entity)) {
          if (c1->enabled) {
            func(entity, *c1, c2);
          }
        }
      });
    }
  }

  /**
   * @brief 遍历拥有三个组件的所有实体
   */
  template <typename T1, typename T2, typename T3, typename Func>
  void forEach(Func &&func) {
    auto *pool1 = getPool<T1>();
    auto *pool2 = getPool<T2>();
    auto *pool3 = getPool<T3>();
    if (!pool1 || !pool2 || !pool3)
      return;

    pool1->forEach([&](EntityId entity, T1 &c1) {
      auto *c2 = pool2->get(entity);
      auto *c3 = pool3->get(entity);
      if (c2 && c3 && c2->enabled && c3->enabled) {
        func(entity, c1, *c2, *c3);
      }
    });
  }

  /**
   * @brief 遍历拥有四个组件的所有实体
   */
  template <typename T1, typename T2, typename T3, typename T4, typename Func>
  void forEach(Func &&func) {
    auto *pool1 = getPool<T1>();
    auto *pool2 = getPool<T2>();
    auto *pool3 = getPool<T3>();
    auto *pool4 = getPool<T4>();
    if (!pool1 || !pool2 || !pool3 || !pool4)
      return;

    pool1->forEach([&](EntityId entity, T1 &c1) {
      auto *c2 = pool2->get(entity);
      auto *c3 = pool3->get(entity);
      auto *c4 = pool4->get(entity);
      if (c2 && c3 && c4 && c2->enabled && c3->enabled && c4->enabled) {
        func(entity, c1, *c2, *c3, *c4);
      }
    });
  }

  /**
   * @brief 遍历拥有五个组件的所有实体
   */
  template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename Func>
  void forEach(Func &&func) {
    auto *pool1 = getPool<T1>();
    auto *pool2 = getPool<T2>();
    auto *pool3 = getPool<T3>();
    auto *pool4 = getPool<T4>();
    auto *pool5 = getPool<T5>();
    if (!pool1 || !pool2 || !pool3 || !pool4 || !pool5)
      return;

    pool1->forEach([&](EntityId entity, T1 &c1) {
      auto *c2 = pool2->get(entity);
      auto *c3 = pool3->get(entity);
      auto *c4 = pool4->get(entity);
      auto *c5 = pool5->get(entity);
      if (c2 && c3 && c4 && c5 && c2->enabled && c3->enabled && c4->enabled &&
          c5->enabled) {
        func(entity, c1, *c2, *c3, *c4, *c5);
      }
    });
  }

  /**
   * @brief 遍历拥有六个组件的所有实体
   */
  template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename Func>
  void forEach(Func &&func) {
    auto *pool1 = getPool<T1>();
    auto *pool2 = getPool<T2>();
    auto *pool3 = getPool<T3>();
    auto *pool4 = getPool<T4>();
    auto *pool5 = getPool<T5>();
    auto *pool6 = getPool<T6>();
    if (!pool1 || !pool2 || !pool3 || !pool4 || !pool5 || !pool6)
      return;

    pool1->forEach([&](EntityId entity, T1 &c1) {
      auto *c2 = pool2->get(entity);
      auto *c3 = pool3->get(entity);
      auto *c4 = pool4->get(entity);
      auto *c5 = pool5->get(entity);
      auto *c6 = pool6->get(entity);
      if (c2 && c3 && c4 && c5 && c6 && c2->enabled && c3->enabled && 
          c4->enabled && c5->enabled && c6->enabled) {
        func(entity, c1, *c2, *c3, *c4, *c5, *c6);
      }
    });
  }

  // ==================== 系统管理 ====================

  /**
   * @brief 添加系统
   * @tparam T 系统类型
   * @param args 构造参数
   * @return 系统引用
   */
  template <typename T, typename... Args> T &addSystem(Args &&...args) {
    static_assert(std::is_base_of<ISystem, T>::value,
                  "System must inherit from ISystem");

    auto system = std::make_unique<T>(std::forward<Args>(args)...);
    T &ref = *system;
    system->init(this);

    // 按优先级插入
    auto it = std::lower_bound(_systems.begin(), _systems.end(), system,
                               [](const auto &a, const auto &b) {
                                 return a->getPriority() < b->getPriority();
                               });
    _systems.insert(it, std::move(system));

    return ref;
  }

  /**
   * @brief 获取系统
   */
  template <typename T> T *getSystem() {
    for (auto &system : _systems) {
      if (auto *ptr = dynamic_cast<T *>(system.get())) {
        return ptr;
      }
    }
    return nullptr;
  }

  /**
   * @brief 移除系统
   */
  template <typename T> void removeSystem() {
    _systems.erase(std::remove_if(_systems.begin(), _systems.end(),
                                  [](const auto &system) {
                                    return dynamic_cast<T *>(system.get()) !=
                                           nullptr;
                                  }),
                   _systems.end());
  }

  // ==================== 更新循环 ====================

  /**
   * @brief 每帧更新 - 调用所有系统的update
   */
  void update(float delta) {
    // 处理待销毁的实体
    processPendingDestructions();

    // 更新所有系统
    for (auto &system : _systems) {
      if (system->isEnabled()) {
        system->update(delta);
      }
    }
  }

  /**
   * @brief 固定时间步更新
   */
  void fixedUpdate(float fixedDelta) {
    for (auto &system : _systems) {
      if (system->isEnabled()) {
        system->fixedUpdate(fixedDelta);
      }
    }
  }

  /**
   * @brief 延迟更新
   */
  void lateUpdate(float delta) {
    for (auto &system : _systems) {
      if (system->isEnabled()) {
        system->lateUpdate(delta);
      }
    }
  }

  /**
   * @brief 关闭世界
   */
  void shutdown() {
    // 关闭所有系统
    for (auto &system : _systems) {
      system->shutdown();
    }
    _systems.clear();

    // 清理所有组件池
    _componentPools.clear();

    // 清理实体
    _activeEntities.clear();
    _entityTags.clear();
    _tagToEntities.clear();
    while (!_recycledIds.empty())
      _recycledIds.pop();
    _entitiesToDestroy.clear();
  }

  // ==================== 事件系统 ====================

  /**
   * @brief 订阅事件
   */
  template <typename EventType, typename Func> void subscribe(Func &&handler) {
    auto typeIndex = std::type_index(typeid(EventType));
    _eventHandlers[typeIndex].push_back(
        [handler = std::forward<Func>(handler)](const IEvent &event) {
          handler(static_cast<const EventType &>(event));
        });
  }

  /**
   * @brief 发送事件
   */
  template <typename EventType> void emit(const EventType &event) {
    auto typeIndex = std::type_index(typeid(EventType));
    auto it = _eventHandlers.find(typeIndex);
    if (it != _eventHandlers.end()) {
      for (auto &handler : it->second) {
        handler(event);
      }
    }
  }

private:
  template <typename T> ComponentPool<T> &getOrCreatePool() {
    ComponentTypeId typeId = getComponentTypeId<T>();
    auto it = _componentPools.find(typeId);
    if (it == _componentPools.end()) {
      auto pool = std::make_unique<ComponentPool<T>>();
      auto &ref = *pool;
      _componentPools[typeId] = std::move(pool);
      return ref;
    }
    return *static_cast<ComponentPool<T> *>(it->second.get());
  }

  void processPendingDestructions() {
    for (EntityId entity : _entitiesToDestroy) {
      destroyEntityImmediate(entity);
    }
    _entitiesToDestroy.clear();
  }

private:
  // 实体管理
  EntityId _nextEntityId = 0;
  std::unordered_set<EntityId> _activeEntities;
  std::queue<EntityId> _recycledIds;
  std::vector<EntityId> _entitiesToDestroy;

  // 标签系统
  std::unordered_map<EntityId, std::string> _entityTags;
  std::unordered_map<std::string, std::unordered_set<EntityId>> _tagToEntities;

  // 组件存储
  std::unordered_map<ComponentTypeId, std::unique_ptr<IComponentPool>>
      _componentPools;

  // 系统列表 (按优先级排序)
  std::vector<std::unique_ptr<ISystem>> _systems;

  // 事件处理
  std::unordered_map<std::type_index,
                     std::vector<std::function<void(const IEvent &)>>>
      _eventHandlers;
};

} // namespace ecs

// ==================== EntityHandle 模板实现 ====================
// 必须在 World 类定义之后实现

namespace ecs {

inline bool EntityHandle::valid() const {
  return _world && _world->isValid(_id);
}

template <typename T, typename... Args>
EntityHandle &EntityHandle::add(Args &&...args) {
  if (_world && valid()) {
    _world->addComponent<T>(_id, std::forward<Args>(args)...);
  }
  return *this;
}

template <typename T> T *EntityHandle::get() {
  return _world ? _world->getComponent<T>(_id) : nullptr;
}

template <typename T> const T *EntityHandle::get() const {
  return _world ? _world->getComponent<T>(_id) : nullptr;
}

template <typename T> bool EntityHandle::has() const {
  return _world && _world->hasComponent<T>(_id);
}

template <typename... Ts> bool EntityHandle::hasAll() const {
  return _world && _world->hasComponents<Ts...>(_id);
}

template <typename T> EntityHandle &EntityHandle::remove() {
  if (_world) {
    _world->removeComponent<T>(_id);
  }
  return *this;
}

template <typename T, typename Func>
EntityHandle &EntityHandle::ifHas(Func &&func) {
  if (auto *comp = get<T>()) {
    func(*comp);
  }
  return *this;
}

inline EntityHandle &EntityHandle::setTag(const std::string &tag) {
  if (_world) {
    _world->setTag(_id, tag);
  }
  return *this;
}

inline const std::string &EntityHandle::getTag() const {
  static const std::string empty;
  return _world ? _world->getTag(_id) : empty;
}

inline void EntityHandle::destroy() {
  if (_world) {
    _world->destroyEntity(_id);
  }
}

inline void EntityHandle::destroyImmediate() {
  if (_world) {
    _world->destroyEntityImmediate(_id);
  }
}

// ==================== EntityBuilder 实现 ====================

inline EntityBuilder::EntityBuilder(World &world) : _world(world) {
  _entity = _world.createEntity();
}

inline EntityBuilder &EntityBuilder::withTag(const std::string &tag) {
  _tag = tag;
  _world.setTag(_entity, tag);
  return *this;
}

template <typename T, typename... Args>
EntityBuilder &EntityBuilder::with(Args &&...args) {
  _world.addComponent<T>(_entity, std::forward<Args>(args)...);
  return *this;
}

template <typename T, typename Func>
EntityBuilder &EntityBuilder::configure(Func &&configurator) {
  auto &comp = _world.addComponent<T>(_entity);
  configurator(comp);
  return *this;
}

template <typename T, typename... Args>
EntityBuilder &EntityBuilder::withIf(bool condition, Args &&...args) {
  if (condition) {
    _world.addComponent<T>(_entity, std::forward<Args>(args)...);
  }
  return *this;
}

template <typename T>
EntityBuilder &EntityBuilder::copyFrom(EntityHandle source) {
  if (auto *srcComp = source.get<T>()) {
    _world.addComponent<T>(_entity, *srcComp);
  }
  return *this;
}

template <typename Func> EntityBuilder &EntityBuilder::apply(Func &&preset) {
  preset(*this);
  return *this;
}

inline EntityHandle EntityBuilder::build() {
  return EntityHandle(&_world, _entity);
}

inline EntityId EntityBuilder::buildId() { return _entity; }

} // namespace ecs

#endif // __ECS_WORLD_H__
