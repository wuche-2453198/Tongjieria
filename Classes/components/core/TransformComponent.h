#ifndef __ECS_COMPONENT_TRANSFORMCOMPONENT_H__
#define __ECS_COMPONENT_TRANSFORMCOMPONENT_H__

#include "components/Entity.h"
#include "cocos2d.h"

namespace ecs {

/**
 * @brief 变换组件 - 位置、旋转、缩放
 */
struct TransformComponent
{
  cocos2d::Vec2 position = cocos2d::Vec2::ZERO;
  float rotation = 0.0f; // 角度
  cocos2d::Vec2 scale = cocos2d::Vec2(1.0f, 1.0f);

  // 速度 (用于运动系统)
  cocos2d::Vec2 velocity = cocos2d::Vec2::ZERO;

  // 父实体 (用于层级关系)
  EntityId parent = INVALID_ENTITY;

  TransformComponent() = default;
  TransformComponent(float x, float y) : position(x, y) {}
  TransformComponent(const cocos2d::Vec2 &pos) : position(pos) {}
};

} // namespace ecs

#endif // __ECS_COMPONENT_TRANSFORMCOMPONENT_H__
