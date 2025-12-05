#include "SlimeEnemy.h"

SlimeEnemy::SlimeEnemy()
    : _isOnGround(false), _jumpCooldown(1.5f),
      _jumpCooldownTimer(_jumpCooldown), _maxHorizontalImpulse(300.0f),
      _maxVerticalImpulse(500.0f), _randomDirection(1), _aggroRange(300.0f),
      _deaggroRange(400.0f) {
  // 随机初始方向
  _randomDirection = (CCRANDOM_0_1() > 0.5f) ? 1 : -1;
}

bool SlimeEnemy::init(const std::string &slimeType) {
  // 先尝试加载第一帧作为默认图片
  bool loadSuccess = false;

  // 根据史莱姆类型构建第一帧的路径
  std::string firstFramePath =
      "Minor monster/" + slimeType + "_Slime/" + slimeType + "_Slime1.png";

  if (Sprite::initWithFile(firstFramePath)) {
    loadSuccess = true;
    CCLOG("%s Slime: Loaded first frame successfully", slimeType.c_str());
  }
  // 如果失败，使用备用方案
  else {
    CCLOG("%s Slime: Failed to load slime images, using fallback",
          slimeType.c_str());
    if (!Sprite::init()) {
      return false;
    }
    this->setTextureRect(Rect(0, 0, 40, 40));
    // 根据类型设置不同的颜色
    if (slimeType == "Green") {
      this->setColor(Color3B::GREEN);
    } else if (slimeType == "Blue") {
      this->setColor(Color3B::BLUE);
    }
    loadSuccess = true;
  }

  if (!loadSuccess) {
    return false;
  }

  // 调用父类Enemy的初始化，设置为史莱姆类型
  Enemy::init(EnemyType::SLIME);

  // 初始化帧动画
  initAnimation(slimeType);

  // 设置物理体和碰撞检测
  setupPhysics();

  // 添加掉落物
  addDropItem("gel", 1, 3, 0.8f);   // 80%概率掉落1-3个凝胶
  addDropItem("coin", 5, 15, 0.5f); // 50%概率掉落5-15金币

  return true;
}

void SlimeEnemy::update(float delta) {
  // 调用父类的update（处理无敌、攻击冷却等）
  Enemy::update(delta);

  // 如果死亡了就不处理
  if (_currentState == State::DEAD) {
    return;
  }

  // ==================== 自动索敌逻辑 ====================
  if (_playerTarget != nullptr) {
    Vec2 playerPos = _playerTarget->getPosition();
    Vec2 currentPos = this->getPosition();
    float distance = currentPos.distance(playerPos);

    // 根据当前状态和距离决定是否切换状态
    if (_currentState == State::CHASE || _currentState == State::ATTACK) {
      // 已经在追击/攻击状态，检查是否脱离仇恨
      if (distance > _deaggroRange) {
        // 目标走远了，失去仇恨，进入巡逻模式
        changeState(State::PATROL);
        CCLOG("Slime: Target too far (%.1f > %.1f), lost aggro", distance,
              _deaggroRange);
      }
    } else {
      // 在巡逻/空闲状态，检查是否索敌
      if (distance <= _aggroRange) {
        // 目标进入仇恨范围，开始追击
        changeState(State::CHASE);
        CCLOG("Slime: Found target (%.1f <= %.1f), start chase", distance,
              _aggroRange);
      }
    }
  } else {
    // 没有目标，保持巡逻/空闲状态
    if (_currentState == State::CHASE || _currentState == State::ATTACK) {
      changeState(State::PATROL);
    }
  }
}

void SlimeEnemy::setupPhysics() {
  // 添加物理体 - 使用矩形防止翻转
  // PhysicsMaterial(密度, 弹性, 摩擦力)
  PhysicsMaterial slimeMaterial(1.0f, 0.0f, 1.0f); // 弹性0,完全不反弹,高摩擦力
  auto physicsBody = PhysicsBody::createBox(
      Size(35, 35), slimeMaterial);      // 矩形物理体，35x35像素
  physicsBody->setDynamic(true);         // 动态，受重力影响
  physicsBody->setMass(1.0f);            // 质量
  physicsBody->setRotationEnable(false); // 禁止旋转，防止翻转
  physicsBody->setContactTestBitmask(0xFFFFFFFF);
  physicsBody->setCollisionBitmask(0xFFFFFFFF);

  // 设置负数group，同组的敌人之间不会碰撞，但仍然可以与地面/玩家碰撞
  physicsBody->setGroup(-1);

  this->setPhysicsBody(physicsBody);

  // 添加碰撞监听器，检测是否在地面
  auto contactListener = EventListenerPhysicsContact::create();
  contactListener->onContactBegin = [this](PhysicsContact &contact) {
    // 碰撞开始
    auto bodyA = contact.getShapeA()->getBody();
    auto bodyB = contact.getShapeB()->getBody();

    // 判断碰撞的物体是否是地面/平台（动态属性为false）
    bool isGround = false;
    if (bodyA == this->getPhysicsBody() && !bodyB->isDynamic()) {
      isGround = true;
    } else if (bodyB == this->getPhysicsBody() && !bodyA->isDynamic()) {
      isGround = true;
    }

    if (isGround) {
      _isOnGround = true;
    }
    return true;
  };

  contactListener->onContactSeparate = [this](PhysicsContact &contact) {
    // 离开地面
    auto bodyA = contact.getShapeA()->getBody();
    auto bodyB = contact.getShapeB()->getBody();

    // 判断分离的物体是否是地面/平台（动态属性为false）
    bool wasGround = false;
    if (bodyA == this->getPhysicsBody() && !bodyB->isDynamic()) {
      wasGround = true;
    } else if (bodyB == this->getPhysicsBody() && !bodyA->isDynamic()) {
      wasGround = true;
    }

    if (wasGround) {
      _isOnGround = false;
    }
  };

  Director::getInstance()
      ->getEventDispatcher()
      ->addEventListenerWithSceneGraphPriority(contactListener, this);
}

void SlimeEnemy::updateChase(float delta) {
  // 检查玩家是否还在范围内
  if (!detectPlayer()) {
    changeState(State::PATROL);
    return;
  }

  if (_playerTarget == nullptr) {
    changeState(State::PATROL);
    return;
  }

  Vec2 playerPos = _playerTarget->getPosition();
  Vec2 currentPos = this->getPosition();
  float distance = currentPos.distance(playerPos);

  // 如果在攻击范围内，切换到攻击状态
  if (distance <= _attackRange) {
    changeState(State::ATTACK);
    return;
  }

  // 只有在地面上且静止时，才计时冷却并跳跃
  if (isGroundedAndStill()) {
    if (_jumpCooldownTimer > 0) {
      _jumpCooldownTimer -= delta;
    } else {
      // 冷却完成，执行跳跃
      performSmartJump(playerPos);
      _jumpCooldownTimer = _jumpCooldown;
    }
  }
}

void SlimeEnemy::updateAttack(float delta) {
  if (_playerTarget == nullptr) {
    changeState(State::PATROL);
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

  // 只有在地面上且静止时，才计时冷却并跳跃
  if (isGroundedAndStill()) {
    if (_jumpCooldownTimer > 0) {
      _jumpCooldownTimer -= delta;
    } else {
      // 冷却完成，执行跳跃
      performSmartJump(playerPos);
      _jumpCooldownTimer = _jumpCooldown;
    }
  }

  // 面向玩家
  if (playerPos.x < currentPos.x) {
    setFacingRight(false);
  } else {
    setFacingRight(true);
  }
}

void SlimeEnemy::performJump(const Vec2 &direction) {
  // 只有在地面上才能执行跳跃
  if (!_isOnGround) {
    return;
  }

  // 跳跃后离开地面
  _isOnGround = false;

  // 使用物理引擎实现真实的跳跃
  auto physicsBody = this->getPhysicsBody();
  if (physicsBody) {
    float horizontalImpulse = direction.x * 300.0f;
    float verticalImpulse = 500.0f;
    physicsBody->applyImpulse(Vec2(horizontalImpulse, verticalImpulse));
  }

  // 跳跃动画效果 - 使用相对缩放保持原始大小
  float jumpDuration = 0.5f;
  auto jumpUp = ScaleBy::create(jumpDuration * 0.4f, 1.2f); // 放大1.2倍
  auto jumpDown =
      ScaleBy::create(jumpDuration * 0.6f, 1.0f / 1.2f); // 缩回原来大小
  auto scaleSeq = Sequence::create(jumpUp, jumpDown, nullptr);
  this->runAction(scaleSeq);

  // 面向移动方向
  if (direction.x < 0) {
    setFacingRight(false);
  } else if (direction.x > 0) {
    setFacingRight(true);
  }
}

void SlimeEnemy::initAnimation(const std::string &slimeType) {
  // 尝试创建帧动画
  Vector<SpriteFrame *> frames;

  // 根据不同的史莱姆类型加载不同的动画帧
  std::string slimeFolder = slimeType;

  // 加载对应类型的2帧动画
  int frameCount = 0;
  for (int i = 1; i <= 2; i++) {
    // 使用子目录路径：Minor monster/Blue_Slime/Blue_Slime1.png
    std::string framePath = "Minor monster/" + slimeType + "Slime" + "/" + slimeType + "Slime" + std::to_string(i) + ".png";
    // 不指定Rect,让Cocos2d-x自动使用图片的实际尺寸
    auto texture =
        Director::getInstance()->getTextureCache()->addImage(framePath);
    if (texture) {
      auto spriteFrame = SpriteFrame::createWithTexture(
          texture, Rect(0, 0, texture->getContentSize().width,
                        texture->getContentSize().height));
      if (spriteFrame) {
        frames.pushBack(spriteFrame);
        frameCount++;
      }
    } else {
      // 如果加载失败，停止加载
      CCLOG("Failed to load frame: %s", framePath.c_str());
      break;
    }
  }

  // 如果成功加载了至少两帧，创建动画
  if (frameCount >= 2) {
    CCLOG("Creating %s Slime animation with %d frames", slimeType.c_str(),
          frameCount);
    auto animation =
        Animation::createWithSpriteFrames(frames, 0.15f); // 0.15秒每帧
    auto animate = Animate::create(animation);
    this->runAction(RepeatForever::create(animate));
  } else {
    CCLOG("Not enough frames for animation (found %d), using static image",
          frameCount);
  }
}

// ==================== 新增：AI行为方法 ====================

void SlimeEnemy::updateIdle(float delta) {
  // 史莱姆没有空闲状态，直接转为巡逻状态
  updatePatrol(delta);
}

void SlimeEnemy::updatePatrol(float delta) {
  // 检测是否有玩家进入范围
  if (detectPlayer()) {
    changeState(State::CHASE);
    return;
  }

  // 只有在地面上且静止时，才计时冷却并跳跃
  if (isGroundedAndStill()) {
    if (_jumpCooldownTimer > 0) {
      _jumpCooldownTimer -= delta;
    } else {
      // 冷却完成，执行跳跃
      performRandomJump();
      _jumpCooldownTimer = _jumpCooldown;
    }
  }
}

// ==================== 新增：跳跃方法 ====================

bool SlimeEnemy::isGroundedAndStill() const {
  if (!_isOnGround) {
    return false;
  }
  auto physicsBody = this->getPhysicsBody();
  if (!physicsBody) {
    return false;
  }
  auto velocity = physicsBody->getVelocity();
  // 判断是否静止（速度接近0）
  return std::abs(velocity.x) < 10.0f && std::abs(velocity.y) < 10.0f;
}

void SlimeEnemy::performRandomJump() {
  // 只有在地面上才能执行跳跃
  if (!_isOnGround) {
    return;
  }

  // 跳跃后离开地面
  _isOnGround = false;

  // 随机改变方向（30%概率）
  if (CCRANDOM_0_1() < 0.3f) {
    _randomDirection = -_randomDirection;
  }

  // 使用最大力度跳跃
  auto physicsBody = this->getPhysicsBody();
  if (physicsBody) {
    float horizontalImpulse = _randomDirection * _maxHorizontalImpulse;
    float verticalImpulse = _maxVerticalImpulse;
    physicsBody->applyImpulse(Vec2(horizontalImpulse, verticalImpulse));
    CCLOG("Random jump: dir=%d, h=%.1f, v=%.1f", _randomDirection,
          horizontalImpulse, verticalImpulse);
  }

  // 跳跃动画效果 - 使用相对缩放保持原始大小
  float jumpDuration = 0.5f;
  auto jumpUp = ScaleBy::create(jumpDuration * 0.4f, 1.2f); // 放大1.2倍
  auto jumpDown =
      ScaleBy::create(jumpDuration * 0.6f, 1.0f / 1.2f); // 缩回原来大小
  auto scaleSeq = Sequence::create(jumpUp, jumpDown, nullptr);
  this->runAction(scaleSeq);

  // 面向移动方向
  setFacingRight(_randomDirection > 0);
}

void SlimeEnemy::performSmartJump(const Vec2 &targetPos) {
  // 只有在地面上才能执行跳跃
  if (!_isOnGround) {
    return;
  }

  // 跳跃后离开地面
  _isOnGround = false;

  Vec2 currentPos = this->getPosition();
  Vec2 direction = targetPos - currentPos;
  float distance = direction.length();
  direction.normalize();

  auto physicsBody = this->getPhysicsBody();
  if (physicsBody) {
    // 智能调整跳跃力度
    float horizontalImpulse =
        (direction.x > 0 ? 1.0f : -1.0f) * _maxHorizontalImpulse;

    float verticalDist = targetPos.y - currentPos.y;
    float horizontalDist = std::abs(targetPos.x - currentPos.x);

    // 基础垂直力度
    float verticalImpulse = _maxVerticalImpulse * 0.6f;

    // 根据高度差调整
    if (verticalDist > 0) {
      float heightFactor = std::min(verticalDist / 100.0f, 1.0f);
      verticalImpulse = _maxVerticalImpulse * (0.6f + 0.4f * heightFactor);
    } else if (verticalDist < -50) {
      verticalImpulse = _maxVerticalImpulse * 0.4f;
    }

    // 根据水平距离微调
    if (horizontalDist > 200) {
      verticalImpulse *= 1.2f;
    }

    // 确保不超过最大力度
    verticalImpulse = std::min(verticalImpulse, _maxVerticalImpulse);

    physicsBody->applyImpulse(Vec2(horizontalImpulse, verticalImpulse));
    CCLOG("Smart jump: dist=%.1f, vDist=%.1f, h=%.1f, v=%.1f", distance,
          verticalDist, horizontalImpulse, verticalImpulse);
  }

  // 跳跃动画效果 - 使用相对缩放保持原始大小
  float jumpDuration = 0.5f;
  auto jumpUp = ScaleBy::create(jumpDuration * 0.4f, 1.2f); // 放大1.2倍
  auto jumpDown =
      ScaleBy::create(jumpDuration * 0.6f, 1.0f / 1.2f); // 缩回原来大小
  auto scaleSeq = Sequence::create(jumpUp, jumpDown, nullptr);
  this->runAction(scaleSeq);

  // 面向目标方向
  if (direction.x < 0) {
    setFacingRight(false);
  } else if (direction.x > 0) {
    setFacingRight(true);
  }
}