#ifndef __ECS_CONFIGCACHE_H__
#define __ECS_CONFIGCACHE_H__

#include "core/factory/MonsterConfig.h"
#include "json/document.h"
#include <string>
#include <unordered_map>
#include <unordered_set>

/**
 * @class ConfigCache
 * @brief 怪物配置缓存 - 提供按需加载和目录预加载能力
 *
 * Requirements: 5.1, 5.2, 5.3, 5.4, 5.5
 */
class ConfigCache {
public:
  static ConfigCache &getInstance() {
    static ConfigCache instance;
    return instance;
  }

  ConfigCache(const ConfigCache &) = delete;
  ConfigCache &operator=(const ConfigCache &) = delete;

  /**
   * @brief 获取配置（自动缓存，如果未加载则加载）
   * @param filePath 配置文件路径（相对于 Resources）
   * @return 配置指针，失败返回 nullptr
   */
  const MonsterConfig *getConfig(const std::string &filePath);

  /**
   * @brief 按 monsterId 获取已缓存配置（不触发加载）
   */
  const MonsterConfig *getCachedConfigById(const std::string &monsterId) const;

  /**
   * @brief 预加载目录下的所有已知配置
   * @param dirPath 目录路径（相对于 Resources）
   * @return 成功加载的数量
   */
  int preloadDirectory(const std::string &dirPath);

  /**
   * @brief 清空缓存
   */
  void clear();

  /**
   * @brief 是否已加载任意配置
   */
  bool isLoaded() const;

  /**
   * @brief 当前缓存的配置数量
   */
  size_t getCacheSize() const;

private:
  ConfigCache() = default;

  bool loadSingleConfig(const std::string &filePath);
  bool parseMonsterConfig(rapidjson::Document &doc, MonsterConfig &config);

  std::unordered_map<std::string, MonsterConfig> _cache;      // monsterId -> config
  std::unordered_map<std::string, std::string> _fileToId;     // filePath -> monsterId
  std::unordered_set<std::string> _failedFiles;               // avoid重复尝试失败文件
};

#endif // __ECS_CONFIGCACHE_H__
