#ifndef __ECS_MONSTERMASTERFACTORY_H__
#define __ECS_MONSTERMASTERFACTORY_H__

#include "core/factory/monster/ConfigCache.h"
#include "core/factory/monster/MonsterEntityPool.h"
#include "core/factory/monster/IMonsterFactory.h"
#include "core/factory/monster/SlimeFactory.h"
#include "core/factory/monster/ZombieFactory.h"
#include "core/factory/monster/DemonEyeFactory.h"
#include "core/factory/monster/DemonFactory.h"
#include "core/factory/monster/EaterFactory.h"
#include "core/factory/monster/AntlionFactory.h"
#include "core/factory/monster/VultureFactory.h"
#include "core/factory/monster/KingSlimeFactory.h"
#include "core/factory/monster/BatFactory.h"
#include "core/factory/monster/SkeletonFactory.h"
#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class MonsterMasterFactory {
public:
  static MonsterMasterFactory &getInstance();

  MonsterMasterFactory(const MonsterMasterFactory &) = delete;
  MonsterMasterFactory &operator=(const MonsterMasterFactory &) = delete;

  void registerFactory(std::unique_ptr<IMonsterFactory> factory,
                       int priority = 0);
  void unregisterFactory(IMonsterFactory *factoryPtr);
  void clearFactories();
  void registerDefaultFactories();

  bool supports(const std::string &monsterId) const;

  ecs::EntityId createMonster(entt::registry &registry,
                              const std::string &monsterId,
                              float x,
                              float y,
                              cocos2d::Node *parentNode = nullptr);

  std::vector<ecs::EntityId> createMonsterBatch(entt::registry &registry,
                                                const std::string &monsterId,
                                                int count,
                                                float x,
                                                float y,
                                                cocos2d::Node *parentNode = nullptr);

  void preallocateEntities(entt::registry &registry, size_t count);

  const MonsterConfig *getConfig(const std::string &monsterId) const;
  std::vector<std::string> getAllSupportedMonsterIds() const;
  bool isSupported(const std::string &monsterId) const;

  // 配置加载/缓存辅助
  bool loadConfigFile(const std::string &filePath);
  int preloadConfigDirectory(const std::string &dirPath);

private:
  MonsterMasterFactory();

  IMonsterFactory *findFactory(const std::string &monsterId) const;

  struct FactoryEntry {
    int priority = 0;
    std::unique_ptr<IMonsterFactory> factory;
  };

  std::vector<FactoryEntry> _factories;
};

#endif
