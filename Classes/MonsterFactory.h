#ifndef __MONSTER_FACTORY_H__
#define __MONSTER_FACTORY_H__

#include "cocos2d.h"
#include "ecs/ECS.h"
#include "json/document.h"
#include <functional>
#include <string>
#include <unordered_map>

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
    float frameTime = 0.15f;
    float scale = 1.0f;
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
  } slowFall;
};

// ==================== 怪物类型创建器接口 ====================

/**
 * @brief 怪物类型创建器 - 用于不同类型怪物的特化创建逻辑
 */
class IMonsterCreator {
public:
  virtual ~IMonsterCreator() = default;
  
  /**
   * @brief 创建怪物实体
   * @param world ECS世界
   * @param config 怪物配置
   * @param x X坐标
   * @param y Y坐标
   * @param parentNode 父节点
   * @return 实体ID
   */
  virtual ecs::EntityId create(ecs::World &world, const MonsterConfig &config,
                               float x, float y,
                               cocos2d::Node *parentNode) = 0;
  
  /**
   * @brief 获取备用颜色（精灵加载失败时使用）
   */
  virtual cocos2d::Color3B getFallbackColor(const std::string &monsterId) = 0;
};

// ==================== 史莱姆基类创建器 ====================

/**
 * @brief 史莱姆基类创建器 - 提供通用的史莱姆创建逻辑
 * 
 * 子类可以重写 addSpecialComponents() 添加特有组件
 */
class SlimeCreatorBase : public IMonsterCreator {
public:
  ecs::EntityId create(ecs::World &world, const MonsterConfig &config,
                       float x, float y, cocos2d::Node *parentNode) override;
  cocos2d::Color3B getFallbackColor(const std::string &monsterId) override;

protected:
  /**
   * @brief 创建精灵和物理体（通用）
   */
  cocos2d::Sprite* createSprite(const MonsterConfig &config, float x, float y,
                                 cocos2d::Node *parentNode);
  
  /**
   * @brief 添加基础组件（通用）
   */
  void addBaseComponents(ecs::World &world, ecs::EntityId entity,
                         const MonsterConfig &config, cocos2d::Sprite *sprite);
  
  /**
   * @brief 添加特有组件（子类重写）
   */
  virtual void addSpecialComponents(ecs::World &world, ecs::EntityId entity,
                                    const MonsterConfig &config) {}
};

// ==================== 各色史莱姆创建器 ====================

/**
 * @brief 绿色史莱姆 - 基础史莱姆，无特殊能力
 */
class GreenSlimeCreator : public SlimeCreatorBase {
public:
  cocos2d::Color3B getFallbackColor(const std::string &monsterId) override {
    return cocos2d::Color3B::GREEN;
  }
};

/**
 * @brief 蓝色史莱姆
 */
class BlueSlimeCreator : public SlimeCreatorBase {
public:
  cocos2d::Color3B getFallbackColor(const std::string &monsterId) override {
    return cocos2d::Color3B::BLUE;
  }
};

/**
 * @brief 红色史莱姆
 */
class RedSlimeCreator : public SlimeCreatorBase {
public:
  cocos2d::Color3B getFallbackColor(const std::string &monsterId) override {
    return cocos2d::Color3B::RED;
  }
};

/**
 * @brief 黄色史莱姆
 */
class YellowSlimeCreator : public SlimeCreatorBase {
public:
  cocos2d::Color3B getFallbackColor(const std::string &monsterId) override {
    return cocos2d::Color3B::YELLOW;
  }
};

/**
 * @brief 紫色史莱姆
 */
class PurpleSlimeCreator : public SlimeCreatorBase {
public:
  cocos2d::Color3B getFallbackColor(const std::string &monsterId) override {
    return cocos2d::Color3B(128, 0, 128);
  }
};

/**
 * @brief 冰雪史莱姆 - 攻击有概率赋予“冷冻”减益
 * 
 * 冷冻效果:
 * - 描述: 你的移动速度已降低
 * - 几率: 8.3% (如果未被冰冻)
 * - 持续时间: 10秒
 */
class IceSlimeCreator : public SlimeCreatorBase {
public:
  cocos2d::Color3B getFallbackColor(const std::string &monsterId) override {
    return cocos2d::Color3B(135, 206, 250); // 淡蓝色 (LightSkyBlue)
  }
protected:
  void addSpecialComponents(ecs::World &world, ecs::EntityId entity,
                            const MonsterConfig &config) override;
};

/**
 * @brief 史莱姆母体 - 死亡时生成1-3只史莱姆宝宝
 */
class MotherSlimeCreator : public SlimeCreatorBase {
public:
  cocos2d::Color3B getFallbackColor(const std::string &monsterId) override {
    return cocos2d::Color3B(255, 182, 193); // 粉红色 (LightPink)
  }
protected:
  void addSpecialComponents(ecs::World &world, ecs::EntityId entity,
                            const MonsterConfig &config) override;
};

/**
 * @brief 史莱姆宝宝 - 由史莱姆母体死亡时生成
 */
class BabySlimeCreator : public SlimeCreatorBase {
public:
  cocos2d::Color3B getFallbackColor(const std::string &monsterId) override {
    return cocos2d::Color3B(144, 238, 144); // 浅绿色 (LightGreen)
  }
};

/**
 * @brief 冰雪尖刺史莱姆 - 发射冰雪尖刺，造成冷冻和冰冻减益
 * 
 * 特性:
 * - 当目标进入发射范围时，向周围发射4个冰雪尖刺
 * - 尖刺以抛物线轨迹飞行（受重力影响）
 * - 发射期间不会跳跃
 * - 必然造成冷冻减益，有几率造成冰冻减益
 */
class SpikedIceSlimeCreator : public SlimeCreatorBase {
public:
  cocos2d::Color3B getFallbackColor(const std::string &monsterId) override {
    return cocos2d::Color3B(100, 149, 237); // 矢车菊蓝 (CornflowerBlue)
  }
protected:
  void addSpecialComponents(ecs::World &world, ecs::EntityId entity,
                            const MonsterConfig &config) override;
};

/**
 * @brief 丛林尖刺史莱姆 - 发射丛林尖刺，造成中毒减益
 * 
 * 特性:
 * - 当目标进入发射范围时，向周围发射4个丛林尖刺
 * - 尖刺以抛物线轨迹飞行（受重力影响）
 * - 发射期间不会跳跃
 * - 37.5%几率造成长时间中毒，25%几率造成短时间中毒
 */
class SpikedJungleSlimeCreator : public SlimeCreatorBase {
public:
  cocos2d::Color3B getFallbackColor(const std::string &monsterId) override {
    return cocos2d::Color3B(34, 139, 34); // 森林绿 (ForestGreen)
  }
protected:
  void addSpecialComponents(ecs::World &world, ecs::EntityId entity,
                            const MonsterConfig &config) override;
};

/**
 * @brief 伞史莱姆 - 下落时有空气阻力，像撑着伞一样缓缓飘落
 * 
 * 特性:
 * - 跳跃后下落时速度受限
 * - 最大下落速度远低于普通史莱姆
 * - 像撑着伞飘落一样优雅
 */
class UmbrellaSlimeCreator : public SlimeCreatorBase {
public:
  cocos2d::Color3B getFallbackColor(const std::string &monsterId) override {
    return cocos2d::Color3B(255, 182, 193); // 浅粉色 (LightPink)
  }
protected:
  void addSpecialComponents(ecs::World &world, ecs::EntityId entity,
                            const MonsterConfig &config) override;
};

// ==================== 怪物工厂 ====================

/**
 * @class MonsterFactory
 * @brief 怪物工厂 - 根据JSON配置创建ECS实体
 *
 * 支持多种怪物类型，每种怪物有独立的创建器：
 * 
 * 史莱姆类:
 * - GreenSlime: 基础史莱姆
 * - BlueSlime: 蓝色史莱姆
 * - RedSlime: 红色史莱姆
 * - YellowSlime: 黄色史莱姆
 * - PurpleSlime: 紫色史莱姆
 * - IceSlime: 冰雪史莱姆 (攻击有概率赋予冷冻减益)
 * 
 * 其他类型 (待实现):
 * - Zombie: 行走式移动的僵尸
 * - Bat: 飞行式移动的蝙蝠
 * - Boss: Boss类怪物
 *
 * 使用示例:
 * @code
 * auto& factory = MonsterFactory::getInstance();
 * factory.loadConfig("config/monsters.json");
 *
 * // 创建史莱姆
 * EntityId slime = factory.createMonster(world, "GreenSlime", x, y, parent);
 *
 * // 创建僵尸 (需要先在JSON中配置)
 * EntityId zombie = factory.createMonster(world, "BasicZombie", x, y, parent);
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
   * @brief 根据配置创建怪物实体（自动选择创建器）
   * @param world ECS世界
   * @param monsterId 怪物ID (如 "GreenSlime")
   * @param x X坐标
   * @param y Y坐标
   * @param parentNode 父节点
   * @return 实体ID
   */
  ecs::EntityId createMonster(ecs::World &world, const std::string &monsterId,
                              float x, float y,
                              cocos2d::Node *parentNode = nullptr);

  /**
   * @brief 注册怪物类型创建器
   * @param type 怪物类型名称 (如 "Slime", "Zombie")
   * @param creator 创建器实例
   */
  void registerCreator(const std::string &type,
                       std::shared_ptr<IMonsterCreator> creator);

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

private:
  MonsterFactory();
  MonsterFactory(const MonsterFactory &) = delete;
  MonsterFactory &operator=(const MonsterFactory &) = delete;

  void registerDefaultCreators();
  bool parseMonsterConfig(const rapidjson::Value &json, MonsterConfig &config);

  std::unordered_map<std::string, MonsterConfig> _configs;
  std::unordered_map<std::string, std::shared_ptr<IMonsterCreator>> _creators;
  bool _loaded = false;
};

#endif // __MONSTER_FACTORY_H__
