#ifndef __ECS_COMPONENTS_H__
#define __ECS_COMPONENTS_H__

#include "Component.h"
#include "cocos2d.h"
#include <functional>
#include <string>
#include <vector>

namespace ecs {

// ==================== 变换组件 ====================

/**
 * @brief 变换组件 - 位置、旋转、缩放
 */
struct TransformComponent : public IComponent {
  cocos2d::Vec2 position = cocos2d::Vec2::ZERO;
  float rotation = 0.0f; // 角度
  cocos2d::Vec2 scale = cocos2d::Vec2(1.0f, 1.0f);

  // 速度 (用于运动系统)
  cocos2d::Vec2 velocity = cocos2d::Vec2::ZERO;

  // 父实体 (用于层级关系)
  EntityId parent = INVALID_ENTITY;

  TransformComponent() = default;
  TransformComponent(float x, float y) : position(x, y) {}
  TransformComponent(const cocos2d::Vec2 &pos) : position(pos) {}
};

// ==================== 史莱姆精灵组件 ====================

/**
 * @brief 史莱姆精灵组件 - 基于现有史莱姆代码设计
 *
 * 特性:
 * - 继承自cocos2d::Sprite，直接作为场景节点
 * - 内置帧动画播放
 * - 跳跃缩放动画效果
 * - 物理体集成
 * - 朝向控制（翻转）
 */
struct SlimeSpriteComponent : public IComponent {
  cocos2d::Sprite *sprite = nullptr;   // 精灵节点
  cocos2d::Node *parentNode = nullptr; // 挂载的父节点
  std::string slimeType = "Green";     // 史莱姆类型: Green, Blue, Yellow

  // 渲染属性
  int zOrder = 0;
  bool visible = true;
  bool facingRight = true; // 朝向
  float baseScale = 1.0f;  // 基础缩放
  cocos2d::Color3B color = cocos2d::Color3B::WHITE;
  uint8_t opacity = 255;

  // 动画相关
  cocos2d::Vector<cocos2d::SpriteFrame *> animFrames; // 动画帧
  float frameTime = 0.15f;                            // 每帧时间
  bool animationLoaded = false;                       // 动画是否已加载
  int currentFrameIndex = 0;                          // 当前帧索引
  float frameTimer = 0.0f;                            // 帧计时器

  SlimeSpriteComponent() = default;

  /**
   * @brief 构造并初始化史莱姆精灵
   * @param type 史莱姆类型 ("Green", "Blue", "Yellow")
   */
  SlimeSpriteComponent(const std::string &type) : slimeType(type) {
    initSprite();
  }

  ~SlimeSpriteComponent() {
    if (sprite) {
      sprite->stopAllActions();
      sprite->removeFromParent();
      sprite->release();
      sprite = nullptr;
    }
  }

  // 禁止拷贝
  SlimeSpriteComponent(const SlimeSpriteComponent &) = delete;
  SlimeSpriteComponent &operator=(const SlimeSpriteComponent &) = delete;

  // 允许移动
  SlimeSpriteComponent(SlimeSpriteComponent &&other) noexcept
      : sprite(other.sprite), parentNode(other.parentNode),
        slimeType(std::move(other.slimeType)), zOrder(other.zOrder),
        visible(other.visible), facingRight(other.facingRight),
        baseScale(other.baseScale), color(other.color), opacity(other.opacity),
        animFrames(std::move(other.animFrames)), frameTime(other.frameTime),
        animationLoaded(other.animationLoaded) {
    other.sprite = nullptr;
    other.parentNode = nullptr;
  }

  /**
   * @brief 初始化精灵 - 加载第一帧图片
   */
  bool initSprite() {
    // 构建第一帧路径: Minor monster/GreenSlime/Green_Slime1.png
    std::string firstFramePath =
        "Minor monster/" + slimeType + "Slime/" + slimeType + "_Slime1.png";

    sprite = cocos2d::Sprite::create(firstFramePath);
    if (sprite) {
      sprite->retain();
      CCLOG("%s Slime: Loaded first frame from %s", slimeType.c_str(),
            firstFramePath.c_str());
      return true;
    }

    // 加载失败，使用备用纯色方块
    CCLOG("%s Slime: Failed to load %s, using fallback", slimeType.c_str(),
          firstFramePath.c_str());
    sprite = cocos2d::Sprite::create();
    if (sprite) {
      sprite->retain();
      sprite->setTextureRect(cocos2d::Rect(0, 0, 40, 40));

      // 根据类型设置颜色
      if (slimeType == "Green") {
        sprite->setColor(cocos2d::Color3B::GREEN);
      } else if (slimeType == "Blue") {
        sprite->setColor(cocos2d::Color3B::BLUE);
      } else if (slimeType == "Yellow") {
        sprite->setColor(cocos2d::Color3B::YELLOW);
      }
      return true;
    }
    return false;
  }

  /**
   * @brief 加载帧动画
   */
  void loadAnimation() {
    if (animationLoaded)
      return;

    animFrames.clear();

    // 加载2帧动画: Minor monster/GreenSlime/GreenSlime1.png
    for (int i = 1; i <= 2; i++) {
      std::string framePath = "Minor monster/" + slimeType + "Slime/" +
                              slimeType + "_Slime" + std::to_string(i) + ".png";

      auto texture =
          cocos2d::Director::getInstance()->getTextureCache()->addImage(
              framePath);
      if (texture) {
        auto frame = cocos2d::SpriteFrame::createWithTexture(
            texture, cocos2d::Rect(0, 0, texture->getContentSize().width,
                                   texture->getContentSize().height));
        if (frame) {
          animFrames.pushBack(frame);
        }
      } else {
        CCLOG("Failed to load frame: %s", framePath.c_str());
        break;
      }
    }

    animationLoaded = (animFrames.size() >= 2);
    CCLOG("%s Slime: Loaded %zu animation frames", slimeType.c_str(),
          animFrames.size());
  }

  /**
   * @brief 播放循环动画
   */
  void playIdleAnimation() {
    if (!sprite || animFrames.size() < 2)
      return;

    sprite->stopAllActions();
    auto animation =
        cocos2d::Animation::createWithSpriteFrames(animFrames, frameTime);
    auto animate = cocos2d::Animate::create(animation);
    sprite->runAction(cocos2d::RepeatForever::create(animate));
  }

  /**
   * @brief 播放跳跃缩放动画
   */
  void playJumpAnimation() {
    if (!sprite)
      return;

    float jumpDuration = 0.5f;
    // 使用ScaleBy保持原始大小
    auto jumpUp = cocos2d::ScaleBy::create(jumpDuration * 0.4f, 1.2f);
    auto jumpDown = cocos2d::ScaleBy::create(jumpDuration * 0.6f, 1.0f / 1.2f);
    auto scaleSeq = cocos2d::Sequence::create(jumpUp, jumpDown, nullptr);
    sprite->runAction(scaleSeq);
  }

  /**
   * @brief 设置朝向
   * @param right true=朝右, false=朝左
   */
  void setFacing(bool right) {
    facingRight = right;
    if (sprite) {
      sprite->setScaleX(baseScale * (right ? 1.0f : -1.0f));
    }
  }

  /**
   * @brief 同步位置到精灵
   */
  void syncPosition(const cocos2d::Vec2 &pos) {
    if (sprite) {
      sprite->setPosition(pos);
    }
  }

  /**
   * @brief 将精灵添加到父节点
   */
  void attachToNode(cocos2d::Node *parent, int z = 0) {
    if (sprite && parent) {
      parentNode = parent;
      zOrder = z;
      parent->addChild(sprite, z);
    }
  }

  /**
   * @brief 设置物理体
   */
  void setupPhysicsBody(float size = 35.0f, int group = -1) {
    if (!sprite)
      return;

    // PhysicsMaterial(密度, 弹性, 摩擦力) - 弹性0防止反弹
    cocos2d::PhysicsMaterial material(1.0f, 0.0f, 1.0f);
    auto body =
        cocos2d::PhysicsBody::createBox(cocos2d::Size(size, size), material);

    body->setDynamic(true);
    body->setMass(1.0f);
    body->setRotationEnable(false); // 禁止旋转防止翻转
    body->setContactTestBitmask(0xFFFFFFFF);
    body->setCollisionBitmask(0xFFFFFFFF);
    body->setGroup(group); // 负数group使同类敌人不碰撞

    sprite->setPhysicsBody(body);
  }

  /**
   * @brief 获取物理体
   */
  cocos2d::PhysicsBody *getPhysicsBody() {
    return sprite ? sprite->getPhysicsBody() : nullptr;
  }

  /**
   * @brief 应用冲量（跳跃）
   */
  void applyImpulse(const cocos2d::Vec2 &impulse) {
    auto body = getPhysicsBody();
    if (body) {
      body->applyImpulse(impulse);
    }
  }

  /**
   * @brief 获取物理体速度
   */
  cocos2d::Vec2 getVelocity() {
    auto body = getPhysicsBody();
    return body ? body->getVelocity() : cocos2d::Vec2::ZERO;
  }
};

// ==================== 游戏逻辑组件 ====================

/**
 * @brief 生命值组件
 */
struct HealthComponent : public IComponent {
  float maxHealth = 100.0f;
  float currentHealth = 100.0f;
  bool isDead = false;
  float invincibleTime = 0.0f;  // 无敌时间
  float invincibleTimer = 0.0f; // 无敌计时器

  HealthComponent() = default;
  HealthComponent(float max) : maxHealth(max), currentHealth(max) {}

  void takeDamage(float damage) {
    if (invincibleTimer > 0)
      return;
    currentHealth = std::max(0.0f, currentHealth - damage);
    if (currentHealth <= 0) {
      isDead = true;
    }
  }

  void heal(float amount) {
    currentHealth = std::min(maxHealth, currentHealth + amount);
  }

  float getHealthPercent() const {
    return maxHealth > 0 ? currentHealth / maxHealth : 0.0f;
  }
};

/**
 * @brief 玩家标记组件
 */
struct PlayerTag : public IComponent {};

/**
 * @brief 敌人标记组件
 */
struct EnemyTag : public IComponent {
  std::string enemyType;
  EnemyTag() = default;
  EnemyTag(const std::string &type) : enemyType(type) {}
};

/**
 * @brief 掉落物组件
 */
struct LootComponent : public IComponent {
  struct DropItem {
    std::string itemId;
    int minCount;
    int maxCount;
    float dropChance;
  };

  std::vector<DropItem> dropTable;

  void addDrop(const std::string &id, int minC, int maxC, float chance) {
    dropTable.push_back({id, minC, maxC, chance});
  }
};

/**
 * @brief 生命周期组件 - 自动销毁
 */
struct LifetimeComponent : public IComponent {
  float lifetime = 1.0f;
  float elapsed = 0.0f;
  std::function<void(EntityId)> onExpire;

  LifetimeComponent() = default;
  LifetimeComponent(float time) : lifetime(time) {}

  bool isExpired() const { return elapsed >= lifetime; }
};

/**
 * @brief 战斗组件 - 攻击参数
 */
struct CombatComponent : public IComponent {
  float attackRange = 50.0f;   // 攻击范围
  float attackDamage = 10.0f;  // 攻击伤害
  float attackCooldown = 1.0f; // 攻击冷却
  float attackTimer = 0.0f;    // 攻击计时器
  bool canAttack = true;       // 是否可以攻击

  CombatComponent() = default;
  CombatComponent(float range, float damage, float cooldown)
      : attackRange(range), attackDamage(damage), attackCooldown(cooldown) {}
};

// ==================== 史莱姆专用组件 ====================

/**
 * @brief 仇恨系统组件 - 管理索敌和仇恨逻辑
 *
 * 功能:
 * - 目标检测与追踪
 * - 仇恨状态切换
 * - 距离计算
 */
struct AggroComponent : public IComponent {
  EntityId targetEntity = INVALID_ENTITY; // 当前目标实体

  // 仇恨距离配置
  float aggroRange = 500.0f;   // 进入仇恨距离
  float deaggroRange = 600.0f; // 脱离仇恨距离

  // 仇恨状态
  bool hasAggro = false;                                 // 是否有仇恨目标
  float distanceToTarget = 99999.0f;                     // 与目标距离
  cocos2d::Vec2 directionToTarget = cocos2d::Vec2::ZERO; // 指向目标方向

  // 目标标签（用于查找目标）
  std::string targetTag = "Player"; // 默认索敌玩家

  AggroComponent() = default;
  AggroComponent(float aggro, float deaggro)
      : aggroRange(aggro), deaggroRange(deaggro) {}

  bool shouldEnterAggro() const {
    return !hasAggro && distanceToTarget <= aggroRange;
  }

  bool shouldExitAggro() const {
    return hasAggro && distanceToTarget > deaggroRange;
  }
};

/**
 * @brief 地面检测组件 - 检测是否在地面/静止
 */
struct GroundDetectorComponent : public IComponent {
  bool isOnGround = false;      // 是否在地面上
  bool isStill = false;         // 是否静止
  float stillThreshold = 10.0f; // 静止判断速度阈值
  int groundContactCount = 0;   // 地面接触计数（用于处理同时接触多个地面）

  GroundDetectorComponent() = default;

  bool isGroundedAndStill() const { return isOnGround && isStill; }
};

/**
 * @brief 跳跃移动组件 - 管理跳跃式移动逻辑
 *
 * 特性:
 * - 跳跃冷却计时
 * - 冲量力度控制
 * - 随机/追踪方向
 */
struct JumpMovementComponent : public IComponent {
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
        maxHorizontalImpulse(hImpulse), maxVerticalImpulse(vImpulse) {
    randomDirection = (rand() % 2 == 0) ? 1 : -1;
  }

  // 计算追踪跳跃冲量
  cocos2d::Vec2 calculateChaseImpulse(float targetDirX, float heightDiff,
                                      float targetDist) {
    cocos2d::Vec2 impulse;

    // 水平方向：始终指向目标
    float dirX = targetDirX > 0 ? 1.0f : -1.0f;
    impulse.x = maxHorizontalImpulse * dirX;

    // 基础垂直力度
    impulse.y = maxVerticalImpulse * 0.6f;

    // 根据高度差调整
    if (heightDiff > 0) {
      float heightFactor = std::min(heightDiff / 100.0f, 1.0f);
      impulse.y = maxVerticalImpulse * (0.6f + 0.4f * heightFactor);
    } else if (heightDiff < -50) {
      impulse.y = maxVerticalImpulse * 0.4f;
    }

    // 根据水平距离调整
    if (targetDist > 200) {
      impulse.y *= 1.2f;
    }

    // 确保不超过最大力度
    impulse.y = std::min(impulse.y, maxVerticalImpulse);

    return impulse;
  }

  // 计算随机跳跃冲量（巡逻用）
  cocos2d::Vec2 calculateRandomImpulse() {
    if ((float)rand() / RAND_MAX < randomDirectionChangeChance) {
      randomDirection *= -1;
    }

    cocos2d::Vec2 impulse;
    impulse.x = maxHorizontalImpulse * patrolImpulseRatio * randomDirection;
    impulse.y = maxVerticalImpulse * 0.8f;

    return impulse;
  }
};

// ==================== 通用怪物精灵组件 ====================

/**
 * @brief 通用怪物精灵组件 - 支持自定义帧序列的精灵动画
 *
 * 与SlimeSpriteComponent不同，此组件支持：
 * - 自定义帧播放序列（如1232循环）
 * - 更通用的怪物类型支持
 */
struct MonsterSpriteComponent : public IComponent {
  cocos2d::Sprite *sprite = nullptr;   // 精灵节点
  cocos2d::Node *parentNode = nullptr; // 挂载的父节点
  std::string monsterType;             // 怪物类型ID

  // 渲染属性
  int zOrder = 0;
  bool visible = true;
  bool facingRight = true; // 朝向
  float baseScale = 1.0f;  // 基础缩放
  cocos2d::Color3B color = cocos2d::Color3B::WHITE;
  uint8_t opacity = 255;

  // 动画相关
  cocos2d::Vector<cocos2d::SpriteFrame *> animFrames; // 所有动画帧
  std::vector<int> frameSequence;  // 帧播放序列（如 {1,2,3,2}）
  float frameTime = 0.15f;         // 每帧时间
  bool animationLoaded = false;    // 动画是否已加载
  int currentFrameIndex = 0;       // 当前帧序列索引
  float frameTimer = 0.0f;         // 帧计时器

  MonsterSpriteComponent() = default;

  ~MonsterSpriteComponent() {
    if (sprite) {
      sprite->stopAllActions();
      sprite->removeFromParent();
      sprite->release();
      sprite = nullptr;
    }
  }

  // 禁止拷贝
  MonsterSpriteComponent(const MonsterSpriteComponent &) = delete;
  MonsterSpriteComponent &operator=(const MonsterSpriteComponent &) = delete;

  // 允许移动
  MonsterSpriteComponent(MonsterSpriteComponent &&other) noexcept
      : sprite(other.sprite), parentNode(other.parentNode),
        monsterType(std::move(other.monsterType)), zOrder(other.zOrder),
        visible(other.visible), facingRight(other.facingRight),
        baseScale(other.baseScale), color(other.color), opacity(other.opacity),
        animFrames(std::move(other.animFrames)),
        frameSequence(std::move(other.frameSequence)),
        frameTime(other.frameTime), animationLoaded(other.animationLoaded),
        currentFrameIndex(other.currentFrameIndex), frameTimer(other.frameTimer) {
    other.sprite = nullptr;
    other.parentNode = nullptr;
  }

  /**
   * @brief 设置朝向
   * @param right true=朝右, false=朝左
   */
  void setFacing(bool right) {
    facingRight = right;
    if (sprite) {
      sprite->setScaleX(baseScale * (right ? 1.0f : -1.0f));
    }
  }

  /**
   * @brief 同步位置到精灵
   */
  void syncPosition(const cocos2d::Vec2 &pos) {
    if (sprite) {
      sprite->setPosition(pos);
    }
  }

  /**
   * @brief 手动更新帧动画（按帧序列播放）
   */
  void updateAnimation(float delta) {
    if (!animationLoaded || frameSequence.empty() || animFrames.empty())
      return;

    frameTimer += delta;
    if (frameTimer >= frameTime) {
      frameTimer -= frameTime;
      currentFrameIndex = (currentFrameIndex + 1) % frameSequence.size();
      
      int frameIdx = frameSequence[currentFrameIndex] - 1; // 帧序列是1-indexed
      if (frameIdx >= 0 && frameIdx < (int)animFrames.size()) {
        sprite->setSpriteFrame(animFrames.at(frameIdx));
      }
    }
  }

  /**
   * @brief 获取物理体
   */
  cocos2d::PhysicsBody *getPhysicsBody() {
    return sprite ? sprite->getPhysicsBody() : nullptr;
  }

  /**
   * @brief 获取物理体速度
   */
  cocos2d::Vec2 getVelocity() {
    auto body = getPhysicsBody();
    return body ? body->getVelocity() : cocos2d::Vec2::ZERO;
  }

  /**
   * @brief 设置物理体速度
   */
  void setVelocity(const cocos2d::Vec2 &vel) {
    auto body = getPhysicsBody();
    if (body) {
      body->setVelocity(vel);
    }
  }

  /**
   * @brief 应用冲量
   */
  void applyImpulse(const cocos2d::Vec2 &impulse) {
    auto body = getPhysicsBody();
    if (body) {
      body->applyImpulse(impulse);
    }
  }
};

// ==================== 行走移动组件 ====================

/**
 * @brief 行走移动组件 - 僵尸等持续行走的怪物
 */
struct WalkMovementComponent : public IComponent {
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
  
  WalkMovementComponent() {
    initDelay = 0.5f + (float)(rand() % 50) / 100.0f;
    patrolDirection = (rand() % 2 == 0) ? 1 : -1;
    currentDirection = patrolDirection;
  }
  WalkMovementComponent(float speed, float jump) : walkSpeed(speed), jumpForce(jump) {
    initDelay = 0.5f + (float)(rand() % 50) / 100.0f;
    patrolDirection = (rand() % 2 == 0) ? 1 : -1;
    currentDirection = patrolDirection;
  }
  
  bool canJump() const { return jumpCooldownTimer <= 0; }
  
  void onJump() {
    jumpCooldownTimer = jumpCooldown;
    jumpCooldownActive = false;
  }
  
  void onLand() {
    if (jumpCooldownTimer > 0 && !jumpCooldownActive) jumpCooldownActive = true;
  }
  
  void updateJumpCooldown(float delta) {
    if (jumpCooldownActive && jumpCooldownTimer > 0) jumpCooldownTimer -= delta;
  }
  
  bool shouldObstacleJump(const cocos2d::Vec2 &currentPos, float delta) {
    if (!obstacleJumpEnabled || obstacleJumpTimer > 0 || !canJump()) return false;
    
    if (!initialized) {
      initTimer += delta;
      if (initTimer >= initDelay) {
        initialized = true;
        stuckCheckStartPos = currentPos;
        stuckTime = 0.0f;
      }
      return false;
    }
    
    if (!isWalking) {
      stuckCheckStartPos = currentPos;
      stuckTime = 0.0f;
      return false;
    }
    
    stuckTime += delta;
    
    if (stuckTime >= stuckThreshold) {
      float actualDist = std::abs(currentPos.x - stuckCheckStartPos.x);
      float expectedDist = walkSpeed * stuckTime;
      stuckCheckStartPos = currentPos;
      stuckTime = 0.0f;
      
      if (expectedDist > 5.0f && actualDist < expectedDist * stuckDistanceRatio) {
        return true;
      }
    }
    return false;
  }
};

struct DeathSpawnComponent : public IComponent {
  std::string spawnType;
  int minCount = 1;
  int maxCount = 3;
  float spawnRadius = 30.0f;
  
  DeathSpawnComponent() = default;
  DeathSpawnComponent(const std::string& type, int min, int max)
    : spawnType(type), minCount(min), maxCount(max) {}
};

// ==================== 缓降组件 ====================

/**
 * @brief 缓降组件 - 用于伞史莱姆等下落时有空气阻力的实体
 * 
 * 当实体下落时，施加向上的阻力，减缓下落速度
 */
struct SlowFallComponent : public IComponent {
  float maxFallSpeed = 100.0f;      // 最大下落速度（正值）
  float fallDamping = 0.85f;        // 下落阻尼系数 (0-1, 越小阻力越大)
  float horizontalDamping = 0.95f;  // 水平阻尼系数 (下落时水平方向的空气阻力)
  bool isActive = true;             // 是否启用缓降
  
  SlowFallComponent() = default;
  SlowFallComponent(float maxSpeed, float fallDamp, float horzDamp = 0.95f)
    : maxFallSpeed(maxSpeed), fallDamping(fallDamp), horizontalDamping(horzDamp) {}
};

// ==================== 投射物相关组件 ====================

/**
 * @brief 投射物攻击组件 - 用于可发射投射物的实体
 * 
 * 用于冰雪尖刺史莱姆等远程攻击的怪物
 */
struct ProjectileAttackComponent : public IComponent {
  // 投射物配置
  std::string projectileSpritePath;     // 投射物贴图路径
  float projectileSpriteWidth = 10.0f;  // 投射物贴图宽度
  float projectileSpriteHeight = 20.0f; // 投射物贴图高度
  int projectileCount = 4;              // 每次发射数量
  float projectileDamage = 8.0f;        // 投射物伤害
  float projectileSpeed = 300.0f;       // 投射物速度
  float projectileLifetime = 3.0f;      // 投射物生命周期
  
  // 发射配置
  float fireInterval = 0.8f;           // 发射间隔
  float fireRange = 200.0f;            // 发射范围（目标在此范围内才发射）
  float horizontalSpread = 80.0f;      // 水平分散角度
  float verticalImpulse = 350.0f;      // 垂直冲量（抛物线高度）
  bool useGravity = true;              // 投射物是否受重力影响
  
  // 状态
  float fireTimer = 0.0f;              // 发射计时器
  bool canFire = true;                 // 是否可以发射
  bool targetInRange = false;          // 目标是否在发射范围内
  
  // 减益效果 - 冰系
  float chillChance = 0.0f;            // 冷冻几率
  float chillDuration = 0.0f;          // 冷冻持续时间
  float chillSpeedReduction = 0.0f;    // 冷冻减速比例
  float freezeChance = 0.0f;           // 冰冻几率
  float freezeDuration = 0.0f;         // 冰冻持续时间
  
  // 减益效果 - 毒系
  float poisonChance1 = 0.0f;          // 毒素几率1 (长时间)
  float poisonDuration1 = 0.0f;        // 毒素持续时间1
  float poisonDamage1 = 0.0f;          // 毒素每秒伤害1
  float poisonChance2 = 0.0f;          // 毒素几率2 (短时间)
  float poisonDuration2 = 0.0f;        // 毒素持续时间2
  float poisonDamage2 = 0.0f;          // 毒素每秒伤害2
  
  ProjectileAttackComponent() = default;
};

/**
 * @brief 投射物组件 - 投射物实体本身
 */
struct ProjectileComponent : public IComponent {
  EntityId owner = INVALID_ENTITY;     // 发射者实体
  float damage = 8.0f;                 // 伤害
  float lifetime = 3.0f;               // 剩余生命周期
  bool hasHit = false;                 // 是否已命中
  
  // 减益效果 - 冰系（继承自发射者）
  float chillChance = 0.0f;
  float chillDuration = 0.0f;
  float chillSpeedReduction = 0.0f;
  float freezeChance = 0.0f;
  float freezeDuration = 0.0f;
  
  // 减益效果 - 毒系（继承自发射者）
  float poisonChance1 = 0.0f;
  float poisonDuration1 = 0.0f;
  float poisonDamage1 = 0.0f;
  float poisonChance2 = 0.0f;
  float poisonDuration2 = 0.0f;
  float poisonDamage2 = 0.0f;
  
  ProjectileComponent() = default;
  ProjectileComponent(EntityId ownerEntity, float dmg) 
    : owner(ownerEntity), damage(dmg) {}
};

/**
 * @brief 减益效果组件 - 附加在受影响的实体上
 */
struct DebuffComponent : public IComponent {
  // 冷冻效果 (Chill) - 减速
  bool hasChillDebuff = false;
  float chillTimer = 0.0f;
  float chillDuration = 0.0f;
  float chillSpeedReduction = 0.0f;  // 0.4 = 减速40%
  
  // 冰冻效果 (Freeze) - 完全无法移动
  bool hasFreezeDebuff = false;
  float freezeTimer = 0.0f;
  float freezeDuration = 0.0f;
  
  // 中毒效果 (Poison) - 持续伤害
  bool hasPoisonDebuff = false;
  float poisonTimer = 0.0f;
  float poisonDuration = 0.0f;
  float poisonDamagePerSecond = 0.0f;  // 每秒伤害
  float poisonTickTimer = 0.0f;        // 伤害计时器
  
  DebuffComponent() = default;
  
  void applyChillDebuff(float duration, float speedReduction) {
    hasChillDebuff = true;
    chillTimer = 0.0f;
    chillDuration = duration;
    chillSpeedReduction = speedReduction;
  }
  
  void applyFreezeDebuff(float duration) {
    hasFreezeDebuff = true;
    freezeTimer = 0.0f;
    freezeDuration = duration;
  }
  
  void applyPoisonDebuff(float duration, float damagePerSecond) {
    // 如果新毒素更强或当前无毒素，则应用
    if (!hasPoisonDebuff || duration > poisonDuration - poisonTimer) {
      hasPoisonDebuff = true;
      poisonTimer = 0.0f;
      poisonDuration = duration;
      poisonDamagePerSecond = damagePerSecond;
      poisonTickTimer = 0.0f;
    }
  }
  
  bool isFrozen() const { return hasFreezeDebuff && freezeTimer < freezeDuration; }
  bool isChilled() const { return hasChillDebuff && chillTimer < chillDuration; }
  bool isPoisoned() const { return hasPoisonDebuff && poisonTimer < poisonDuration; }
  
  float getSpeedMultiplier() const {
    if (isFrozen()) return 0.0f;
    if (isChilled()) return 1.0f - chillSpeedReduction;
    return 1.0f;
  }
};

/**
 * @brief 投射物精灵组件 - 管理投射物的精灵显示
 */
struct ProjectileSpriteComponent : public IComponent {
  cocos2d::Sprite* sprite = nullptr;
  float rotation = 0.0f;  // 根据速度方向旋转
  
  ProjectileSpriteComponent() = default;
  
  ~ProjectileSpriteComponent() {
    if (sprite) {
      sprite->stopAllActions();
      sprite->removeFromParent();
      sprite->release();
      sprite = nullptr;
    }
  }
  
  // 禁止拷贝
  ProjectileSpriteComponent(const ProjectileSpriteComponent&) = delete;
  ProjectileSpriteComponent& operator=(const ProjectileSpriteComponent&) = delete;
  
  // 允许移动
  ProjectileSpriteComponent(ProjectileSpriteComponent&& other) noexcept
    : sprite(other.sprite), rotation(other.rotation) {
    other.sprite = nullptr;
  }
};

} // namespace ecs

#endif // __ECS_COMPONENTS_H__
