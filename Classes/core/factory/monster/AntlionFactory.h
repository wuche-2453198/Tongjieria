#ifndef __ANTLION_FACTORY_H__
#define __ANTLION_FACTORY_H__

#include "BaseMonsterFactory.h"

/**
 * @class AntlionFactory
 * @brief 蚁狮专业工厂 - 处理蚁狮类型怪物的创建
 * 
 * 支持的怪物类型：
 * - Antlion
 * 
 * 蚁狮特性：
 * - 静止不动，头部伸出地面
 * - 头部会朝向玩家
 * - 定期向玩家发射沙球射弹
 * - 只能向上45度角范围内射击
 * 
 * 组件附加顺序（优化内存布局）：
 * 1. 核心组件（TransformComponent, EntityStateFlags）
 * 2. 物理组件（PhysicsBodyComponent）- 圆形物理体，有重力，高摩擦力，锁定旋转
 * 3. 渲染组件（RenderComponent, ParentNodeComponent, AnimationComponent）
 * 4. 战斗组件（HealthComponent, AggroComponent）
 * 5. 移动组件（AntlionMovementComponent）- 静止射击行为
 * 6. 动画状态组件（AnimationStateComponent）- 状态驱动动画
 */
class AntlionFactory : public BaseMonsterFactory {
public:
  /**
   * @brief 构造函数 - 加载 config/desert/Antlion.json 配置
   */
  AntlionFactory();

  /**
   * @brief 创建蚁狮实体
   * @param registry EnTT注册表
   * @param monsterId 怪物ID（"Antlion"）
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
   * @brief 附加蚁狮专用组件
   * 配置 AntlionMovementComponent 和 AnimationStateComponent
   * - 静止射击行为
   * - 状态驱动动画（idle, tracking, shooting）
   */
  void attachAntlionComponents(entt::registry &registry, entt::entity entity,
                               const MonsterConfig &config);

  /**
   * @brief 配置蚁狮物理体
   * 圆形物理体，有重力，高摩擦力，锁定旋转
   */
  void configurePhysicsForAntlion(entt::registry &registry, entt::entity entity,
                                  const MonsterConfig &config);
};

#endif // __ANTLION_FACTORY_H__
