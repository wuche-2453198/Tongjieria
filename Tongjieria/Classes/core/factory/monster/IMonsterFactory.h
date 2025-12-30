#ifndef __ECS_IMONSTERFACTORY_H__
#define __ECS_IMONSTERFACTORY_H__

#include "components/Entity.h"
#include <entt/entt.hpp>
#include <string>
#include <vector>

namespace cocos2d {
class Node;
}

struct MonsterConfig;

/**
 * @class IMonsterFactory
 * @brief 怪物工厂接口 - 定义所有专业工厂的通用契约
 * 
 * 所有专业工厂（SlimeFactory、ZombieFactory等）必须实现此接口。
 * 提供统一的怪物创建、配置查询和支持检查方法。
 */
class IMonsterFactory {
public:
  virtual ~IMonsterFactory() = default;

  /**
   * @brief 检查是否支持指定怪物ID
   * @param monsterId 怪物ID (如 "GreenSlime")
   * @return 是否支持
   */
  virtual bool supports(const std::string &monsterId) const = 0;

  /**
   * @brief 创建怪物实体
   * @param registry EnTT注册表
   * @param monsterId 怪物ID
   * @param x X坐标
   * @param y Y坐标
   * @param parentNode 父节点（可选）
   * @return 实体ID，失败返回 INVALID_ENTITY
   */
  virtual ecs::EntityId create(entt::registry &registry,
                               const std::string &monsterId,
                               float x,
                               float y,
                               cocos2d::Node *parentNode) = 0;

  /**
   * @brief 获取怪物配置
   * @param monsterId 怪物ID
   * @return 配置指针，不存在返回nullptr
   */
  virtual const MonsterConfig *getConfig(const std::string &monsterId) const {
    (void)monsterId;
    return nullptr;
  }

  /**
   * @brief 获取支持的所有怪物ID
   * @return 怪物ID列表
   */
  virtual std::vector<std::string> getSupportedIds() const = 0;
};

#endif
