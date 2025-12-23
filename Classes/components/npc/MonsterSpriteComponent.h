#ifndef __ECS_COMPONENT_MONSTERSPRITECOMPONENT_H__
#define __ECS_COMPONENT_MONSTERSPRITECOMPONENT_H__

#include "cocos2d.h"
#include <string>
#include <vector>

namespace ecs {

struct MonsterSpriteComponent
{
  cocos2d::Sprite *sprite = nullptr;
  cocos2d::Node *parentNode = nullptr;
  std::string monsterType;

  int zOrder = 0;
  bool visible = true;
  bool facingRight = true;
  float baseScale = 1.0f;
  cocos2d::Color3B color = cocos2d::Color3B::WHITE;
  uint8_t opacity = 255;

  cocos2d::Vector<cocos2d::SpriteFrame *> animFrames;
  std::vector<int> frameSequence;
  float frameTime = 0.15f;
  bool animationLoaded = false;
  int currentFrameIndex = 0;
  float frameTimer = 0.0f;

  MonsterSpriteComponent() = default;

  ~MonsterSpriteComponent()
  {
    if (sprite)
    {
      sprite->stopAllActions();
      sprite->removeFromParent();
      sprite->release();
      sprite = nullptr;
    }
  }

  MonsterSpriteComponent(const MonsterSpriteComponent &) = delete;
  MonsterSpriteComponent &operator=(const MonsterSpriteComponent &) = delete;

  MonsterSpriteComponent(MonsterSpriteComponent &&other) noexcept
      : sprite(other.sprite), parentNode(other.parentNode),
        monsterType(std::move(other.monsterType)), zOrder(other.zOrder),
        visible(other.visible), facingRight(other.facingRight),
        baseScale(other.baseScale), color(other.color), opacity(other.opacity),
        animFrames(std::move(other.animFrames)),
        frameSequence(std::move(other.frameSequence)),
        frameTime(other.frameTime), animationLoaded(other.animationLoaded),
        currentFrameIndex(other.currentFrameIndex), frameTimer(other.frameTimer)
  {
    other.sprite = nullptr;
    other.parentNode = nullptr;
  }

  void setFacing(bool right)
  {
    facingRight = right;
    if (sprite) sprite->setScaleX(baseScale * (right ? 1.0f : -1.0f));
  }

  void syncPosition(const cocos2d::Vec2 &pos)
  {
    if (sprite) sprite->setPosition(pos);
  }

  void updateAnimation(float delta)
  {
    if (!animationLoaded || frameSequence.empty() || animFrames.empty()) return;
    frameTimer += delta;
    if (frameTimer >= frameTime)
    {
      frameTimer -= frameTime;
      currentFrameIndex = (currentFrameIndex + 1) % frameSequence.size();
      int frameIdx = frameSequence[currentFrameIndex] - 1;
      if (frameIdx >= 0 && frameIdx < (int)animFrames.size())
        sprite->setSpriteFrame(animFrames.at(frameIdx));
    }
  }

  cocos2d::PhysicsBody *getPhysicsBody()
  {
    return sprite ? sprite->getPhysicsBody() : nullptr;
  }

  cocos2d::Vec2 getVelocity()
  {
    auto body = getPhysicsBody();
    return body ? body->getVelocity() : cocos2d::Vec2::ZERO;
  }

  void setVelocity(const cocos2d::Vec2 &vel)
  {
    auto body = getPhysicsBody();
    if (body) body->setVelocity(vel);
  }

  void applyImpulse(const cocos2d::Vec2 &impulse)
  {
    auto body = getPhysicsBody();
    if (body) body->applyImpulse(impulse);
  }
};

} // namespace ecs

#endif // __ECS_COMPONENT_MONSTERSPRITECOMPONENT_H__
