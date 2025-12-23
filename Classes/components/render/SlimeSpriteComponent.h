#ifndef __ECS_COMPONENT_SLIMESPRITECOMPONENT_H__
#define __ECS_COMPONENT_SLIMESPRITECOMPONENT_H__

#include "cocos2d.h"
#include <string>

namespace ecs {

struct SlimeSpriteComponent
{
  cocos2d::Sprite *sprite = nullptr;
  cocos2d::Node *parentNode = nullptr;
  std::string slimeType = "Green";

  int zOrder = 0;
  bool visible = true;
  bool facingRight = true;
  float baseScale = 1.0f;
  cocos2d::Color3B color = cocos2d::Color3B::WHITE;
  uint8_t opacity = 255;

  cocos2d::Vector<cocos2d::SpriteFrame *> animFrames;
  float frameTime = 0.15f;
  bool animationLoaded = false;
  int currentFrameIndex = 0;
  float frameTimer = 0.0f;

  SlimeSpriteComponent() = default;
  SlimeSpriteComponent(const std::string &type) : slimeType(type) { initSprite(); }

  ~SlimeSpriteComponent()
  {
    if (sprite)
    {
      sprite->stopAllActions();
      sprite->removeFromParent();
      sprite->release();
      sprite = nullptr;
    }
  }

  SlimeSpriteComponent(const SlimeSpriteComponent &) = delete;
  SlimeSpriteComponent &operator=(const SlimeSpriteComponent &) = delete;

  SlimeSpriteComponent(SlimeSpriteComponent &&other) noexcept
      : sprite(other.sprite), parentNode(other.parentNode),
        slimeType(std::move(other.slimeType)), zOrder(other.zOrder),
        visible(other.visible), facingRight(other.facingRight),
        baseScale(other.baseScale), color(other.color), opacity(other.opacity),
        animFrames(std::move(other.animFrames)), frameTime(other.frameTime),
        animationLoaded(other.animationLoaded)
  {
    other.sprite = nullptr;
    other.parentNode = nullptr;
  }

  bool initSprite()
  {
    std::string firstFramePath = "Minor monster/" + slimeType + "Slime/" + slimeType + "_Slime1.png";
    sprite = cocos2d::Sprite::create(firstFramePath);
    if (sprite)
    {
      sprite->retain();
      return true;
    }
    sprite = cocos2d::Sprite::create();
    if (sprite)
    {
      sprite->retain();
      sprite->setTextureRect(cocos2d::Rect(0, 0, 40, 40));
      if (slimeType == "Green") sprite->setColor(cocos2d::Color3B::GREEN);
      else if (slimeType == "Blue") sprite->setColor(cocos2d::Color3B::BLUE);
      else if (slimeType == "Yellow") sprite->setColor(cocos2d::Color3B::YELLOW);
      return true;
    }
    return false;
  }

  void loadAnimation()
  {
    if (animationLoaded) return;
    animFrames.clear();
    for (int i = 1; i <= 2; i++)
    {
      std::string framePath = "Minor monster/" + slimeType + "Slime/" + slimeType + "_Slime" + std::to_string(i) + ".png";
      auto texture = cocos2d::Director::getInstance()->getTextureCache()->addImage(framePath);
      if (texture)
      {
        auto frame = cocos2d::SpriteFrame::createWithTexture(texture, cocos2d::Rect(0, 0, texture->getContentSize().width, texture->getContentSize().height));
        if (frame) animFrames.pushBack(frame);
      }
      else break;
    }
    animationLoaded = (animFrames.size() >= 2);
  }

  void playIdleAnimation()
  {
    if (!sprite || animFrames.size() < 2) return;
    sprite->stopAllActions();
    auto animation = cocos2d::Animation::createWithSpriteFrames(animFrames, frameTime);
    auto animate = cocos2d::Animate::create(animation);
    sprite->runAction(cocos2d::RepeatForever::create(animate));
  }

  void playJumpAnimation()
  {
    if (!sprite) return;
    float jumpDuration = 0.5f;
    auto jumpUp = cocos2d::ScaleBy::create(jumpDuration * 0.4f, 1.2f);
    auto jumpDown = cocos2d::ScaleBy::create(jumpDuration * 0.6f, 1.0f / 1.2f);
    auto scaleSeq = cocos2d::Sequence::create(jumpUp, jumpDown, nullptr);
    sprite->runAction(scaleSeq);
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

  void attachToNode(cocos2d::Node *parent, int z = 0)
  {
    if (sprite && parent)
    {
      parentNode = parent;
      zOrder = z;
      parent->addChild(sprite, z);
    }
  }

  void setupPhysicsBody(float size = 35.0f, int group = -1)
  {
    if (!sprite) return;
    cocos2d::PhysicsMaterial material(1.0f, 0.0f, 1.0f);
    auto body = cocos2d::PhysicsBody::createBox(cocos2d::Size(size, size), material);
    body->setDynamic(true);
    body->setMass(1.0f);
    body->setRotationEnable(false);
    body->setContactTestBitmask(0xFFFFFFFF);
    body->setCollisionBitmask(0xFFFFFFFF);
    body->setGroup(group);
    sprite->setPhysicsBody(body);
  }

  cocos2d::PhysicsBody *getPhysicsBody()
  {
    return sprite ? sprite->getPhysicsBody() : nullptr;
  }

  void applyImpulse(const cocos2d::Vec2 &impulse)
  {
    auto body = getPhysicsBody();
    if (body) body->applyImpulse(impulse);
  }

  cocos2d::Vec2 getVelocity()
  {
    auto body = getPhysicsBody();
    return body ? body->getVelocity() : cocos2d::Vec2::ZERO;
  }
};

} // namespace ecs

#endif // __ECS_COMPONENT_SLIMESPRITECOMPONENT_H__
