#include "MonsterMasterFactory.h"
#include "components/AllComponents.h"

MonsterMasterFactory &MonsterMasterFactory::getInstance() {
  static MonsterMasterFactory instance;
  return instance;
}

MonsterMasterFactory::MonsterMasterFactory() { registerDefaultFactories(); }

void MonsterMasterFactory::registerFactory(std::unique_ptr<IMonsterFactory> factory,
                                           int priority) {
  if (!factory) {
    return;
  }
  _factories.push_back(FactoryEntry{priority, std::move(factory)});
  std::stable_sort(_factories.begin(), _factories.end(),
                   [](const FactoryEntry &a, const FactoryEntry &b) {
                     return a.priority > b.priority;
                   });
}

void MonsterMasterFactory::unregisterFactory(IMonsterFactory *factoryPtr) {
  if (!factoryPtr) {
    return;
  }
  _factories.erase(std::remove_if(_factories.begin(), _factories.end(),
                                  [factoryPtr](const FactoryEntry &entry) {
                                    return entry.factory.get() == factoryPtr;
                                  }),
                   _factories.end());
}

void MonsterMasterFactory::clearFactories() { _factories.clear(); }

void MonsterMasterFactory::registerDefaultFactories() {
  clearFactories();
  registerFactory(std::make_unique<KingSlimeFactory>(), 120);
  registerFactory(std::make_unique<VultureFactory>(), 110);
  registerFactory(std::make_unique<AntlionFactory>(), 105);
  registerFactory(std::make_unique<SkeletonFactory>(), 104);
  registerFactory(std::make_unique<BatFactory>(), 103);
  registerFactory(std::make_unique<EaterFactory>(), 102);
  registerFactory(std::make_unique<DemonFactory>(), 101);
  registerFactory(std::make_unique<DemonEyeFactory>(), 101);
  registerFactory(std::make_unique<ZombieFactory>(), 100);
  registerFactory(std::make_unique<SlimeFactory>(), 95);
}

bool MonsterMasterFactory::supports(const std::string &monsterId) const {
  return findFactory(monsterId) != nullptr;
}

IMonsterFactory *MonsterMasterFactory::findFactory(
    const std::string &monsterId) const {
  for (const auto &entry : _factories) {
    if (entry.factory && entry.factory->supports(monsterId)) {
      return entry.factory.get();
    }
  }
  return nullptr;
}

ecs::EntityId MonsterMasterFactory::createMonster(entt::registry &registry,
                                                  const std::string &monsterId,
                                                  float x,
                                                  float y,
                                                  cocos2d::Node *parentNode) {
  auto *factory = findFactory(monsterId);
  if (!factory) {
    CCLOG("MonsterMasterFactory: Unsupported monster ID: %s",
          monsterId.c_str());
    return ecs::INVALID_ENTITY;
  }
  return factory->create(registry, monsterId, x, y, parentNode);
}

std::vector<ecs::EntityId> MonsterMasterFactory::createMonsterBatch(
    entt::registry &registry,
    const std::string &monsterId,
    int count,
    float x,
    float y,
    cocos2d::Node *parentNode) {
  std::vector<ecs::EntityId> result;
  if (count <= 0) {
    return result;
  }

  // 预分配实体
  MonsterEntityPool::getInstance().preallocate(registry, static_cast<size_t>(count));

  result.reserve(static_cast<size_t>(count));
  for (int i = 0; i < count; ++i) {
    // 简单平铺创建（可根据需要做随机偏移）
    float offsetX = x + i * 5.0f;
    float offsetY = y;
    auto id = createMonster(registry, monsterId, offsetX, offsetY, parentNode);
    if (id != ecs::INVALID_ENTITY) {
      result.push_back(id);
    }
  }
  return result;
}

void MonsterMasterFactory::preallocateEntities(entt::registry &registry,
                                               size_t count) {
  MonsterEntityPool::getInstance().preallocate(registry, count);
}

const MonsterConfig *MonsterMasterFactory::getConfig(
    const std::string &monsterId) const {
  // 先尝试每个工厂自己的配置
  for (const auto &entry : _factories) {
    if (!entry.factory) {
      continue;
    }
    if (const auto *cfg = entry.factory->getConfig(monsterId)) {
      return cfg;
    }
  }

  // 回退到 ConfigCache（仅返回已缓存的配置，不触发加载）
  return ConfigCache::getInstance().getCachedConfigById(monsterId);
}

std::vector<std::string> MonsterMasterFactory::getAllSupportedMonsterIds() const {
  std::vector<std::string> ids;
  for (const auto &entry : _factories) {
    if (!entry.factory) continue;
    auto supported = entry.factory->getSupportedIds();
    ids.insert(ids.end(), supported.begin(), supported.end());
  }
  // 去重
  std::sort(ids.begin(), ids.end());
  ids.erase(std::unique(ids.begin(), ids.end()), ids.end());
  return ids;
}

bool MonsterMasterFactory::isSupported(const std::string &monsterId) const {
  return supports(monsterId);
}

bool MonsterMasterFactory::loadConfigFile(const std::string &filePath) {
  return ConfigCache::getInstance().getConfig(filePath) != nullptr;
}

int MonsterMasterFactory::preloadConfigDirectory(const std::string &dirPath) {
  return ConfigCache::getInstance().preloadDirectory(dirPath);
}
