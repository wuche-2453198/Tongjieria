#ifndef __ECS_COMPONENT_DEMONEYEMOVEMENTCOMPONENT_H__
#define __ECS_COMPONENT_DEMONEYEMOVEMENTCOMPONENT_H__

#include "cocos2d.h"
#include <cstdlib>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace ecs {

/**
 * @brief 恶魔眼移动组件 - 飞行追踪类怪物
 *
 * 行为特点：
 * - 飞行（无重力）
 * - 缓慢转向追踪玩家
 * - 撞墙/物块时弧形回弹
 * - 被击退时弧形轨迹回弹
 */
struct DemonEyeMovementComponent
{
  // 飞行速度配置
  float flySpeed = 150.0f;
  float maxSpeed = 400.0f;
  float acceleration = 200.0f;

  // AI状态
  enum State {
    HOVERING, // 盘旋/寻找高度
    DASHING   // 俯冲攻击
  };
  State aiState = HOVERING;
  float aiTimer = 0.0f;

  // 转向配置
  float turnRate = 2.5f;

  // 当前运动状态
  cocos2d::Vec2 currentVelocity = cocos2d::Vec2::ZERO;
  float currentAngle = 0.0f;
  float targetAngle = 0.0f;

  // 巡逻配置（无目标时）
  float patrolChangeInterval = 2.0f;
  float patrolTimer = 0.0f;
  float patrolAngle = 0.0f;
  
  // DASHING冷却（防止状态快速循环）
  float dashCooldown = 2.0f;
  float dashCooldownTimer = 0.0f;

  // 抖动/摆动效果（使飞行更自然）
  float wobbleAmplitude = 0.3f;
  float wobbleFrequency = 2.0f;
  float wobblePhase = 0.0f;

  DemonEyeMovementComponent()
  {
    // 随机初始朝向
    currentAngle = ((float)rand() / RAND_MAX) * 2 * M_PI;
    patrolAngle = currentAngle;
    wobblePhase = ((float)rand() / RAND_MAX) * 2 * M_PI;
  }

  DemonEyeMovementComponent(float speed, float turn)
      : flySpeed(speed), turnRate(turn)
  {
    currentAngle = ((float)rand() / RAND_MAX) * 2 * M_PI;
    patrolAngle = currentAngle;
    wobblePhase = ((float)rand() / RAND_MAX) * 2 * M_PI;
  }

  /**
   * @brief 计算角度差（考虑角度环绕）
   */
  static float angleDifference(float target, float current)
  {
    float diff = target - current;
    while (diff > M_PI)
      diff -= 2 * M_PI;
    while (diff < -M_PI)
      diff += 2 * M_PI;
    return diff;
  }

  /**
   * @brief 平滑转向到目标角度
   * @param delta 帧时间
   * @return 新的当前角度
   */
  float smoothTurn(float delta)
  {
    float diff = angleDifference(targetAngle, currentAngle);
    float maxTurn = turnRate * delta;

    if (std::abs(diff) <= maxTurn)
    {
      currentAngle = targetAngle;
    }
    else
    {
      currentAngle += (diff > 0 ? maxTurn : -maxTurn);
    }

    // 标准化角度到 [-PI, PI]
    while (currentAngle > M_PI)
      currentAngle -= 2 * M_PI;
    while (currentAngle < -M_PI)
      currentAngle += 2 * M_PI;

    return currentAngle;
  }
};

} // namespace ecs

#endif // __ECS_COMPONENT_DEMONEYEMOVEMENTCOMPONENT_H__
