#ifndef __MONSTER_FACTORY_H__
#define __MONSTER_FACTORY_H__

#include "cocos2d.h"
#include "ecs/AllComponents.h"
#include <entt/entt.hpp>
#include "json/document.h"
#include <string>
#include <unordered_map>
#include <vector>

/**
 * @class MonsterConfig
 * @brief 怪物配置数据结构 - 通用配置，适用于所有怪物类型
 */
struct MonsterConfig {
  std::string id;   // 怪物ID (如 "GreenSlime")
  std::string type; // 怪物类型 (如 "Slime", "Zombie", "Bat")

  // 显示属性
  struct Display {
    std::string spriteFolder;
    std::string spritePrefix;
    int frameCount = 2;
    std::vector<int> frameSequence; // 自定义帧序列，如 {1,2,3,2}，为空则按顺序播放
    float frameTime = 0.15f;
    float scale = 1.0f;
    float anchorY = 0.5f; // Y轴锚点（0=底部，0.5=中心，1=顶部）
    int zOrder = 1; // 渲染层级
    cocos2d::Color3B fallbackColor = cocos2d::Color3B::WHITE; // 备用颜色
  } display;

  // 物理属性
  struct Physics {
    float bodyWidth = 40.0f;   // 物理体宽度
    float bodyHeight = 25.0f;  // 物理体高度（史莱姆是扁的）
    float bodyHeightOffset = 0.0f; // 物理体高度补偿（用于调整精灵与地面的间隙）
    float mass = 1.0f;
    float friction = 1.0f;
    float restitution = 0.0f;
    int collisionGroup = -1;
    bool useGravity = true;  // 是否受重力影响
  } physics;

  // 移动属性
  struct Movement {
    std::string type = "jump"; // "jump", "walk", "fly", "hover"
    float speed = 100.0f;      // 移动速度 (walk/fly类型)
    float jumpCooldown = 2.0f;
    float horizontalImpulse = 300.0f;
    float verticalImpulse = 500.0f;
    float patrolImpulseRatio = 0.5f;
    float directionChangeChance = 0.3f;
    
    // 行走类型专用参数
    float walkSpeed = 80.0f;           // 行走速度
    float jumpForce = 450.0f;          // 跳跃力度
    bool obstacleJumpEnabled = true;   // 是否启用障碍物跳跃
    bool targetJumpEnabled = true;     // 是否启用目标跳跃反应
    float targetJumpReactionTime = 0.1f; // 目标跳跃反应时间
    float patrolDirectionChangeInterval = 3.0f; // 巡逻改变方向间隔
    
    // 飞行类型专用参数（DemonEye等）
    float flySpeed = 120.0f;           // 飞行速度
    float maxSpeed = 200.0f;           // 最大速度
    float acceleration = 80.0f;        // 加速度
    float turnRate = 1.5f;             // 转向速率（弧度/秒）
    float wobbleAmplitude = 0.3f;      // 摆动幅度
    float wobbleFrequency = 2.0f;      // 摆动频率
    
    // 蚁狮类型专用参数
    float detectionRange = 300.0f;     // 检测范围
    float shootInterval = 5.0f;        // 射击间隔
    float projectileSpeed = 400.0f;    // 射弹速度
    float rotationSpeed = 2.0f;        // 头部转向速度
  } movement;

  // AI属性
  struct AI {
    std::string behavior = "aggressive"; // "aggressive", "passive", "neutral"
    float aggroRange = 300.0f;
    float deaggroRange = 400.0f;
    std::string targetTag = "Player";
  } ai;

  // 战斗属性
  struct Stats {
    float maxHealth = 50.0f;
    float attackDamage = 10.0f;
    float attackRange = 50.0f;
    float attackCooldown = 1.0f;
    float defense = 0.0f;      // 防御力
  } stats;

  // 掉落物
  struct LootItem {
    std::string itemId;
    int minCount;
    int maxCount;
    float dropChance;
  };
  std::vector<LootItem> loot;
  
  // 投射物攻击配置（可选，用于远程攻击怪物）
  struct Projectile {
    bool enabled = false;             // 是否启用投射物攻击
    std::string spritePath;           // 投射物贴图路径
    float spriteWidth = 10.0f;        // 投射物宽度
    float spriteHeight = 20.0f;       // 投射物高度
    int count = 4;                    // 每次发射数量
    float damage = 8.0f;              // 投射物伤害
    float speed = 300.0f;             // 投射物速度
    float lifetime = 3.0f;            // 投射物生命周期
    float fireInterval = 0.8f;        // 发射间隔
    float fireRange = 200.0f;         // 发射范围
    float horizontalSpread = 80.0f;   // 水平分散角度
    float verticalImpulse = 350.0f;   // 垂直冲量
    bool useGravity = true;           // 是否受重力影响
  } projectile;
  
  // 减益效果配置（可选）
  struct Debuff {
    // 冰系
    float chillChance = 0.0f;         // 冷冻几率
    float chillDuration = 0.0f;       // 冷冻持续时间
    float chillSpeedReduction = 0.0f; // 冷冻减速比例
    float freezeChance = 0.0f;        // 冰冻几率
    float freezeDuration = 0.0f;      // 冰冻持续时间
    // 毒系
    float poisonChance1 = 0.0f;       // 毒素几率1（长时间）
    float poisonDuration1 = 0.0f;     // 毒素持续时间1
    float poisonDamage1 = 0.0f;       // 毒素每秒伤害1
    float poisonChance2 = 0.0f;       // 毒素几率2（短时间）
    float poisonDuration2 = 0.0f;     // 毒素持续时间2
    float poisonDamage2 = 0.0f;       // 毒素每秒伤害2
  } debuff;
  
  // 缓降配置（可选，用于伞史莱姆等）
  struct SlowFall {
    bool enabled = false;             // 是否启用缓降
    float maxFallSpeed = 100.0f;      // 最大下落速度
    float fallDamping = 0.85f;        // 下落阻尼系数
    float horizontalDamping = 0.95f;  // 水平阻尼系数
  } slowFall;
  
  // 史莱姆王特定配置
  struct KingSlimeConfig {
    float baseScale = 2.0f;           // 基础缩放
    float minScale = 0.8f;            // 最小缩放
    int smallJumpsPerCycle = 3;       // 每周期小跳次数
    float bigJumpMultiplier = 2.0f;   // 大跳倍率
    float teleportInterval = 15.0f;   // 传送间隔
    float teleportRange = 300.0f;     // 传送范围
    int totalSlimesToSpawn = 80;      // 总生成史莱姆数
    float spawnHealthInterval = 0.012f; // 生成间隔（血量百分比）
    int maxSpawnPerInterval = 2;      // 每次最多生成数量
  } kingSlime;
};

// ==================== 怪物工厂 ====================

/**
 * @class MonsterFactory
 * @brief 怪物工厂 - 根据JSON配置创建ECS实体（使用EnTT框架）
 *
 * 支持多种怪物类型:
 * - Slime: 史莱姆类（跳跃移动）
 * - Zombie: 僵尸类（行走移动）
 * - Bat: 蝙蝠类（飞行移动，待实现）
 *
 * 使用示例:
 * @code
 * auto& factory = MonsterFactory::getInstance();
 * factory.loadConfigsFromDir("config/slimes");
 *
 * // 创建史莱姆
 * entt::registry registry;
 * EntityId slime = factory.createMonster(registry, "GreenSlime", x, y, parent);
 * @endcode
 */
class MonsterFactory {
public:
  static MonsterFactory &getInstance() {
    static MonsterFactory instance;
    return instance;
  }

  /**
   * @brief 加载怪物配置文件（单个文件，包含多个怪物）
   * @param configPath 配置文件路径 (相对于Resources)
   * @return 是否加载成功
   */
  bool loadConfig(const std::string &configPath);

  /**
   * @brief 从目录加载所有怪物配置（每个怪物一个文件）
   * @param dirPath 目录路径 (相对于Resources)
   * @return 加载成功的数量
   */
  int loadConfigsFromDir(const std::string &dirPath);

  /**
   * @brief 加载单个怪物配置文件
   * @param filePath 文件路径 (相对于Resources)
   * @return 是否加载成功
   */
  bool loadSingleConfig(const std::string &filePath);

  /**
   * @brief 根据配置创建怪物实体
   * @param registry EnTT注册表
   * @param monsterId 怪物ID (如 "GreenSlime")
   * @param x X坐标
   * @param y Y坐标
   * @param parentNode 父节点
   * @return 实体ID
   */
  ecs::EntityId createMonster(entt::registry &registry, const std::string &monsterId,
                              float x, float y,
                              cocos2d::Node *parentNode = nullptr);

  /**
   * @brief 获取怪物配置
   * @param monsterId 怪物ID
   * @return 配置指针，不存在返回nullptr
   */
  const MonsterConfig *getConfig(const std::string &monsterId) const;

  /**
   * @brief 获取所有已加载的怪物ID
   */
  std::vector<std::string> getAllMonsterIds() const;

  /**
   * @brief 获取指定类型的所有怪物ID
   */
  std::vector<std::string> getMonsterIdsByType(const std::string &type) const;

  /**
   * @brief 是否已加载配置
   */
  bool isLoaded() const { return _loaded; }

  /**
   * @brief 清空所有已加载的配置（用于场景切换）
   */
  void clearConfigs() {
    _configs.clear();
    _loaded = false;
  }

private:
  MonsterFactory() = default;
  MonsterFactory(const MonsterFactory &) = delete;
  MonsterFactory &operator=(const MonsterFactory &) = delete;

  bool parseMonsterConfig(const rapidjson::Value &json, MonsterConfig &config);

  std::unordered_map<std::string, MonsterConfig> _configs;
  bool _loaded = false;
};

#endif // __MONSTER_FACTORY_H__
