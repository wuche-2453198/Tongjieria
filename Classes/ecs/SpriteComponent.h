#ifndef __ECS_SPRITE_COMPONENT_H__
#define __ECS_SPRITE_COMPONENT_H__

#include "Component.h"
#include "cocos2d.h"
#include <string>

namespace ecs {

/**
 * @brief Node 到 Entity 的映射组件
 *
 * 用于在碰撞检测等场景中快速查找 Node 对应的 Entity
 */
struct NodeEntityMap {
  static NodeEntityMap &getInstance() {
    static NodeEntityMap instance;
    return instance;
  }

  void registerNode(cocos2d::Node *node, EntityId entity) {
    if (node) {
      _nodeToEntity[node] = entity;
    }
  }

  void unregisterNode(cocos2d::Node *node) {
    if (node) {
      _nodeToEntity.erase(node);
    }
  }

  EntityId findEntity(cocos2d::Node *node) const {
    auto it = _nodeToEntity.find(node);
    return it != _nodeToEntity.end() ? it->second : INVALID_ENTITY;
  }

  void clear() { _nodeToEntity.clear(); }

private:
  NodeEntityMap() = default;
  std::unordered_map<cocos2d::Node *, EntityId> _nodeToEntity;
};

} // namespace ecs

#endif // __ECS_SPRITE_COMPONENT_H__
