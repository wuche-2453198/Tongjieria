#ifndef __ECS_COMPONENT_PROJECTILESPRITECOMPONENT_H__
#define __ECS_COMPONENT_PROJECTILESPRITECOMPONENT_H__

#include "cocos2d.h"

namespace ecs {

struct ProjectileSpriteComponent
{
  cocos2d::Sprite *sprite = nullptr;
  float rotation = 0.0f;

  ProjectileSpriteComponent() = default;

  ~ProjectileSpriteComponent()
  {
    if (sprite)
    {
      sprite->stopAllActions();
      sprite->removeFromParent();
      sprite->release();
      sprite = nullptr;
    }
  }

  ProjectileSpriteComponent(const ProjectileSpriteComponent &) = delete;
  ProjectileSpriteComponent &operator=(const ProjectileSpriteComponent &) = delete;

  ProjectileSpriteComponent(ProjectileSpriteComponent &&other) noexcept
      : sprite(other.sprite), rotation(other.rotation)
  {
    other.sprite = nullptr;
  }

  ProjectileSpriteComponent &operator=(ProjectileSpriteComponent &&other) noexcept
  {
    if (this != &other)
    {
      if (sprite)
      {
        sprite->stopAllActions();
        sprite->removeFromParent();
        sprite->release();
      }
      sprite = other.sprite;
      rotation = other.rotation;
      other.sprite = nullptr;
    }
    return *this;
  }
};

} // namespace ecs

#endif // __ECS_COMPONENT_PROJECTILESPRITECOMPONENT_H__
