#ifndef __EATER_FACTORY_H__
#define __EATER_FACTORY_H__

#include "BaseMonsterFactory.h"

/**
 * @class EaterFactory
 * @brief 噬魂怪/血腥怪专业工厂 - 处理所有噬魂怪和猩红喀迈拉类型的创建
 * 
 * 支持的怪物类型：
 * - EaterOfSouls_Small, EaterOfSouls_Medium, EaterOfSouls_Large
 * - Crimera_Small, Crimera_Medium, Crimera_Large
 * 
 * 组件附加顺序（优化内存布局）：
 * 1. 核心组件（TransformComponent, EntityStateFlags）
 * 2. 物理组件（PhysicsBodyComponent）- 矩形物理体，无重力
 * 3. 渲染组件（RenderComponent, ParentNodeComponent, AnimationComponent）
 * 4. 战斗组件（HealthComponent, AggroComponent）
 * 5. 移动组件（EaterOfSoulsMovementComponent）- 绕圈飞行+冲刺
 * 
 * 注意：噬魂怪不需要地面检测组件（GroundDetectorComponent）
 */
class EaterFactory : public BaseMonsterFactory {
public:
  /**
   * @brief 构造函数 - 加载 config/eaters 目录配置
   */
  EaterFactory();

  /**
   * @brief 创建噬魂怪/血腥怪实体
   * @param registry EnTT注册表
   * @param monsterId 怪物ID（如 "EaterOfSouls_Medium"）
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
   * @brief 附加绕圈飞行组件（EaterOfSoulsMovementComponent）
   * 配置飞行速度、绕圈半径、冲刺参数等
   * 根据怪物ID判断尺寸变体 (Small/Medium/Large)
   */
  void attachCircleFlightComponents(entt::registry &registry, entt::entity entity,
                                    const std::string &monsterId,
                                    const MonsterConfig &config);

  /**
   * @brief 配置无重力矩形物理体
   * 噬魂怪使用矩形物理体，无重力，有弹性
   */
  void configurePhysicsForFlying(entt::registry &registry, entt::entity entity,
                                 const MonsterConfig &config);
};

#endif // __EATER_FACTORY_H__
