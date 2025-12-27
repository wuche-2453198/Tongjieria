#ifndef __KING_SLIME_FACTORY_H__
#define __KING_SLIME_FACTORY_H__

#include "BaseMonsterFactory.h"

/**
 * @class KingSlimeFactory
 * @brief 史莱姆王Boss专业工厂 - 处理KingSlime Boss的创建
 * 
 * 支持的怪物类型：
 * - KingSlime（史莱姆王Boss）
 * 
 * Boss特有机制：
 * - 传送系统：定时传送到玩家附近
 * - 生成小史莱姆：根据血量损失生成小史莱姆
 * - 缩放阶段变化：血量越低体型越小，但攻击越快
 * - 跳跃模式：小跳+大跳循环
 * 
 * 组件附加顺序（优化内存布局）：
 * 1. 核心组件（TransformComponent, EntityStateFlags）
 * 2. 物理组件（PhysicsBodyComponent）- 更高的速度限制
 * 3. 渲染组件（RenderComponent, ParentNodeComponent, AnimationComponent）
 * 4. 战斗组件（HealthComponent, AggroComponent）
 * 5. 地面检测组件（GroundDetectorComponent）
 * 6. 移动组件（JumpMovementComponent）
 * 7. Boss组件（KingSlimeComponent）
 */
class KingSlimeFactory : public BaseMonsterFactory {
public:
  /**
   * @brief 构造函数 - 加载 config/bosses/KingSlime.json 配置
   */
  KingSlimeFactory();

  /**
   * @brief 创建史莱姆王实体
   * @param registry EnTT注册表
   * @param monsterId 怪物ID（应为 "KingSlime"）
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
   * @brief 附加史莱姆王移动组件（JumpMovementComponent）
   * 配置跳跃参数，支持小跳+大跳模式
   */
  void attachKingSlimeMovement(entt::registry &registry, entt::entity entity,
                               const MonsterConfig &config);

  /**
   * @brief 附加史莱姆王Boss组件（KingSlimeComponent）
   * 配置传送、生成小史莱姆、缩放阶段变化等Boss机制
   */
  void attachKingSlimeComponents(entt::registry &registry, entt::entity entity,
                                 const MonsterConfig &config);
};

#endif // __KING_SLIME_FACTORY_H__
