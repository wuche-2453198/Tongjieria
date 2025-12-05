#ifndef __ENEMY_H__
#define __ENEMY_H__

#include "cocos2d.h"

USING_NS_CC;

/**
 * @class Enemy
 * @brief 泰拉瑞亚风格的敌人基类框架
 *
 * 提供敌人的基础属性、AI行为、战斗系统等功能
 * 所有具体的敌人类型应继承此类并实现特定行为
 */
class Enemy : public Sprite {
public:
  /**
   * 敌人状态枚举
   */
  enum class State {
    IDLE,   // 空闲状态
    PATROL, // 巡逻状态
    CHASE,  // 追击状态
    ATTACK, // 攻击状态
    HURT,   // 受伤状态
    DEAD    // 死亡状态
  };

  /**
   * 敌人类型枚举 (可扩展)
   */
  enum class EnemyType {
    SLIME,      // 史莱姆
    ZOMBIE,     // 僵尸
    SKELETON,   // 骷髅
    FLYING_EYE, // 飞眼
                // 可继续添加其他类型
  };

  /**
   * 掉落物品结构
   */
  struct DropItem {
    std::string itemId; // 物品ID
    int minCount;       // 最小掉落数量
    int maxCount;       // 最大掉落数量
    float dropChance;   // 掉落概率 (0.0 - 1.0)

    DropItem(const std::string &id, int min, int max, float chance)
        : itemId(id), minCount(min), maxCount(max), dropChance(chance) {}
  };

public:
  /**
   * 创建敌人实例
   * @param filename 精灵图片路径
   * @param type 敌人类型
   * @return 敌人实例指针
   */
  static Enemy *create(const std::string &filename, EnemyType type);

  /**
   * 初始化敌人
   * @param type 敌人类型
   * @return 初始化是否成功
   */
  virtual bool init(EnemyType type);

  /**
   * 每帧更新函数
   * @param delta 时间间隔
   */
  virtual void update(float delta) override;

  // ==================== 属性设置/获取 ====================

  /**
   * 设置最大生命值
   */
  void setMaxHealth(float maxHP);
  float getMaxHealth() const { return _maxHealth; }

  /**
   * 设置当前生命值
   */
  void setCurrentHealth(float hp);
  float getCurrentHealth() const { return _currentHealth; }

  /**
   * 设置移动速度
   */
  void setMoveSpeed(float speed) { _moveSpeed = speed; }
  float getMoveSpeed() const { return _moveSpeed; }

  /**
   * 设置攻击力
   */
  void setAttackDamage(float damage) { _attackDamage = damage; }
  float getAttackDamage() const { return _attackDamage; }

  /**
   * 设置防御力
   */
  void setDefense(float defense) { _defense = defense; }
  float getDefense() const { return _defense; }

  /**
   * 设置击退抗性 (0-1, 1表示完全免疫击退)
   */
  void setKnockbackResist(float resist) { _knockbackResist = resist; }
  float getKnockbackResist() const { return _knockbackResist; }

  /**
   * 设置检测范围
   */
  void setDetectionRange(float range) { _detectionRange = range; }
  float getDetectionRange() const { return _detectionRange; }

  /**
   * 设置攻击范围
   */
  void setAttackRange(float range) { _attackRange = range; }
  float getAttackRange() const { return _attackRange; }

  // ==================== 战斗系统 ====================

  /**
   * 受到伤害
   * @param damage 伤害值
   * @param knockbackDirection 击退方向
   * @param knockbackForce 击退力度
   */
  virtual void takeDamage(float damage, const Vec2 &knockbackDirection,
                          float knockbackForce);

  /**
   * 治疗
   * @param healAmount 治疗量
   */
  void heal(float healAmount);

  /**
   * 是否存活
   */
  bool isAlive() const { return _currentHealth > 0; }

  /**
   * 死亡处理
   */
  virtual void die();

  // ==================== AI系统 ====================

  /**
   * 设置玩家目标
   */
  void setPlayerTarget(Node *player) { _playerTarget = player; }

  /**
   * 获取当前状态
   */
  State getCurrentState() const { return _currentState; }

  /**
   * 切换状态
   */
  virtual void changeState(State newState);

  // ==================== 掉落系统 ====================

  /**
   * 添加掉落物品
   */
  void addDropItem(const std::string &itemId, int minCount, int maxCount,
                   float dropChance);

  /**
   * 生成掉落物
   */
  virtual void spawnDrops();

  // ==================== 动画系统 ====================

  /**
   * 播放动画
   * @param animName 动画名称
   * @param loop 是否循环
   */
  virtual void playAnimation(const std::string &animName, bool loop = true);

  /**
   * 设置面向方向 (true = 右, false = 左)
   */
  void setFacingRight(bool facingRight);
  bool isFacingRight() const { return _facingRight; }

protected:
  Enemy();
  virtual ~Enemy();

  // ==================== AI行为函数 (子类可重写) ====================

  /**
   * 空闲行为
   */
  virtual void updateIdle(float delta);

  /**
   * 巡逻行为
   */
  virtual void updatePatrol(float delta);

  /**
   * 追击行为
   */
  virtual void updateChase(float delta);

  /**
   * 攻击行为
   */
  virtual void updateAttack(float delta);

  /**
   * 受伤行为
   */
  virtual void updateHurt(float delta);

  /**
   * 检测玩家
   * @return 玩家是否在检测范围内
   */
  virtual bool detectPlayer();

  /**
   * 移动向目标
   */
  virtual void moveTowards(const Vec2 &target, float delta);

  /**
   * 应用击退效果
   */
  virtual void applyKnockback(const Vec2 &direction, float force);

  /**
   * 更新血条显示
   */
  virtual void updateHealthBar();

  /**
   * 初始化血条
   */
  virtual void initHealthBar();

protected:
  // ==================== 基础属性 ====================
  EnemyType _enemyType; // 敌人类型
  State _currentState;  // 当前状态

  float _maxHealth;       // 最大生命值
  float _currentHealth;   // 当前生命值
  float _moveSpeed;       // 移动速度
  float _attackDamage;    // 攻击力
  float _defense;         // 防御力
  float _knockbackResist; // 击退抗性

  float _detectionRange;      // 检测范围
  float _attackRange;         // 攻击范围
  float _attackCooldown;      // 攻击冷却时间
  float _attackCooldownTimer; // 攻击冷却计时器

  // ==================== AI相关 ====================
  Node *_playerTarget;   // 玩家目标
  Vec2 _velocity;        // 速度向量
  Vec2 _patrolStartPos;  // 巡逻起始位置
  float _patrolDistance; // 巡逻距离
  int _patrolDirection;  // 巡逻方向 (1 或 -1)

  // ==================== 动画与渲染 ====================
  bool _facingRight; // 是否面向右边

  // ==================== 血条 ====================
  Sprite *_healthBarBg;   // 血条背景
  Sprite *_healthBarFill; // 血条填充

  // ==================== 掉落物 ====================
  std::vector<DropItem> _dropTable; // 掉落表

  // ==================== 状态计时器 ====================
  float _hurtStateTimer;    // 受伤状态计时器
  float _hurtStateDuration; // 受伤状态持续时间

  // ==================== 其他 ====================
  bool _isInvincible;        // 是否无敌（受伤后短暂无敌）
  float _invincibleTimer;    // 无敌计时器
  float _invincibleDuration; // 无敌持续时间
};

#endif // __ENEMY_H__
