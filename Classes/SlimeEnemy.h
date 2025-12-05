#ifndef __SLIME_ENEMY_H__
#define __SLIME_ENEMY_H__

#include "Enemy.h"

/**
 * @class SlimeEnemy
 * @brief 史莱姆敌人 - Enemy类的具体实现示例
 *
 * 展示如何继承Enemy基类创建具体的敌人类型
 */
class SlimeEnemy : public Enemy {
public:
  /**
   * 创建史莱姆实例
   */
  static SlimeEnemy *create();

  /**
   * 初始化史莱姆（子类不应重写此方法）
   * @param slimeType 史莱姆类型（如"Green"、"Blue"）
   */
  bool init(const std::string &slimeType);

  /**
   * 每帧更新 - 包含自动索敌逻辑
   */
  virtual void update(float delta) override;

  /**
   * 执行跳跃 - 公开函数，供测试调用
   */
  void performJump(const Vec2 &direction);

  /**
   * 检查是否在地面上
   */
  bool isOnGround() const { return _isOnGround; }

protected:
  SlimeEnemy();
  virtual ~SlimeEnemy() = default;

  /**
   * 重写空闲行为 - 随机左右跳跃
   */
  virtual void updateIdle(float delta) override;

  /**
   * 重写巡逻行为 - 随机左右跳跃
   */
  virtual void updatePatrol(float delta) override;

  /**
   * 重写攻击行为 - 史莱姆跳跃攻击
   */
  virtual void updateAttack(float delta) override;

  /**
   * 重写追击行为 - 史莱姆跳跃移动
   */
  virtual void updateChase(float delta) override;

  /**
   * 初始化帧动画
   */
  void initAnimation(const std::string &slimeType);

  /**
   * 设置物理体和碰撞检测（由子类调用）
   */
  void setupPhysics();

  /**
   * 执行智能跳跃（有目标时调整力度）
   */
  void performSmartJump(const Vec2 &targetPos);

  /**
   * 执行随机跳跃（无目标时）
   */
  void performRandomJump();

  /**
   * 检查是否在地面上且静止（可以开始计时冷却）
   */
  bool isGroundedAndStill() const;

  bool _isOnGround;         // 是否在地面上
  float _jumpCooldown;      // 跳跃冷却时间
  float _jumpCooldownTimer; // 跳跃冷却计时器

  // 仇恨系统
  float _aggroRange;   // 仇恨距离（超过此距离就失去仇恨）
  float _deaggroRange; // 脱离仇恨距离（稍大于仇恨距离，防止频繁切换）

  // 最大跳跃力度（默认值）
  float _maxHorizontalImpulse; // 最大水平冲量
  float _maxVerticalImpulse;   // 最大垂直冲量

  int _randomDirection; // 随机方向（1或-1）
};

#endif // __SLIME_ENEMY_H__
