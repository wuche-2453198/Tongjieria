#ifndef __ECS_SYSTEMS_H__
#define __ECS_SYSTEMS_H__

#include "Components.h"
#include "Entity.h"
#include "System.h"
#include "cocos2d.h"

namespace ecs {

/**
 * @brief 简单的射线预测碰撞修正，防止高速移动穿入静态障碍
 * @param body 需要修正的刚体
 * @param delta 当前帧的delta时间
 */
inline void performRaycastCorrection(cocos2d::PhysicsBody *body, float delta) {
  // 暂时禁用射线修正，避免将实体推出屏幕或导致嵌入问题
  // 交由物理引擎自行处理碰撞
  return;
}

// ==================== 史莱姆渲染系统 ====================

/**
 * @brief 史莱姆渲染系统 - 同步Transform到SlimeSprite
 */
class SlimeRenderSystem : public ISystem {
public:
  const char *getName() const override { return "SlimeRenderSystem"; }
  int getPriority() const override { return SystemPriority::RENDER; }

  void update(float delta) override {
    _world->forEach<TransformComponent, SlimeSpriteComponent>(
        [](EntityId entity, TransformComponent &transform,
           SlimeSpriteComponent &slimeSprite) {
          if (!slimeSprite.sprite)
            return;

          // 注意：不要手动同步位置！物理引擎会自动更新精灵位置
          // 只从精灵读取位置到transform供其他系统使用
          if (auto *body = slimeSprite.sprite->getPhysicsBody()) {
            transform.position = slimeSprite.sprite->getPosition();
          }

          // 同步显示属性
          slimeSprite.sprite->setRotation(transform.rotation);
          slimeSprite.sprite->setVisible(slimeSprite.visible);
          slimeSprite.sprite->setColor(slimeSprite.color);
          slimeSprite.sprite->setOpacity(slimeSprite.opacity);
        });
  }
};

// ==================== 生命周期系统 ====================

/**
 * @brief 生命周期系统 - 处理自动销毁
 */
class LifetimeSystem : public ISystem {
public:
  const char *getName() const override { return "LifetimeSystem"; }
  int getPriority() const override { return SystemPriority::UI + 100; }

  void update(float delta) override {
    std::vector<EntityId> toDestroy;

    _world->forEach<LifetimeComponent>(
        [delta, &toDestroy, this](EntityId entity,
                                  LifetimeComponent &lifetime) {
          lifetime.elapsed += delta;
          if (lifetime.isExpired()) {
            if (lifetime.onExpire) {
              lifetime.onExpire(entity);
            }
            toDestroy.push_back(entity);
          }
        });

    for (EntityId entity : toDestroy) {
      _world->destroyEntity(entity);
    }
  }
};

// ==================== 生命值系统 ====================

/**
 * @brief 生命值系统 - 处理无敌时间和死亡
 */
class HealthSystem : public ISystem {
public:
  const char *getName() const override { return "HealthSystem"; }
  int getPriority() const override { return SystemPriority::COLLISION + 50; }

  void update(float delta) override {
    _world->forEach<HealthComponent>(
        [delta](EntityId entity, HealthComponent &health) {
          // 更新无敌时间
          if (health.invincibleTimer > 0) {
            health.invincibleTimer -= delta;
          }
        });
  }
};

// ==================== 战斗系统 ====================

/**
 * @brief 战斗系统 - 处理攻击冷却
 */
class CombatSystem : public ISystem {
public:
  const char *getName() const override { return "CombatSystem"; }
  int getPriority() const override { return SystemPriority::COLLISION + 100; }

  void update(float delta) override {
    _world->forEach<CombatComponent>(
        [delta](EntityId entity, CombatComponent &combat) {
          if (combat.attackTimer > 0) {
            combat.attackTimer -= delta;
          }
        });
  }
};

// ==================== 仇恨检测系统 ====================

/**
 * @brief 仇恨检测系统 - 检测目标并更新仇恨状态
 */
class AggroSystem : public ISystem {
public:
  const char *getName() const override { return "AggroSystem"; }
  int getPriority() const override { return SystemPriority::AI - 10; }

  void update(float delta) override {
    _world->forEach<AggroComponent, TransformComponent>(
        [this](EntityId entity, AggroComponent &aggro,
               TransformComponent &transform) {
          // 查找目标实体
          EntityId target = _world->findEntityByTag(aggro.targetTag);
          aggro.targetEntity = target;

          if (target == INVALID_ENTITY) {
            aggro.distanceToTarget = 99999.0f;
            aggro.directionToTarget = cocos2d::Vec2::ZERO;
            if (aggro.hasAggro) {
              aggro.hasAggro = false;
              CCLOG("Entity %u: Lost target, exiting aggro", entity);
            }
            return;
          }

          // 计算到目标的距离和方向
          auto *targetTransform =
              _world->getComponent<TransformComponent>(target);
          if (!targetTransform)
            return;

          cocos2d::Vec2 diff = targetTransform->position - transform.position;
          aggro.distanceToTarget = diff.length();
          aggro.directionToTarget = diff.getNormalized();

          // 仇恨状态切换
          if (aggro.shouldEnterAggro()) {
            aggro.hasAggro = true;
            CCLOG("Entity %u: Target in range (%.1f <= %.1f), entering aggro",
                  entity, aggro.distanceToTarget, aggro.aggroRange);
          } else if (aggro.shouldExitAggro()) {
            aggro.hasAggro = false;
            CCLOG("Entity %u: Target too far (%.1f > %.1f), exiting aggro",
                  entity, aggro.distanceToTarget, aggro.deaggroRange);
          }
        });
  }
};

// ==================== 地面检测系统 ====================

/**
 * @brief 地面检测系统 - 根据物理体速度判断地面和静止状态
 */
class GroundDetectorSystem : public ISystem {
public:
  const char *getName() const override { return "GroundDetectorSystem"; }
  int getPriority() const override { return SystemPriority::PHYSICS + 10; }

  void update(float delta) override {
    _world->forEach<GroundDetectorComponent, SlimeSpriteComponent>(
        [](EntityId entity, GroundDetectorComponent &ground,
           SlimeSpriteComponent &sprite) {
          cocos2d::Vec2 velocity = sprite.getVelocity();

          // 判断是否静止
          ground.isStill = std::abs(velocity.x) < ground.stillThreshold &&
                           std::abs(velocity.y) < ground.stillThreshold;
        });
  }
};

// ==================== 跳跃移动系统 ====================

/**
 * @brief 跳跃移动系统 - 管理跳跃冷却和执行跳跃
 */
class JumpMovementSystem : public ISystem {
public:
  const char *getName() const override { return "JumpMovementSystem"; }
  int getPriority() const override { return SystemPriority::MOVEMENT - 10; }

  void update(float delta) override {
    _world->forEach<JumpMovementComponent, GroundDetectorComponent,
                    AggroComponent, SlimeSpriteComponent, TransformComponent>(
        [delta, this](EntityId entity, JumpMovementComponent &jump,
                      GroundDetectorComponent &ground, AggroComponent &aggro,
                      SlimeSpriteComponent &sprite,
                      TransformComponent &transform) {
          // 只要在地面就计时冷却（不需要静止）
          // 通过法线检测已经确保只有真正的地面接触才会设置isOnGround
          if (ground.isOnGround) {
            jump.jumpTimer += delta;
            if (jump.jumpTimer >= jump.jumpCooldown) {
              jump.readyToJump = true;
            }
          }

          // 执行跳跃
          if (jump.readyToJump && ground.isOnGround) {
            cocos2d::Vec2 impulse;

            // 更新追踪状态
            jump.isChasing = aggro.hasAggro;

            if (aggro.hasAggro && aggro.targetEntity != INVALID_ENTITY) {
              // 有仇恨目标：智能追踪跳跃
              auto *targetTransform =
                  _world->getComponent<TransformComponent>(aggro.targetEntity);
              if (targetTransform) {
                float heightDiff =
                    targetTransform->position.y - transform.position.y;
                impulse = jump.calculateChaseImpulse(aggro.directionToTarget.x,
                                                     heightDiff,
                                                     aggro.distanceToTarget);

                // 更新朝向
                sprite.setFacing(aggro.directionToTarget.x > 0);
              }
            } else {
              // 无仇恨目标：随机巡逻跳跃
              impulse = jump.calculateRandomImpulse();
              sprite.setFacing(jump.randomDirection > 0);
            }

            // 应用冲量
            sprite.applyImpulse(impulse);
            jump.lastJumpImpulse = impulse;

            // 播放跳跃动画
            sprite.playJumpAnimation();

            // 重置状态
            jump.readyToJump = false;
            jump.jumpTimer = 0.0f;
            ground.isOnGround = false;

            CCLOG("Entity %u: Jump (%.1f, %.1f) %s", entity, impulse.x,
                  impulse.y, jump.isChasing ? "CHASE" : "PATROL");
          }
        });
  }
};

// ==================== 史莱姆渲染同步系统 ====================

/**
 * @brief 史莱姆同步系统 - 从精灵读取位置到Transform（物理引擎驱动精灵位置）
 */
class SlimeSyncSystem : public ISystem {
public:
  const char *getName() const override { return "SlimeSyncSystem"; }
  int getPriority() const override { return SystemPriority::RENDER - 10; }

  void update(float delta) override {
    _world->forEach<TransformComponent, SlimeSpriteComponent>(
        [delta](EntityId entity, TransformComponent &transform,
                SlimeSpriteComponent &sprite) {
          if (!sprite.sprite)
            return;

          // 物理引擎自动更新精灵位置，我们只需要读取它
          transform.position = sprite.sprite->getPosition();
        });
  }
};

// ==================== 行走移动系统 ====================

/**
 * @brief 行走移动系统 - 管理僵尸等行走类怪物的移动逻辑
 *
 * 特性:
 * - 持续行走而非跳跃移动
 * - 遇到障碍物卡住时自动跳跃
 * - 目标在高处时跳跃追击
 * - 目标跳起时延迟反应后跟着跳
 */
class WalkMovementSystem : public ISystem {
public:
  const char *getName() const override { return "WalkMovementSystem"; }
  int getPriority() const override { return SystemPriority::MOVEMENT; }

  void update(float delta) override {
    _world->forEach<WalkMovementComponent, GroundDetectorComponent,
                    AggroComponent, MonsterSpriteComponent, TransformComponent>(
        [delta, this](EntityId entity, WalkMovementComponent &walk,
                      GroundDetectorComponent &ground, AggroComponent &aggro,
                      MonsterSpriteComponent &sprite,
                      TransformComponent &transform) {
          
          // 检测落地（从空中到地面）
          if (ground.isOnGround && walk.isJumping) {
            walk.onLand(); // 激活跳跃冷却计时
          }
          
          // 更新跳跃冷却（只有在地面上且激活后才计时）
          if (ground.isOnGround) {
            walk.updateJumpCooldown(delta);
          }
          
          // 更新障碍物跳跃冷却（只有在地面上才计时）
          if (ground.isOnGround && walk.obstacleJumpTimer > 0) {
            walk.obstacleJumpTimer -= delta;
          }
          
          cocos2d::Vec2 currentVelocity = sprite.getVelocity();
          bool shouldJump = false;
          float targetHeightDiff = 0.0f;
          float horizontalDistToTarget = 9999.0f;
          
          // 首先计算与目标的水平距离（用于后续判断）
          if (aggro.hasAggro && aggro.targetEntity != INVALID_ENTITY) {
            auto *targetTransform =
                _world->getComponent<TransformComponent>(aggro.targetEntity);
            if (targetTransform) {
              horizontalDistToTarget = std::abs(targetTransform->position.x - transform.position.x);
            }
          }
          
          // 更新反应跳跃计时器
          if (walk.pendingReactionJump) {
            walk.targetJumpReactionTimer += delta;
            if (walk.targetJumpReactionTimer >= walk.targetJumpReactionTime) {
              walk.pendingReactionJump = false;
              walk.targetJumpReactionTimer = 0.0f;
              // 执行反应跳跃（只有在跳跃检测范围内才执行）
              if (ground.isOnGround && walk.canJump() && 
                  horizontalDistToTarget < walk.jumpDetectionRange) {
                executeJump(walk, sprite, ground);
                CCLOG("Entity %u: Reaction jump (target jumped)", entity);
              }
            }
          }
          
          // 确定移动方向和目标
          if (aggro.hasAggro && aggro.targetEntity != INVALID_ENTITY) {
            // 有仇恨目标：向目标移动
            auto *targetTransform =
                _world->getComponent<TransformComponent>(aggro.targetEntity);
            if (targetTransform) {
              
              // 设置移动方向（基于目标位置而非方向向量）
              float dirX = targetTransform->position.x - transform.position.x;
              if (std::abs(dirX) > 5.0f) { // 只有距离超过5像素时才改变方向
                walk.currentDirection = dirX > 0 ? 1 : -1;
              }
              
              // 计算高度差
              targetHeightDiff = targetTransform->position.y - transform.position.y;
              
              // 检测目标是否刚跳起来（只有在检测范围内才检测）
              if (walk.targetJumpEnabled && !walk.pendingReactionJump &&
                  horizontalDistToTarget < walk.jumpDetectionRange) {
                float targetCurrentY = targetTransform->position.y;
                float targetYDelta = targetCurrentY - walk.targetLastY;
                
                // 如果目标Y坐标突然上升超过阈值，说明目标跳了
                if (targetYDelta > 20.0f && walk.targetLastY > 0) {
                  walk.pendingReactionJump = true;
                  walk.targetJumpReactionTimer = 0.0f;
                  CCLOG("Entity %u: Detected target jump, will react in %.1fs", 
                        entity, walk.targetJumpReactionTime);
                }
              }
              walk.targetLastY = targetTransform->position.y;
              
              // 目标在高处，需要跳跃追击（只有在检测范围内才跳）
              if (targetHeightDiff > walk.targetHeightThreshold && 
                  ground.isOnGround && walk.canJump() &&
                  horizontalDistToTarget < walk.jumpDetectionRange) {
                shouldJump = true;
              }
            }
          } else {
            // 无仇恨目标：巡逻模式
            walk.patrolTimer += delta;
            if (walk.patrolTimer >= walk.patrolDirectionChangeInterval) {
              walk.patrolTimer = 0.0f;
              // 随机改变方向
              if ((float)rand() / RAND_MAX < walk.patrolDirectionChangeChance) {
                walk.patrolDirection *= -1;
              }
            }
            walk.currentDirection = walk.patrolDirection;
            sprite.setFacing(walk.currentDirection > 0);
            
            // 重置目标追踪状态
            walk.targetLastY = 0.0f;
            walk.pendingReactionJump = false;
          }
          
          // 检测障碍物卡住（传统位移检测）
          if (walk.shouldObstacleJump(transform.position, delta) && ground.isOnGround) {
            shouldJump = true;
            CCLOG("Entity %u: Obstacle detected (stuck), jumping", entity);
          }
          
          // 即时障碍物检测（通过速度差异）
          if (walk.useInstantObstacleDetection && ground.isOnGround && 
              walk.initialized && walk.canJump() && !shouldJump) {
            float actualVelX = std::abs(currentVelocity.x);
            float expectedVelX = walk.walkSpeed;
            
            // 如果在地面上，期望移动，但实际速度很小，说明被障碍物阻挡
            if (walk.isWalking && expectedVelX > 10.0f && 
                actualVelX < expectedVelX * walk.actualSpeedRatio) {
              shouldJump = true;
              CCLOG("Entity %u: Obstacle detected (speed diff), jumping", entity);
            }
          }
          
          // 执行跳跃
          if (shouldJump && ground.isOnGround && walk.canJump()) {
            executeJump(walk, sprite, ground);
          }
          
          // 应用水平移动和朝向（在地面上时）
          if (ground.isOnGround) {
            walk.expectedSpeed = walk.walkSpeed;
            currentVelocity.x = walk.walkSpeed * walk.currentDirection;
            sprite.setVelocity(currentVelocity);
            walk.isWalking = true;
            // 同步朝向（只有在地面上有水平速度时才更新朝向）
            if (std::abs(currentVelocity.x) > 1.0f) {
              // 僵尸贴图朝向为面向左侧（原始帧向左），为了让行走方向一致，在向右移动时需要水平翻转
              bool movingRight = currentVelocity.x > 0;
              sprite.setFacing(!movingRight);
            }
          }
          
          // 更新跳跃状态
          walk.isJumping = !ground.isOnGround;
        });
  }

private:
  void executeJump(WalkMovementComponent &walk, MonsterSpriteComponent &sprite,
                   GroundDetectorComponent &ground) {
    // 跳跃时保持当前水平速度
    cocos2d::Vec2 currentVel = sprite.getVelocity();
    cocos2d::Vec2 jumpVel(currentVel.x, walk.jumpForce);
    sprite.setVelocity(jumpVel);
    
    walk.onJump(); // 触发通用跳跃冷却
    walk.obstacleJumpTimer = walk.obstacleJumpCooldown;
    walk.stuckTime = 0.0f;
    ground.isOnGround = false;
    walk.isJumping = true;
  }
};

// ==================== 怪物精灵动画系统 ====================

/**
 * @brief 怪物精灵动画系统 - 更新MonsterSpriteComponent的帧动画
 */
class MonsterAnimationSystem : public ISystem {
public:
  const char *getName() const override { return "MonsterAnimationSystem"; }
  int getPriority() const override { return SystemPriority::ANIMATION; }

  void update(float delta) override {
    _world->forEach<MonsterSpriteComponent>(
        [delta](EntityId entity, MonsterSpriteComponent &sprite) {
          sprite.updateAnimation(delta);
        });
  }
};

// ==================== 怪物渲染同步系统 ====================

/**
 * @brief 怪物同步系统 - 同步Transform位置到精灵（从物理体读取）
 */
class MonsterSyncSystem : public ISystem {
public:
  const char *getName() const override { return "MonsterSyncSystem"; }
  int getPriority() const override { return SystemPriority::RENDER - 10; }

  void update(float delta) override {
    _world->forEach<TransformComponent, MonsterSpriteComponent>(
        [this, delta](EntityId entity, TransformComponent &transform,
                      MonsterSpriteComponent &sprite) {
          if (!sprite.sprite)
            return;

          auto *body = sprite.getPhysicsBody();
          if (!body)
            return;

          performRaycastCorrection(body, delta);
          // 从精灵位置同步到Transform（物理体驱动）
          transform.position = body->getPosition();
        });
  }
};

// ==================== 怪物地面检测系统 ====================

/**
 * @brief 怪物地面检测系统 - 根据物理体速度判断MonsterSpriteComponent的地面状态
 */
class MonsterGroundDetectorSystem : public ISystem {
public:
  const char *getName() const override { return "MonsterGroundDetectorSystem"; }
  int getPriority() const override { return SystemPriority::PHYSICS + 10; }

  void update(float delta) override {
    _world->forEach<GroundDetectorComponent, MonsterSpriteComponent>(
        [](EntityId entity, GroundDetectorComponent &ground,
           MonsterSpriteComponent &sprite) {
          cocos2d::Vec2 velocity = sprite.getVelocity();

          // 判断是否静止
          ground.isStill = std::abs(velocity.x) < ground.stillThreshold &&
                           std::abs(velocity.y) < ground.stillThreshold;
        });
  }
};

// ==================== 缓降系统 ====================

/**
 * @brief 缓降系统 - 处理伞史莱姆等下落时的空气阻力
 * 
 * 当实体下落时，限制其最大下落速度，模拟撑伞的效果
 */
class SlowFallSystem : public ISystem {
public:
  const char *getName() const override { return "SlowFallSystem"; }
  int getPriority() const override { return SystemPriority::PHYSICS + 1; }

  void update(float delta) override {
    _world->forEach<SlowFallComponent, SlimeSpriteComponent>(
        [](EntityId entity, SlowFallComponent &slowFall, 
           SlimeSpriteComponent &sprite) {
          if (!slowFall.isActive || !sprite.sprite)
            return;
          
          auto *body = sprite.sprite->getPhysicsBody();
          if (!body)
            return;
          
          cocos2d::Vec2 velocity = body->getVelocity();
          
          // 只在下落时应用缓降效果（velocity.y < 0 表示向下）
          if (velocity.y < 0) {
            bool modified = false;
            
            // 垂直方向：限制最大下落速度
            float maxFall = -slowFall.maxFallSpeed;
            if (velocity.y < maxFall) {
              // 应用阻尼，逐渐减速到最大下落速度
              velocity.y = velocity.y * slowFall.fallDamping;
              if (velocity.y < maxFall) {
                velocity.y = maxFall;
              }
              modified = true;
            }
            
            // 水平方向：施加空气阻力
            if (std::abs(velocity.x) > 10.0f) {
              velocity.x = velocity.x * slowFall.horizontalDamping;
              modified = true;
            }
            
            if (modified) {
              body->setVelocity(velocity);
            }
          }
        });
  }
};

// ==================== 投射物攻击系统 ====================

/**
 * @brief 投射物攻击系统 - 处理冰雪尖刺史莱姆等远程攻击
 * 
 * 当目标进入发射范围时，发射多个投射物
 * 投射物以抛物线轨迹飞行（受重力影响）
 */
class ProjectileAttackSystem : public ISystem {
public:
  const char *getName() const override { return "ProjectileAttackSystem"; }
  int getPriority() const override { return SystemPriority::AI + 10; }

  void update(float delta) override {
    _world->forEach<ProjectileAttackComponent, AggroComponent, 
                    TransformComponent, SlimeSpriteComponent,
                    GroundDetectorComponent, JumpMovementComponent>(
        [delta, this](EntityId entity, ProjectileAttackComponent &attack,
                      AggroComponent &aggro, TransformComponent &transform,
                      SlimeSpriteComponent &sprite, GroundDetectorComponent &ground,
                      JumpMovementComponent &jump) {
          
          // 更新发射计时器
          if (!attack.canFire) {
            attack.fireTimer += delta;
            if (attack.fireTimer >= attack.fireInterval) {
              attack.canFire = true;
              attack.fireTimer = 0.0f;
            }
          }
          
          // 检查目标是否在发射范围内
          attack.targetInRange = aggro.hasAggro && 
                                  aggro.distanceToTarget <= attack.fireRange;
          
          // 如果目标在范围内，阻止跳跃并尝试发射
          if (attack.targetInRange) {
            // 阻止跳跃 - 重置跳跃计时器
            jump.readyToJump = false;
            jump.jumpTimer = 0.0f;
            
            // 更新朝向
            if (aggro.directionToTarget.x != 0) {
              sprite.setFacing(aggro.directionToTarget.x > 0);
            }
            
            // 只有在地面上且可以发射时才发射
            if (attack.canFire && ground.isOnGround) {
              fireProjectiles(entity, attack, transform, aggro, sprite);
              attack.canFire = false;
              attack.fireTimer = 0.0f;
            }
          }
        });
  }

private:
  void fireProjectiles(EntityId owner, ProjectileAttackComponent &attack,
                       TransformComponent &transform, AggroComponent &aggro,
                       SlimeSpriteComponent &ownerSprite) {
    
    cocos2d::Node* parentNode = ownerSprite.sprite ? 
                                 ownerSprite.sprite->getParent() : nullptr;
    if (!parentNode) return;
    
    // 发射方向固定为正上方散开，不指向仇恨目标
    // 基础角度90度（正上方），左右对称散开
    float baseAngle = 90.0f; // 正上方
    float spreadStep = attack.projectileCount > 1 ? 
                       attack.horizontalSpread / (attack.projectileCount - 1) : 0;
    float startAngle = baseAngle - attack.horizontalSpread / 2.0f;
    
    for (int i = 0; i < attack.projectileCount; i++) {
      float angle = startAngle + spreadStep * i;
      float radians = angle * M_PI / 180.0f;
      
      // 计算初始速度
      cocos2d::Vec2 velocity;
      velocity.x = cos(radians) * attack.projectileSpeed;
      velocity.y = sin(radians) * attack.verticalImpulse;
      
      // 创建投射物实体
      EntityId projectile = _world->createEntity();
      
      // 添加变换组件
      auto &projTransform = _world->addComponent<TransformComponent>(projectile);
      projTransform.position = transform.position;
      projTransform.velocity = velocity;
      
      // 添加投射物组件
      auto &projComp = _world->addComponent<ProjectileComponent>(projectile);
      projComp.owner = owner;
      projComp.damage = attack.projectileDamage;
      projComp.lifetime = attack.projectileLifetime;
      // 冰系减益
      projComp.chillChance = attack.chillChance;
      projComp.chillDuration = attack.chillDuration;
      projComp.chillSpeedReduction = attack.chillSpeedReduction;
      projComp.freezeChance = attack.freezeChance;
      projComp.freezeDuration = attack.freezeDuration;
      // 毒系减益
      projComp.poisonChance1 = attack.poisonChance1;
      projComp.poisonDuration1 = attack.poisonDuration1;
      projComp.poisonDamage1 = attack.poisonDamage1;
      projComp.poisonChance2 = attack.poisonChance2;
      projComp.poisonDuration2 = attack.poisonDuration2;
      projComp.poisonDamage2 = attack.poisonDamage2;
      
      // 创建投射物精灵
      auto &projSprite = _world->addComponent<ProjectileSpriteComponent>(projectile);
      projSprite.sprite = cocos2d::Sprite::create(attack.projectileSpritePath);
      
      if (projSprite.sprite) {
        projSprite.sprite->retain();
        
        // 根据物理体尺寸计算缩放
        float scaleX = attack.projectileSpriteWidth / 
                       projSprite.sprite->getContentSize().width;
        float scaleY = attack.projectileSpriteHeight / 
                       projSprite.sprite->getContentSize().height;
        projSprite.sprite->setScale(scaleX, scaleY);
        
        projSprite.sprite->setPosition(transform.position);
        parentNode->addChild(projSprite.sprite, 2);
        
        // 设置物理体
        cocos2d::PhysicsMaterial material(0.1f, 0.0f, 0.0f);
        auto body = cocos2d::PhysicsBody::createBox(
            cocos2d::Size(attack.projectileSpriteWidth, attack.projectileSpriteHeight),
            material);
        body->setDynamic(true);
        body->setMass(0.1f);
        body->setGravityEnable(attack.useGravity);
        body->setRotationEnable(true);
        body->setVelocity(velocity);
        body->setContactTestBitmask(0xFFFFFFFF); // 检测所有碰撞事件
        body->setCollisionBitmask(0x0001);       // 只和地形(静态物体)产生物理碰撞
        body->setCategoryBitmask(0x0004);        // 投射物类别
        body->setGroup(-2);  // 负数组，和敌人组(-1)不同，不产生物理碰撞
        projSprite.sprite->setPhysicsBody(body);
        
        // 根据速度方向设置旋转
        float rotAngle = atan2(velocity.y, velocity.x) * 180.0f / M_PI;
        projSprite.sprite->setRotation(-rotAngle + 90.0f); // 尖刺朝向飞行方向
        
        // 注册到 NodeEntityMap 以便碰撞检测
        NodeEntityMap::getInstance().registerNode(projSprite.sprite, projectile);
      }
      
      CCLOG("Fired Ice Spike %d at angle %.1f, velocity (%.1f, %.1f)", 
            i, angle, velocity.x, velocity.y);
    }
    
    CCLOG("Entity %u: Fired %d ice spikes!", owner, attack.projectileCount);
  }
};

// ==================== 投射物更新系统 ====================

/**
 * @brief 投射物系统 - 更新投射物位置、旋转和生命周期
 */
class ProjectileSystem : public ISystem {
public:
  const char *getName() const override { return "ProjectileSystem"; }
  int getPriority() const override { return SystemPriority::PHYSICS + 5; }

  void update(float delta) override {
    std::vector<EntityId> toDestroy;
    
    _world->forEach<ProjectileComponent, ProjectileSpriteComponent, TransformComponent>(
        [delta, &toDestroy, this](EntityId entity, ProjectileComponent &proj,
                                   ProjectileSpriteComponent &sprite,
                                   TransformComponent &transform) {
          // 更新生命周期
          proj.lifetime -= delta;
          if (proj.lifetime <= 0 || proj.hasHit) {
            toDestroy.push_back(entity);
            return;
          }
          
          // 从物理体同步位置和旋转
          if (sprite.sprite && sprite.sprite->getPhysicsBody()) {
            transform.position = sprite.sprite->getPosition();
            cocos2d::Vec2 velocity = sprite.sprite->getPhysicsBody()->getVelocity();
            
            // 根据速度方向更新旋转
            if (velocity.lengthSquared() > 1.0f) {
              float rotAngle = atan2(velocity.y, velocity.x) * 180.0f / M_PI;
              sprite.sprite->setRotation(-rotAngle + 90.0f);
            }
          }
        });
    
    // 销毁过期的投射物
    for (EntityId entity : toDestroy) {
      _world->destroyEntity(entity);
    }
  }
};

// ==================== 减益效果系统 ====================

/**
 * @brief 减益效果系统 - 处理冷冻、冰冻和中毒减益的计时
 */
class DebuffSystem : public ISystem {
public:
  const char *getName() const override { return "DebuffSystem"; }
  int getPriority() const override { return SystemPriority::AI - 5; }

  void update(float delta) override {
    _world->forEach<DebuffComponent>(
        [delta, this](EntityId entity, DebuffComponent &debuff) {
          // 更新冷冻计时器
          if (debuff.hasChillDebuff) {
            debuff.chillTimer += delta;
            if (debuff.chillTimer >= debuff.chillDuration) {
              debuff.hasChillDebuff = false;
              CCLOG("Entity %u: Chill debuff expired", entity);
            }
          }
          
          // 更新冰冻计时器
          if (debuff.hasFreezeDebuff) {
            debuff.freezeTimer += delta;
            if (debuff.freezeTimer >= debuff.freezeDuration) {
              debuff.hasFreezeDebuff = false;
              CCLOG("Entity %u: Freeze debuff expired", entity);
            }
          }
          
          // 更新中毒计时器和伤害
          if (debuff.hasPoisonDebuff) {
            debuff.poisonTimer += delta;
            debuff.poisonTickTimer += delta;
            
            // 每秒造成一次伤害
            if (debuff.poisonTickTimer >= 1.0f) {
              debuff.poisonTickTimer -= 1.0f;
              
              // 对实体造成毒素伤害
              auto *health = _world->getComponent<HealthComponent>(entity);
              if (health) {
                health->takeDamage(debuff.poisonDamagePerSecond);
                CCLOG("Entity %u: Poison tick %.1f damage (%.1fs remaining)", 
                      entity, debuff.poisonDamagePerSecond,
                      debuff.poisonDuration - debuff.poisonTimer);
              }
            }
            
            if (debuff.poisonTimer >= debuff.poisonDuration) {
              debuff.hasPoisonDebuff = false;
              CCLOG("Entity %u: Poison debuff expired", entity);
            }
          }
        });
  }
};

} // namespace ecs

#endif // __ECS_SYSTEMS_H__
