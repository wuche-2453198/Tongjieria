#ifndef __ECS_COMPONENT_EATEROFSOULSMOVEMENT_H__
#define __ECS_COMPONENT_EATEROFSOULSMOVEMENT_H__

#include "cocos2d.h"
#include <cstdlib>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace ecs {

/**
 * @brief 噬魂怪移动组件 - 飞行敌怪
 *
 * 行为特点：
 * - 飞行（无重力）
 * - 先在一定距离外绕着玩家转圈
 * - 周期性冲向玩家
 * - 三种不同大小的变体
 */
struct EaterOfSoulsMovementComponent
{
  // 尺寸变体
  enum SizeVariant {
    SMALL,   // 36*67
    MEDIUM,  // 42*78
    LARGE    // 47*89
  };
  SizeVariant sizeVariant = MEDIUM;

  // 飞行速度配置（比恶魔眼慢）
  float flySpeed = 80.0f;       // 降低基础飞行速度
  float maxSpeed = 200.0f;      // 降低最大速度
  float acceleration = 150.0f;  // 降低加速度

  // AI状态
  enum State {
    CIRCLING,  // 绕圈状态
    CHARGING   // 冲刺攻击
  };
  State aiState = CIRCLING;
  float aiTimer = 0.0f;

  // 绕圈行为配置（更靠近玩家）
  float circleRadius = 120.0f;        // 绕圈半径（更小，更靠近玩家）
  float circleSpeed = 1.2f;           // 绕圈角速度（弧度/秒）
  float circleAngle = 0.0f;           // 当前绕圈角度
  float circleRadiusVariation = 20.0f; // 半径变化幅度

  // 冲刺配置
  float chargeInterval = 2.5f;        // 冲刺间隔时间（更频繁）
  float chargeDuration = 1.2f;        // 冲刺持续时间
  float chargeSpeed = 280.0f;         // 冲刺速度（比恶魔眼慢）
  float chargeCooldown = 0.0f;        // 冲刺冷却计时器

  // 转向配置
  float turnRate = 3.0f;

  // 当前运动状态
  cocos2d::Vec2 currentVelocity = cocos2d::Vec2::ZERO;
  float currentAngle = 0.0f;
  float targetAngle = 0.0f;

  // 抖动/摆动效果（使飞行更自然）
  float wobbleAmplitude = 0.2f;
  float wobbleFrequency = 2.5f;
  float wobblePhase = 0.0f;

  EaterOfSoulsMovementComponent()
  {
    // 随机初始朝向
    currentAngle = ((float)rand() / RAND_MAX) * 2 * M_PI;
    circleAngle = ((float)rand() / RAND_MAX) * 2 * M_PI;
    wobblePhase = ((float)rand() / RAND_MAX) * 2 * M_PI;
    chargeCooldown = ((float)rand() / RAND_MAX) * chargeInterval;
  }

  EaterOfSoulsMovementComponent(SizeVariant variant)
      : sizeVariant(variant)
  {
    currentAngle = ((float)rand() / RAND_MAX) * 2 * M_PI;
    circleAngle = ((float)rand() / RAND_MAX) * 2 * M_PI;
    wobblePhase = ((float)rand() / RAND_MAX) * 2 * M_PI;
    chargeCooldown = ((float)rand() / RAND_MAX) * chargeInterval;

    // 根据尺寸调整属性（整体比恶魔眼慢）
    switch (variant) {
      case SMALL:
        flySpeed = 90.0f;
        maxSpeed = 220.0f;
        circleRadius = 100.0f;  // 更靠近玩家
        circleSpeed = 1.4f;
        chargeSpeed = 300.0f;
        break;
      case MEDIUM:
        flySpeed = 80.0f;
        maxSpeed = 200.0f;
        circleRadius = 120.0f;  // 更靠近玩家
        circleSpeed = 1.2f;
        chargeSpeed = 280.0f;
        break;
      case LARGE:
        flySpeed = 70.0f;
        maxSpeed = 180.0f;
        circleRadius = 140.0f;  // 更靠近玩家
        circleSpeed = 1.0f;
        chargeSpeed = 260.0f;
        break;
    }
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

#endif // __ECS_COMPONENT_EATEROFSOULSMOVEMENT_H__
