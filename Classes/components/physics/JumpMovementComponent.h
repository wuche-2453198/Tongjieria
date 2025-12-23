#ifndef __ECS_COMPONENT_JUMPMOVEMENTCOMPONENT_H__
#define __ECS_COMPONENT_JUMPMOVEMENTCOMPONENT_H__

#include "cocos2d.h"
#include <cstdlib>

namespace ecs {

/**
 * @brief 跳跃移动组件 - 管理跳跃式移动逻辑
 *
 * 特性:
 * - 跳跃冷却计时
 * - 冲量力度控制
 * - 随机/追踪方向
 */
struct JumpMovementComponent
{
  // 跳跃冷却
  float jumpCooldown = 2.0f; // 跳跃冷却时间
  float jumpTimer = 0.0f;    // 跳跃计时器
  bool readyToJump = false;  // 是否准备好跳跃

  // 跳跃力度（最大值）
  float maxHorizontalImpulse = 300.0f; // 最大水平冲量
  float maxVerticalImpulse = 500.0f;   // 最大垂直冲量

  // 随机跳跃参数
  int randomDirection = 1;                  // 随机方向 (1 或 -1)
  float randomDirectionChangeChance = 0.3f; // 改变方向的概率
  float patrolImpulseRatio = 0.5f;          // 巡逻时的冲量比例

  // 追踪跳跃参数
  bool isChasing = false; // 是否正在追踪

  // 最后一次跳跃的冲量
  cocos2d::Vec2 lastJumpImpulse = cocos2d::Vec2::ZERO;

  JumpMovementComponent() = default;
  JumpMovementComponent(float cooldown, float hImpulse, float vImpulse)
      : jumpCooldown(cooldown), jumpTimer(cooldown),
        maxHorizontalImpulse(hImpulse), maxVerticalImpulse(vImpulse)
  {
    randomDirection = (rand() % 2 == 0) ? 1 : -1;
  }

  // 计算追踪跳跃冲量
  cocos2d::Vec2 calculateChaseImpulse(float targetDirX, float heightDiff,
                                      float targetDist)
  {
    cocos2d::Vec2 impulse;

    // 水平方向：始终指向目标
    float dirX = targetDirX > 0 ? 1.0f : -1.0f;
    impulse.x = maxHorizontalImpulse * dirX;

    // 垂直方向：直接使用maxVerticalImpulse（已包含大跳倍率）
    impulse.y = maxVerticalImpulse;

    return impulse;
  }

  // 计算随机跳跃冲量（巡逻用）
  cocos2d::Vec2 calculateRandomImpulse()
  {
    if ((float)rand() / RAND_MAX < randomDirectionChangeChance)
    {
      randomDirection *= -1;
    }

    cocos2d::Vec2 impulse;
    impulse.x = maxHorizontalImpulse * patrolImpulseRatio * randomDirection;
    impulse.y = maxVerticalImpulse * 0.8f;

    return impulse;
  }
};

} // namespace ecs

#endif // __ECS_COMPONENT_JUMPMOVEMENTCOMPONENT_H__
