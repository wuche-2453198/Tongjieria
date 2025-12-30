#ifndef __DEMONEYE_FACTORY_H__
#define __DEMONEYE_FACTORY_H__

#include "BaseMonsterFactory.h"

/**
 * @class DemonEyeFactory
 * @brief 恶魔眼专业工厂 - 处理所有恶魔眼类型的创建
 * 
 * 支持的怪物类型：
 * - DemonEye, Bigger-DemonEye
 * - PurpleEye, GreenEye
 * - CataractEye, DilatedEye
 * 
 * 组件附加顺序（优化内存布局）：
 * 1. 核心组件（TransformComponent, EntityStateFlags）
 * 2. 物理组件（PhysicsBodyComponent）- 圆形物理体，无重力
 * 3. 渲染组件（RenderComponent, ParentNodeComponent, AnimationComponent）
 * 4. 战斗组件（HealthComponent, AggroComponent）
 * 5. 移动组件（DemonEyeMovementComponent）- 飞行追踪
 * 
 * 注意：恶魔眼不需要地面检测组件（GroundDetectorComponent）
 */
class DemonEyeFactory : public BaseMonsterFactory {
public:
  /**
   * @brief 构造函数 - 加载 config/eyes 目录配置
   */
  DemonEyeFactory();

  /**
   * @brief 创建恶魔眼实体
   * @param registry EnTT注册表
   * @param monsterId 怪物ID（如 "DemonEye"）
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
   * @brief 附加飞行组件（DemonEyeMovementComponent）
   * 配置飞行速度、转向率、摆动效果等
   */
  void attachFlyingComponents(entt::registry &registry, entt::entity entity,
                              const MonsterConfig &config);

  /**
   * @brief 配置无重力圆形物理体
   * 恶魔眼使用圆形物理体，无重力，有弹性
   */
  void configurePhysicsForFlying(entt::registry &registry, entt::entity entity,
                                 const MonsterConfig &config);
};

#endif // __DEMONEYE_FACTORY_H__
