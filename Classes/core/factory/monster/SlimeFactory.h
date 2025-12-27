#ifndef __SLIME_FACTORY_H__
#define __SLIME_FACTORY_H__

#include "BaseMonsterFactory.h"

/**
 * @class SlimeFactory
 * @brief 史莱姆专业工厂 - 处理所有普通史莱姆类型的创建
 * 
 * 支持的怪物类型：
 * - GreenSlime, BlueSlime, RedSlime, YellowSlime, PurpleSlime, PinkSlime
 * - IceSlime
 * - SpikedSlime, SpikedIceSlime, SpikedJungleSlime
 * - UmbrellaSlime
 * - MotherSlime, BabySlime
 * 
 * 注意：KingSlime 由 KingSlimeFactory 处理
 * 
 * 组件附加顺序（优化内存布局）：
 * 1. 核心组件（TransformComponent, EntityStateFlags）
 * 2. 物理组件（PhysicsBodyComponent）
 * 3. 渲染组件（RenderComponent, ParentNodeComponent, AnimationComponent）
 * 4. 战斗组件（HealthComponent, AggroComponent）
 * 5. 地面检测组件（GroundDetectorComponent）
 * 6. 移动组件（JumpMovementComponent）
 * 7. 特殊组件（SlowFallComponent, ProjectileAttackComponent）
 */
class SlimeFactory : public BaseMonsterFactory {
public:
  /**
   * @brief 构造函数 - 加载 config/slimes 目录配置（排除 KingSlime）
   */
  SlimeFactory();

  /**
   * @brief 创建史莱姆实体
   * @param registry EnTT注册表
   * @param monsterId 怪物ID（如 "GreenSlime"）
   * @param x X坐标
   * @param y Y坐标
   * @param parentNode 父节点（可选）
   * @return 实体ID，失败返回 INVALID_ENTITY
   */
  ecs::EntityId create(entt::registry &registry,
                       const std::string &monsterId,
                       float x, float y,
                       cocos2d::Node *parentNode) override;

private:
  /**
   * @brief 附加史莱姆移动组件（JumpMovementComponent）
   */
  void attachSlimeMovement(entt::registry &registry, entt::entity entity,
                           const MonsterConfig &config);

  /**
   * @brief 附加特殊史莱姆组件
   * 处理 UmbrellaSlime（缓降）和 SpikedSlime（投射物攻击）等特殊能力
   */
  void attachSpecialSlimeComponents(entt::registry &registry, entt::entity entity,
                                    const std::string &monsterId,
                                    const MonsterConfig &config);
};

#endif // __SLIME_FACTORY_H__
