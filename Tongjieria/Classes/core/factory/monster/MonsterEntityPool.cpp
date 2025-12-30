#include "MonsterEntityPool.h"
#include "components/AllComponents.h"

void MonsterEntityPool::preallocate(entt::registry &registry, size_t count) {
  for (size_t i = 0; i < count; ++i) {
    auto entity = registry.create();
    ecs::EntityId id = entt::to_integral(entity);
    _free.push(id);
    _stats.totalAllocated++;
  }
  _stats.available = _free.size();
}

ecs::EntityId MonsterEntityPool::acquire(entt::registry &registry) {
  ecs::EntityId id = ecs::INVALID_ENTITY;
  if (!_free.empty()) {
    id = _free.front();
    _free.pop();
  } else {
    auto entity = registry.create();
    id = entt::to_integral(entity);
    _stats.totalAllocated++;
  }

  _stats.inUse++;
  _stats.available = _free.size();
  return id;
}

void MonsterEntityPool::release(entt::registry &registry, ecs::EntityId id) {
  if (id == ecs::INVALID_ENTITY) {
    return;
  }
  auto entity = static_cast<entt::entity>(id);
  if (!registry.valid(entity)) {
    return;
  }

  // 销毁旧实体，创建一个干净的新实体放回池中
  registry.destroy(entity);
  auto newEntity = registry.create();
  ecs::EntityId newId = entt::to_integral(newEntity);

  _free.push(newId);

  if (_stats.inUse > 0) {
    _stats.inUse--;
  }
  _stats.available = _free.size();
  // totalAllocated 不变（销毁+新建保持数量稳定）
}

MonsterEntityPool::Statistics MonsterEntityPool::getStatistics() const {
  MonsterEntityPool::Statistics stats = _stats;
  stats.available = _free.size();
  return stats;
}

void MonsterEntityPool::resetEntity(entt::registry &registry, ecs::EntityId id) {
  // 复用 release + acquire 语义：回收并立即再取一个干净的实体
  release(registry, id);
}

void MonsterEntityPool::activateEntity(entt::registry &registry, ecs::EntityId id) {
  if (id == ecs::INVALID_ENTITY) {
    return;
  }
  auto entity = static_cast<entt::entity>(id);
  if (!registry.valid(entity)) {
    return;
  }

  if (auto *state = registry.try_get<ecs::EntityStateFlags>(entity)) {
    state->isActive = true;
    state->isOnScreen = true;
  }
}

void MonsterEntityPool::deactivateEntity(entt::registry &registry, ecs::EntityId id) {
  if (id == ecs::INVALID_ENTITY) {
    return;
  }
  auto entity = static_cast<entt::entity>(id);
  if (!registry.valid(entity)) {
    return;
  }

  if (auto *state = registry.try_get<ecs::EntityStateFlags>(entity)) {
    state->isActive = false;
  }
}
