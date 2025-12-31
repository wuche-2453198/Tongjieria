#ifndef __ECS_COMPONENT_WARRIORMOVEMENTCOMPONENT_H__
#define __ECS_COMPONENT_WARRIORMOVEMENTCOMPONENT_H__

#include "cocos2d.h"
#include <cstdlib>
#include <cmath>

namespace ecs {

/**
 * @brief 战士移动组件 - 僵尸等持续行走的怪物（战士AI）
 *
 * 行为特点：
 * - 行走追踪玩家
 * - 跳过洞和障碍物
 * - 尝试垂直对齐目标高度
 * - 追击失败时后退重试
 */
struct WarriorMovementComponent
{
  float walkSpeed = 80.0f;
  float jumpForce = 450.0f;

  float jumpCooldown = 0.2f;
  float jumpCooldownTimer = 0.0f;
  bool jumpCooldownActive = false;

  bool obstacleJumpEnabled = true;
  float obstacleCheckDistance = 20.0f;
  float obstacleJumpCooldown = 1.0f;
  float obstacleJumpTimer = 0.0f;
  cocos2d::Vec2 stuckCheckStartPos = cocos2d::Vec2(-9999, -9999);
  float stuckTime = 0.0f;
  float stuckThreshold = 0.5f;
  float stuckDistanceRatio = 0.2f;

  bool useInstantObstacleDetection = false;
  float expectedSpeed = 0.0f;
  float actualSpeedRatio = 0.2f;
  float initDelay = 1.0f;
  float initTimer = 0.0f;
  bool initialized = false;

  bool targetJumpEnabled = true;
  float targetJumpReactionTime = 0.1f;
  float targetJumpReactionTimer = 0.0f;
  bool targetWasOnGround = true;
  float targetLastY = 0.0f;
  float targetHeightThreshold = 30.0f;
  float jumpDetectionRange = 150.0f;
  bool pendingReactionJump = false;

  int patrolDirection = 1;
  float patrolDirectionChangeInterval = 3.0f;
  float patrolTimer = 0.0f;
  float patrolDirectionChangeChance = 0.3f;

  bool isWalking = false;
  bool isJumping = false;
  int currentDirection = 1;

  // 战士AI新增：后退重试机制（只在跳跃失败时触发）
  bool isRetreating = false;
  float retreatTimer = 0.0f;
  float retreatDuration = 0.5f;
  float retreatCooldown = 3.0f;
  float retreatCooldownTimer = 0.0f;
  int jumpFailCount = 0;
  int jumpFailThreshold = 2;
  bool wasJumping = false;
  cocos2d::Vec2 preJumpPosition = cocos2d::Vec2(0, 0);

  // 战士AI新增：跳洞检测
  bool gapJumpEnabled = true;
  float gapDetectionRange = 50.0f;

  // 战士AI新增：垂直对齐
  bool verticalAlignEnabled = true;
  float verticalAlignThreshold = 20.0f;

  WarriorMovementComponent()
  {
    initDelay = 0.5f + (float)(rand() % 50) / 100.0f;
    patrolDirection = (rand() % 2 == 0) ? 1 : -1;
    currentDirection = patrolDirection;
  }

  WarriorMovementComponent(float speed, float jump) : walkSpeed(speed), jumpForce(jump)
  {
    initDelay = 0.5f + (float)(rand() % 50) / 100.0f;
    patrolDirection = (rand() % 2 == 0) ? 1 : -1;
    currentDirection = patrolDirection;
  }

  bool canJump() const { return jumpCooldownTimer <= 0; }

  void onJump()
  {
    jumpCooldownTimer = jumpCooldown;
    jumpCooldownActive = false;
  }

  void onLand()
  {
    if (jumpCooldownTimer > 0 && !jumpCooldownActive)
      jumpCooldownActive = true;
  }

  void updateJumpCooldown(float delta)
  {
    if (jumpCooldownActive && jumpCooldownTimer > 0)
      jumpCooldownTimer -= delta;
  }

  void startRetreat()
  {
    if (retreatCooldownTimer <= 0 && !isRetreating)
    {
      isRetreating = true;
      retreatTimer = retreatDuration;
      retreatCooldownTimer = retreatCooldown;
      jumpFailCount = 0;
    }
  }

  void updateRetreat(float delta)
  {
    if (retreatCooldownTimer > 0)
      retreatCooldownTimer -= delta;
    if (isRetreating)
    {
      retreatTimer -= delta;
      if (retreatTimer <= 0)
      {
        isRetreating = false;
      }
    }
  }

  void onJumpStart(const cocos2d::Vec2 &pos)
  {
    preJumpPosition = pos;
  }

  bool checkJumpFailed(const cocos2d::Vec2 &currentPos)
  {
    float horizontalMove = std::abs(currentPos.x - preJumpPosition.x);
    return horizontalMove < 20.0f;
  }

  void onJumpFailed()
  {
    jumpFailCount++;
  }

  void onJumpSuccess()
  {
    jumpFailCount = 0;
  }

  bool shouldRetreat() const
  {
    return jumpFailCount >= jumpFailThreshold && retreatCooldownTimer <= 0;
  }

  bool shouldObstacleJump(const cocos2d::Vec2 &currentPos, float delta)
  {
    if (!obstacleJumpEnabled || obstacleJumpTimer > 0 || !canJump())
      return false;

    if (!initialized)
    {
      initTimer += delta;
      if (initTimer >= initDelay)
      {
        initialized = true;
        stuckCheckStartPos = currentPos;
        stuckTime = 0.0f;
      }
      return false;
    }

    if (!isWalking)
    {
      stuckCheckStartPos = currentPos;
      stuckTime = 0.0f;
      return false;
    }

    stuckTime += delta;

    if (stuckTime >= stuckThreshold)
    {
      float actualDist = std::abs(currentPos.x - stuckCheckStartPos.x);
      float expectedDist = walkSpeed * stuckTime;
      stuckCheckStartPos = currentPos;
      stuckTime = 0.0f;

      if (expectedDist > 5.0f && actualDist < expectedDist * stuckDistanceRatio)
      {
        return true;
      }
    }
    return false;
  }
};

// 为了向后兼容，保留别名
using WalkMovementComponent = WarriorMovementComponent;

} // namespace ecs

#endif // __ECS_COMPONENT_WARRIORMOVEMENTCOMPONENT_H__
