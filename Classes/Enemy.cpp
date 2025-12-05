#include "Enemy.h"

Enemy::Enemy()
    : _enemyType(EnemyType::SLIME), _currentState(State::IDLE),
      _maxHealth(100.0f), _currentHealth(100.0f), _moveSpeed(100.0f),
      _attackDamage(10.0f), _defense(0.0f), _knockbackResist(0.0f),
      _detectionRange(300.0f), _attackRange(50.0f), _attackCooldown(1.0f),
      _attackCooldownTimer(0.0f), _playerTarget(nullptr), _velocity(Vec2::ZERO),
      _patrolStartPos(Vec2::ZERO), _patrolDistance(200.0f), _patrolDirection(1),
      _facingRight(true), _healthBarBg(nullptr), _healthBarFill(nullptr),
      _hurtStateTimer(0.0f), _hurtStateDuration(0.3f), _isInvincible(false),
      _invincibleTimer(0.0f), _invincibleDuration(0.5f) {}

Enemy::~Enemy() {}

Enemy *Enemy::create(const std::string &filename, EnemyType type) {
  Enemy *enemy = new (std::nothrow) Enemy();
  if (enemy && enemy->initWithFile(filename)) {
    enemy->init(type);
    enemy->autorelease();
    return enemy;
  }
  CC_SAFE_DELETE(enemy);
  return nullptr;
}

bool Enemy::init(EnemyType type) {
  _enemyType = type;

  // 根据敌人类型设置默认属性
  switch (_enemyType) {
  case EnemyType::SLIME:
    setMaxHealth(50.0f);
    setMoveSpeed(80.0f);
    setAttackDamage(8.0f);
    setDefense(2.0f);
    setKnockbackResist(0.1f);
    setDetectionRange(250.0f);
    setAttackRange(40.0f);
    break;

  case EnemyType::ZOMBIE:
    setMaxHealth(80.0f);
    setMoveSpeed(60.0f);
    setAttackDamage(15.0f);
    setDefense(5.0f);
    setKnockbackResist(0.3f);
    setDetectionRange(300.0f);
    setAttackRange(50.0f);
    break;

  case EnemyType::SKELETON:
    setMaxHealth(60.0f);
    setMoveSpeed(100.0f);
    setAttackDamage(12.0f);
    setDefense(3.0f);
    setKnockbackResist(0.2f);
    setDetectionRange(400.0f);
    setAttackRange(200.0f); // 远程攻击
    break;

  case EnemyType::FLYING_EYE:
    setMaxHealth(40.0f);
    setMoveSpeed(120.0f);
    setAttackDamage(10.0f);
    setDefense(1.0f);
    setKnockbackResist(0.0f);
    setDetectionRange(350.0f);
    setAttackRange(150.0f);
    break;

  default:
    break;
  }

  // 初始化血条 - 已禁用
  // initHealthBar();
  // 记录初始位置作为巡逻起点
  _patrolStartPos = this->getPosition();

  // 启动更新
  scheduleUpdate();

  return true;
}

void Enemy::update(float delta) {
  Sprite::update(delta);

  // 更新攻击冷却
  if (_attackCooldownTimer > 0) {
    _attackCooldownTimer -= delta;
  }

  // 更新无敌状态
  if (_isInvincible) {
    _invincibleTimer -= delta;
    if (_invincibleTimer <= 0) {
      _isInvincible = false;
      setOpacity(255); // 恢复正常显示
    } else {
      // 闪烁效果
      int opacity = ((int)(_invincibleTimer * 10) % 2 == 0) ? 255 : 128;
      setOpacity(opacity);
    }
  }

  // 根据当前状态执行对应的AI逻辑
  switch (_currentState) {
  case State::IDLE:
    updateIdle(delta);
    break;
  case State::PATROL:
    updatePatrol(delta);
    break;
  case State::CHASE:
    updateChase(delta);
    break;
  case State::ATTACK:
    updateAttack(delta);
    break;
  case State::HURT:
    updateHurt(delta);
    break;
  case State::DEAD:
    // 死亡状态不更新
    break;
  }

  // 应用速度的逻辑已移除，史莱姆仅使用跳跃移动
  // velocity仅用于击退效果，在applyKnockback中使用
}

// ==================== 属性设置 ====================

void Enemy::setMaxHealth(float maxHP) {
  _maxHealth = maxHP;
  _currentHealth = _maxHealth;
}

void Enemy::setCurrentHealth(float hp) {
  _currentHealth = std::max(0.0f, std::min(hp, _maxHealth));
  // updateHealthBar(); // 血条已禁用

  if (_currentHealth <= 0 && _currentState != State::DEAD) {
    die();
  }
}

// ==================== 战斗系统 ====================

void Enemy::takeDamage(float damage, const Vec2 &knockbackDirection,
                       float knockbackForce) {
  // 如果已经死亡或无敌，不受伤
  if (_currentState == State::DEAD || _isInvincible) {
    return;
  }

  // 计算实际伤害 (简单的防御公式)
  float actualDamage = damage - _defense;
  actualDamage = MAX(actualDamage, 1.0f); // 至少造成1点伤害

  // 扣除生命值
  setCurrentHealth(_currentHealth - actualDamage);

  // 播放受伤动画/音效 (这里可以添加)
  // playAnimation("hurt", false);

  // 应用击退效果
  if (knockbackForce > 0 && _knockbackResist < 1.0f) {
    float actualKnockback = knockbackForce * (1.0f - _knockbackResist);
    applyKnockback(knockbackDirection, actualKnockback);
  }

  // 如果还活着，切换到受伤状态
  if (isAlive()) {
    changeState(State::HURT);
    _hurtStateTimer = _hurtStateDuration;

    // 设置短暂无敌
    _isInvincible = true;
    _invincibleTimer = _invincibleDuration;
  }
}

void Enemy::heal(float healAmount) {
  setCurrentHealth(_currentHealth + healAmount);
}

void Enemy::die() {
  changeState(State::DEAD);

  // 停止所有动作
  stopAllActions();
  _velocity = Vec2::ZERO;

  // 播放死亡动画
  // playAnimation("death", false);

  // 生成掉落物
  spawnDrops();

  // 死亡动画结束后移除节点
  auto fadeOut = FadeOut::create(1.0f);
  auto removeFunc = RemoveSelf::create();
  auto sequence = Sequence::create(fadeOut, removeFunc, nullptr);
  this->runAction(sequence);
}

// ==================== AI系统 ====================

void Enemy::changeState(State newState) {
  if (_currentState == newState) {
    return;
  }

  State oldState = _currentState;
  _currentState = newState;

  // 状态切换时的处理
  switch (_currentState) {
  case State::IDLE:
    _velocity = Vec2::ZERO;
    break;
  case State::PATROL:
    _patrolStartPos = this->getPosition();
    break;
  case State::CHASE:
    break;
  case State::ATTACK:
    _velocity = Vec2::ZERO;
    break;
  case State::HURT:
    break;
  case State::DEAD:
    break;
  }
}

void Enemy::updateIdle(float delta) {
  // 检测玩家
  if (detectPlayer()) {
    changeState(State::CHASE);
    return;
  }

  // 一段时间后切换到巡逻状态
  static float idleTimer = 0;
  idleTimer += delta;
  if (idleTimer > 2.0f) {
    idleTimer = 0;
    changeState(State::PATROL);
  }
}

void Enemy::updatePatrol(float delta) {
  // 巡逻逻辑由子类实现（如史莱姆的跳跃巡逻）
  // 检测玩家
  if (detectPlayer()) {
    changeState(State::CHASE);
    return;
  }

  // 默认什么都不做，由子类重写
}

void Enemy::updateChase(float delta) {
  // 追击逻辑由子类实现（如史莱姆的跳跃追击）
  // 检查玩家是否还在范围内
  if (!detectPlayer()) {
    changeState(State::PATROL);
    return;
  }

  if (_playerTarget == nullptr) {
    changeState(State::IDLE);
    return;
  }

  // 默认什么都不做，由子类重写
}

void Enemy::updateAttack(float delta) {
  if (_playerTarget == nullptr) {
    changeState(State::IDLE);
    return;
  }

  Vec2 playerPos = _playerTarget->getPosition();
  Vec2 currentPos = this->getPosition();
  float distance = currentPos.distance(playerPos);

  // 如果玩家离开攻击范围，继续追击
  if (distance > _attackRange) {
    changeState(State::CHASE);
    return;
  }

  // 攻击冷却检查
  if (_attackCooldownTimer <= 0) {
    // 执行攻击
    // playAnimation("attack", false);

    // 这里应该创建攻击判定/发射弹幕等
    // 具体实现由子类完成

    _attackCooldownTimer = _attackCooldown;
  }

  // 面向玩家
  if (playerPos.x < currentPos.x) {
    setFacingRight(false);
  } else {
    setFacingRight(true);
  }
}

void Enemy::updateHurt(float delta) {
  _hurtStateTimer -= delta;

  if (_hurtStateTimer <= 0) {
    // 受伤状态结束，根据情况切换状态
    if (detectPlayer()) {
      changeState(State::CHASE);
    } else {
      changeState(State::PATROL);
    }
  }
}

bool Enemy::detectPlayer() {
  if (_playerTarget == nullptr) {
    return false;
  }

  Vec2 playerPos = _playerTarget->getPosition();
  Vec2 currentPos = this->getPosition();
  float distance = currentPos.distance(playerPos);

  return distance <= _detectionRange;
}

void Enemy::moveTowards(const Vec2 &target, float delta) {
  Vec2 currentPos = this->getPosition();
  Vec2 direction = target - currentPos;
  direction.normalize();

  _velocity = direction * _moveSpeed;

  // 更新朝向
  if (direction.x < 0) {
    setFacingRight(false);
  } else if (direction.x > 0) {
    setFacingRight(true);
  }
}

void Enemy::applyKnockback(const Vec2 &direction, float force) {
  Vec2 knockbackVec = direction.getNormalized() * force;
  _velocity += knockbackVec;
}

// ==================== 掉落系统 ====================

void Enemy::addDropItem(const std::string &itemId, int minCount, int maxCount,
                        float dropChance) {
  _dropTable.push_back(DropItem(itemId, minCount, maxCount, dropChance));
}

void Enemy::spawnDrops() {
  for (const auto &drop : _dropTable) {
    float randomValue = CCRANDOM_0_1();
    if (randomValue <= drop.dropChance) {
      int count = random(drop.minCount, drop.maxCount);

      // 在这里创建掉落物节点
      // 具体实现取决于你的物品系统
      CCLOG("Dropped %d x %s", count, drop.itemId.c_str());
    }
  }
}

// ==================== 动画系统 ====================

void Enemy::playAnimation(const std::string &animName, bool loop) {
  // 这里应该根据你的动画资源实现
  // 例如使用精灵帧动画或骨骼动画
  // 示例：
  // auto animation = AnimationCache::getInstance()->getAnimation(animName);
  // if (animation)
  // {
  //     auto animate = Animate::create(animation);
  //     if (loop)
  //     {
  //         this->runAction(RepeatForever::create(animate));
  //     }
  //     else
  //     {
  //         this->runAction(animate);
  //     }
  // }
}

void Enemy::setFacingRight(bool facingRight) {
  if (_facingRight == facingRight) {
    return;
  }

  _facingRight = facingRight;

  // 翻转精灵
  if (_facingRight) {
    this->setFlippedX(false);
  } else {
    this->setFlippedX(true);
  }
}

// ==================== 血条系统 ====================

void Enemy::initHealthBar() {
  // 创建血条背景 - 调小尺寸
  _healthBarBg = Sprite::create();
  _healthBarBg->setTextureRect(Rect(0, 0, 40, 4)); // 调小为40x4
  _healthBarBg->setColor(Color3B(50, 50, 50));     // 深灰色背景
  _healthBarBg->setOpacity(200);                   // 设置半透明
  _healthBarBg->setAnchorPoint(Vec2(0.5f, 0.5f));  // 明确设置中心锁点
  // 血条位置设置在史莱姆图片中心下方
  _healthBarBg->setPosition(Vec2(0, -12)); // 垂直距离-12像素，水平居中
  this->addChild(_healthBarBg, 100);       // 更高的z-order确保显示在最前面

  // 创建血条填充
  _healthBarFill = Sprite::create();
  _healthBarFill->setTextureRect(Rect(0, 0, 40, 4)); // 调小为40x4
  _healthBarFill->setColor(Color3B::GREEN);          // 满血时为绿色
  _healthBarFill->setOpacity(255);                   // 完全不透明
  _healthBarFill->setAnchorPoint(Vec2(0, 0.5f));     // 左对齐，从左边开始缩放
  _healthBarFill->setPosition(Vec2(-20, 0)); // 左侧对齐背景左端，垂直居中
  _healthBarBg->addChild(_healthBarFill);
}

void Enemy::updateHealthBar() {
  if (_healthBarFill == nullptr) {
    return;
  }

  float healthPercent = _currentHealth / _maxHealth;
  _healthBarFill->setScaleX(healthPercent);

  // 根据生命值百分比改变颜色
  if (healthPercent > 0.5f) {
    _healthBarFill->setColor(Color3B::GREEN);
  } else if (healthPercent > 0.25f) {
    _healthBarFill->setColor(Color3B::YELLOW);
  } else {
    _healthBarFill->setColor(Color3B::RED);
  }
}
