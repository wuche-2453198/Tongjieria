#ifndef __VULTURE_FACTORY_H__
#define __VULTURE_FACTORY_H__

#include "BaseMonsterFactory.h"

/**
 * @class VultureFactory
 * @brief 秃鹰专业工厂 - 处理秃鹰类型怪物的创建
 * 
 * 支持的怪物类型：
 * - Vulture（栖息形态）
 * - FlyingVulture（飞行形态）
 * 
 * 秃鹰特性：
 * - 初始栖息在地面，玩家靠近或受击后升空
 * - 飞行时在玩家上方盘旋
 * - 当玩家在正下方时猛扑攻击
 * - 状态驱动动画（idle, hovering, dashing）
 * 
 * 组件附加顺序（优化内存布局）：
 * 1. 核心组件（TransformComponent, EntityStateFlags）
 * 2. 物理组件（PhysicsBodyComponent）- 栖息时有重力，飞行时无重力
 * 3. 渲染组件（RenderComponent, ParentNodeComponent, AnimationComponent）
 * 4. 战斗组件（HealthComponent, AggroComponent）
 * 5. 移动组件（VultureMovementComponent）- 飞行猛扑行为
 * 6. 动画状态组件（AnimationStateComponent）- 状态驱动动画
 */
class VultureFactory : public BaseMonsterFactory {
public:
  /**
   * @brief 构造函数 - 加载 config/desert/Vulture.json 配置
   */
  VultureFactory();

  /**
   * @brief 创建秃鹰实体
   * @param registry EnTT注册表
   * @param monsterId 怪物ID（"Vulture" 或 "FlyingVulture"）
   * @param x X坐标
   * @param y Y坐标
   * @param parentNode 父节点（可选）
   * @return 实体ID，失败返回 INVALID_ENTITY
   */
  ecs::EntityId create(entt::registry &registry,
                       const std::string &monsterId,
                       float x, float y,
                       cocos2d::Node *parentNode) override;

  /**
   * @brief 检查是否支持指定怪物ID
   * 支持 "Vulture" 和 "FlyingVulture"
   */
  bool supports(const std::string &monsterId) const override;

  /**
   * @brief 获取支持的所有怪物ID
   */
  std::vector<std::string> getSupportedIds() const override;

  /**
   * @brief 创建飞行形态的秃鹰
   * 用于栖息秃鹰被激活后转换为飞行形态
   */
  ecs::EntityId createFlyingVulture(entt::registry &registry,
                                    float x, float y,
                                    cocos2d::Node *parentNode);

private:
  /**
   * @brief 附加秃鹰专用组件
   * 配置 VultureMovementComponent 和 AnimationStateComponent
   * - 飞行猛扑行为
   * - 状态驱动动画（idle, hovering, dashing）
   */
  void attachVultureComponents(entt::registry &registry, entt::entity entity,
                               const MonsterConfig &config, bool isFlying);

  /**
   * @brief 配置栖息形态物理体
   * 矩形物理体，有重力，落地后静止
   */
  void configurePhysicsForIdle(entt::registry &registry, entt::entity entity,
                               const MonsterConfig &config);

  /**
   * @brief 配置飞行形态物理体
   * 矩形物理体，无重力，低阻尼
   */
  void configurePhysicsForFlying(entt::registry &registry, entt::entity entity,
                                 const MonsterConfig &config);

  /**
   * @brief 注册秃鹰动画资源
   * 注册 vulture_idle 和 vulture_fly 资源
   */
  void registerVultureAnimationResources(const MonsterConfig &config,
                                         const std::vector<std::string> &framePaths);
};

#endif // __VULTURE_FACTORY_H__
