#ifndef __BASE_MONSTER_FACTORY_H__
#define __BASE_MONSTER_FACTORY_H__

#include "IMonsterFactory.h"
#include "core/factory/MonsterConfig.h"
#include <unordered_map>
#include <string>
#include <vector>

namespace cocos2d {
class Node;
}

/**
 * @class BaseMonsterFactory
 * @brief 怪物工厂基类 - 提供通用接口和辅助方法
 * 
 * 所有专业工厂（SlimeFactory、ZombieFactory等）继承此类。
 * 提供：
 * - 配置加载方法
 * - 通用组件附加方法（按优化顺序）
 * - 精灵资源注册
 * 
 * 组件附加顺序（优化内存布局）：
 * 1. TransformComponent, EntityStateFlags（高频访问）
 * 2. PhysicsBodyComponent（物理系统）
 * 3. RenderComponent, ParentNodeComponent, AnimationComponent（渲染系统）
 * 4. HealthComponent, AggroComponent（战斗系统）
 */
class BaseMonsterFactory : public IMonsterFactory {
public:
  virtual ~BaseMonsterFactory() = default;

  // ==================== IMonsterFactory 接口实现 ====================
  
  /**
   * @brief 检查是否支持指定怪物ID
   */
  bool supports(const std::string &monsterId) const override;

  /**
   * @brief 获取怪物配置
   */
  const MonsterConfig *getConfig(const std::string &monsterId) const override;

  /**
   * @brief 获取支持的所有怪物ID
   */
  std::vector<std::string> getSupportedIds() const override;

protected:
  // ==================== 配置加载方法 ====================
  
  /**
   * @brief 从目录加载所有配置文件
   * @param dirPath 目录路径（相对于Resources）
   * @return 加载成功的数量
   */
  int loadConfigs(const std::string &dirPath);

  /**
   * @brief 加载单个配置文件
   * @param filePath 文件路径（相对于Resources）
   * @return 是否加载成功
   */
  bool loadSingleConfig(const std::string &filePath);

  // ==================== 通用组件附加方法 ====================
  
  /**
   * @brief 附加核心组件（Transform, EntityStateFlags）
   * 高频访问组件，优先附加
   */
  void attachCoreComponents(entt::registry &registry, entt::entity entity,
                            const MonsterConfig &config, float x, float y);

  /**
   * @brief 附加物理组件（PhysicsBodyComponent）
   */
  void attachPhysicsComponents(entt::registry &registry, entt::entity entity,
                               const MonsterConfig &config);

  /**
   * @brief 附加渲染组件（RenderComponent, ParentNodeComponent, AnimationComponent）
   */
  void attachRenderComponents(entt::registry &registry, entt::entity entity,
                              const MonsterConfig &config,
                              cocos2d::Node *parentNode);

  /**
   * @brief 附加战斗组件（HealthComponent, AggroComponent）
   */
  void attachCombatComponents(entt::registry &registry, entt::entity entity,
                              const MonsterConfig &config);

  /**
   * @brief 注册精灵资源到SpriteManager
   * @param config 怪物配置
   * @return 资源ID
   */
  std::string registerSpriteResource(const MonsterConfig &config);

  // ==================== 成员变量 ====================
  
  /** 配置存储（monsterId -> MonsterConfig） */
  std::unordered_map<std::string, MonsterConfig> _configs;

  /** 怪物类型标识（子类设置，如 "Slime", "Zombie"） */
  std::string _monsterType;
};

#endif // __BASE_MONSTER_FACTORY_H__
