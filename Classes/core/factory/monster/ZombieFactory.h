#ifndef __ZOMBIE_FACTORY_H__
#define __ZOMBIE_FACTORY_H__

#include "BaseMonsterFactory.h"

/**
 * @class ZombieFactory
 * @brief 僵尸专业工厂 - 处理所有僵尸类型的创建
 * 
 * 支持的怪物类型：
 * - Zombie, 31px-Zombie, Bigger-Zombie
 * - BaldZombie, 29px-BaldZombie, Bigger-BaldZombie
 * - PincushionZombie, 32px-PincushionZombie, Bigger-PincushionZombie
 * 
 * 组件附加顺序（优化内存布局）：
 * 1. 核心组件（TransformComponent, EntityStateFlags）
 * 2. 物理组件（PhysicsBodyComponent）
 * 3. 渲染组件（RenderComponent, ParentNodeComponent, AnimationComponent）
 * 4. 战斗组件（HealthComponent, AggroComponent）
 * 5. 地面检测组件（GroundDetectorComponent）
 * 6. 移动组件（WarriorMovementComponent）
 */
class ZombieFactory : public BaseMonsterFactory {
public:
  /**
   * @brief 构造函数 - 加载 config/zombies 目录配置
   */
  ZombieFactory();

  /**
   * @brief 创建僵尸实体
   * @param registry EnTT注册表
   * @param monsterId 怪物ID（如 "Zombie"）
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
   * @brief 附加僵尸移动组件（WarriorMovementComponent）
   */
  void attachZombieMovement(entt::registry &registry, entt::entity entity,
                            const MonsterConfig &config);
};

#endif // __ZOMBIE_FACTORY_H__
