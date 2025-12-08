#include "MonsterFactory.h"
#include "platform/CCFileUtils.h"
#include "ecs/SpriteComponent.h"

USING_NS_CC;

// ==================== MonsterFactory 实现 ====================

MonsterFactory::MonsterFactory() {
  registerDefaultCreators();
}

void MonsterFactory::registerDefaultCreators() {
  // 注册各色史莱姆创建器 (每种史莱姆独立创建器)
  registerCreator("GreenSlime", std::make_shared<GreenSlimeCreator>());
  registerCreator("BlueSlime", std::make_shared<BlueSlimeCreator>());
  registerCreator("RedSlime", std::make_shared<RedSlimeCreator>());
  registerCreator("YellowSlime", std::make_shared<YellowSlimeCreator>());
  registerCreator("PurpleSlime", std::make_shared<PurpleSlimeCreator>());
  registerCreator("IceSlime", std::make_shared<IceSlimeCreator>());
  registerCreator("SpikedIceSlime", std::make_shared<SpikedIceSlimeCreator>());
  registerCreator("SpikedJungleSlime", std::make_shared<SpikedJungleSlimeCreator>());
  registerCreator("UmbrellaSlime", std::make_shared<UmbrellaSlimeCreator>());
  registerCreator("MotherSlime", std::make_shared<MotherSlimeCreator>());
  registerCreator("BabySlime", std::make_shared<BabySlimeCreator>());
  
  // 未来可以在这里注册更多创建器:
  // registerCreator("BasicZombie", std::make_shared<ZombieCreator>());
  // registerCreator("VampireBat", std::make_shared<BatCreator>());
  
  CCLOG("MonsterFactory: Default creators registered");
}

void MonsterFactory::registerCreator(const std::string &type,
                                     std::shared_ptr<IMonsterCreator> creator) {
  _creators[type] = creator;
  CCLOG("MonsterFactory: Registered creator for type '%s'", type.c_str());
}

bool MonsterFactory::loadConfig(const std::string &configPath) {
  // 读取JSON文件
  std::string fullPath =
      FileUtils::getInstance()->fullPathForFilename(configPath);
  std::string content = FileUtils::getInstance()->getStringFromFile(fullPath);

  if (content.empty()) {
    CCLOG("MonsterFactory: Failed to load config file: %s", configPath.c_str());
    return false;
  }

  // 解析JSON
  rapidjson::Document doc;
  doc.Parse(content.c_str());

  if (doc.HasParseError()) {
    CCLOG("MonsterFactory: JSON parse error at offset %zu: %d",
          doc.GetErrorOffset(), doc.GetParseError());
    return false;
  }

  // 解析monsters对象
  if (!doc.HasMember("monsters") || !doc["monsters"].IsObject()) {
    CCLOG("MonsterFactory: Invalid config format - missing 'monsters' object");
    return false;
  }

  const auto &monsters = doc["monsters"];
  for (auto it = monsters.MemberBegin(); it != monsters.MemberEnd(); ++it) {
    std::string monsterId = it->name.GetString();
    MonsterConfig config;
    config.id = monsterId;

    if (parseMonsterConfig(it->value, config)) {
      _configs[monsterId] = config;
      CCLOG("MonsterFactory: Loaded config for '%s'", monsterId.c_str());
    } else {
      CCLOG("MonsterFactory: Failed to parse config for '%s'",
            monsterId.c_str());
    }
  }

  _loaded = true;
  CCLOG("MonsterFactory: Loaded %zu monster configs", _configs.size());
  return true;
}

bool MonsterFactory::loadSingleConfig(const std::string &filePath) {
  std::string fullPath =
      FileUtils::getInstance()->fullPathForFilename(filePath);
  std::string content = FileUtils::getInstance()->getStringFromFile(fullPath);

  if (content.empty()) {
    CCLOG("MonsterFactory: Failed to load file: %s", filePath.c_str());
    return false;
  }

  rapidjson::Document doc;
  doc.Parse(content.c_str());

  if (doc.HasParseError()) {
    CCLOG("MonsterFactory: JSON parse error in %s", filePath.c_str());
    return false;
  }

  // 单个怪物配置文件必须有id字段
  if (!doc.HasMember("id") || !doc["id"].IsString()) {
    CCLOG("MonsterFactory: Missing 'id' field in %s", filePath.c_str());
    return false;
  }

  MonsterConfig config;
  config.id = doc["id"].GetString();

  if (parseMonsterConfig(doc, config)) {
    _configs[config.id] = config;
    _loaded = true;
    CCLOG("MonsterFactory: Loaded single config '%s'", config.id.c_str());
    return true;
  }

  return false;
}

int MonsterFactory::loadConfigsFromDir(const std::string &dirPath) {
  int loadedCount = 0;
  
  // 获取目录下所有json文件
  std::string fullDirPath = FileUtils::getInstance()->fullPathForFilename(dirPath);
  std::vector<std::string> files;
  
  // 手动列出已知的史莱姆配置文件
  // (cocos2d-x 没有直接的目录遍历API，这里使用预定义列表)
  std::vector<std::string> knownFiles = {
    "GreenSlime.json",
    "BlueSlime.json", 
    "RedSlime.json",
    "YellowSlime.json",
    "PurpleSlime.json",
    "IceSlime.json",
    "SpikedIceSlime.json",
    "SpikedJungleSlime.json",
    "UmbrellaSlime.json",
    "MotherSlime.json",
    "BabySlime.json"
  };
  
  for (const auto &filename : knownFiles) {
    std::string filePath = dirPath + "/" + filename;
    if (loadSingleConfig(filePath)) {
      loadedCount++;
    }
  }
  
  CCLOG("MonsterFactory: Loaded %d configs from directory '%s'", 
        loadedCount, dirPath.c_str());
  return loadedCount;
}

bool MonsterFactory::parseMonsterConfig(const rapidjson::Value &json,
                                        MonsterConfig &config) {
  // 解析type
  if (json.HasMember("type") && json["type"].IsString()) {
    config.type = json["type"].GetString();
  }

  // 解析display
  if (json.HasMember("display") && json["display"].IsObject()) {
    const auto &display = json["display"];
    if (display.HasMember("spriteFolder"))
      config.display.spriteFolder = display["spriteFolder"].GetString();
    if (display.HasMember("spritePrefix"))
      config.display.spritePrefix = display["spritePrefix"].GetString();
    if (display.HasMember("frameCount"))
      config.display.frameCount = display["frameCount"].GetInt();
    if (display.HasMember("frameTime"))
      config.display.frameTime = display["frameTime"].GetFloat();
    if (display.HasMember("scale"))
      config.display.scale = display["scale"].GetFloat();
  }

  // 解析physics
  if (json.HasMember("physics") && json["physics"].IsObject()) {
    const auto &physics = json["physics"];
    if (physics.HasMember("bodyWidth"))
      config.physics.bodyWidth = physics["bodyWidth"].GetFloat();
    if (physics.HasMember("bodyHeight"))
      config.physics.bodyHeight = physics["bodyHeight"].GetFloat();
    if (physics.HasMember("bodyHeightOffset"))
      config.physics.bodyHeightOffset = physics["bodyHeightOffset"].GetFloat();
    if (physics.HasMember("mass"))
      config.physics.mass = physics["mass"].GetFloat();
    if (physics.HasMember("friction"))
      config.physics.friction = physics["friction"].GetFloat();
    if (physics.HasMember("restitution"))
      config.physics.restitution = physics["restitution"].GetFloat();
    if (physics.HasMember("collisionGroup"))
      config.physics.collisionGroup = physics["collisionGroup"].GetInt();
  }

  // 解析movement
  if (json.HasMember("movement") && json["movement"].IsObject()) {
    const auto &movement = json["movement"];
    if (movement.HasMember("type"))
      config.movement.type = movement["type"].GetString();
    if (movement.HasMember("jumpCooldown"))
      config.movement.jumpCooldown = movement["jumpCooldown"].GetFloat();
    if (movement.HasMember("horizontalImpulse"))
      config.movement.horizontalImpulse =
          movement["horizontalImpulse"].GetFloat();
    if (movement.HasMember("verticalImpulse"))
      config.movement.verticalImpulse = movement["verticalImpulse"].GetFloat();
    if (movement.HasMember("patrolImpulseRatio"))
      config.movement.patrolImpulseRatio =
          movement["patrolImpulseRatio"].GetFloat();
    if (movement.HasMember("directionChangeChance"))
      config.movement.directionChangeChance =
          movement["directionChangeChance"].GetFloat();
  }

  // 解析ai
  if (json.HasMember("ai") && json["ai"].IsObject()) {
    const auto &ai = json["ai"];
    if (ai.HasMember("aggroRange"))
      config.ai.aggroRange = ai["aggroRange"].GetFloat();
    if (ai.HasMember("deaggroRange"))
      config.ai.deaggroRange = ai["deaggroRange"].GetFloat();
    if (ai.HasMember("targetTag"))
      config.ai.targetTag = ai["targetTag"].GetString();
  }

  // 解析stats
  if (json.HasMember("stats") && json["stats"].IsObject()) {
    const auto &stats = json["stats"];
    if (stats.HasMember("maxHealth"))
      config.stats.maxHealth = stats["maxHealth"].GetFloat();
    if (stats.HasMember("attackDamage"))
      config.stats.attackDamage = stats["attackDamage"].GetFloat();
    if (stats.HasMember("attackRange"))
      config.stats.attackRange = stats["attackRange"].GetFloat();
    if (stats.HasMember("attackCooldown"))
      config.stats.attackCooldown = stats["attackCooldown"].GetFloat();
  }

  // 解析loot
  if (json.HasMember("loot") && json["loot"].IsArray()) {
    const auto &lootArray = json["loot"];
    for (rapidjson::SizeType i = 0; i < lootArray.Size(); i++) {
      const auto &item = lootArray[i];
      MonsterConfig::LootItem lootItem;
      if (item.HasMember("itemId"))
        lootItem.itemId = item["itemId"].GetString();
      if (item.HasMember("minCount"))
        lootItem.minCount = item["minCount"].GetInt();
      if (item.HasMember("maxCount"))
        lootItem.maxCount = item["maxCount"].GetInt();
      if (item.HasMember("dropChance"))
        lootItem.dropChance = item["dropChance"].GetFloat();
      config.loot.push_back(lootItem);
    }
  }

  // 解析projectile（投射物攻击配置）
  if (json.HasMember("projectile") && json["projectile"].IsObject()) {
    config.projectile.enabled = true;
    const auto &proj = json["projectile"];
    if (proj.HasMember("spritePath"))
      config.projectile.spritePath = proj["spritePath"].GetString();
    if (proj.HasMember("spriteWidth"))
      config.projectile.spriteWidth = proj["spriteWidth"].GetFloat();
    if (proj.HasMember("spriteHeight"))
      config.projectile.spriteHeight = proj["spriteHeight"].GetFloat();
    if (proj.HasMember("count"))
      config.projectile.count = proj["count"].GetInt();
    if (proj.HasMember("damage"))
      config.projectile.damage = proj["damage"].GetFloat();
    if (proj.HasMember("speed"))
      config.projectile.speed = proj["speed"].GetFloat();
    if (proj.HasMember("lifetime"))
      config.projectile.lifetime = proj["lifetime"].GetFloat();
    if (proj.HasMember("fireInterval"))
      config.projectile.fireInterval = proj["fireInterval"].GetFloat();
    if (proj.HasMember("fireRange"))
      config.projectile.fireRange = proj["fireRange"].GetFloat();
    if (proj.HasMember("horizontalSpread"))
      config.projectile.horizontalSpread = proj["horizontalSpread"].GetFloat();
    if (proj.HasMember("verticalImpulse"))
      config.projectile.verticalImpulse = proj["verticalImpulse"].GetFloat();
    if (proj.HasMember("useGravity"))
      config.projectile.useGravity = proj["useGravity"].GetBool();
  }

  // 解析debuff（减益效果配置）
  if (json.HasMember("debuff") && json["debuff"].IsObject()) {
    const auto &debuff = json["debuff"];
    // 冰系
    if (debuff.HasMember("chillChance"))
      config.debuff.chillChance = debuff["chillChance"].GetFloat();
    if (debuff.HasMember("chillDuration"))
      config.debuff.chillDuration = debuff["chillDuration"].GetFloat();
    if (debuff.HasMember("chillSpeedReduction"))
      config.debuff.chillSpeedReduction = debuff["chillSpeedReduction"].GetFloat();
    if (debuff.HasMember("freezeChance"))
      config.debuff.freezeChance = debuff["freezeChance"].GetFloat();
    if (debuff.HasMember("freezeDuration"))
      config.debuff.freezeDuration = debuff["freezeDuration"].GetFloat();
    // 毒系
    if (debuff.HasMember("poisonChance1"))
      config.debuff.poisonChance1 = debuff["poisonChance1"].GetFloat();
    if (debuff.HasMember("poisonDuration1"))
      config.debuff.poisonDuration1 = debuff["poisonDuration1"].GetFloat();
    if (debuff.HasMember("poisonDamage1"))
      config.debuff.poisonDamage1 = debuff["poisonDamage1"].GetFloat();
    if (debuff.HasMember("poisonChance2"))
      config.debuff.poisonChance2 = debuff["poisonChance2"].GetFloat();
    if (debuff.HasMember("poisonDuration2"))
      config.debuff.poisonDuration2 = debuff["poisonDuration2"].GetFloat();
    if (debuff.HasMember("poisonDamage2"))
      config.debuff.poisonDamage2 = debuff["poisonDamage2"].GetFloat();
  }

  // 解析slowFall（缓降配置）
  if (json.HasMember("slowFall") && json["slowFall"].IsObject()) {
    config.slowFall.enabled = true;
    const auto &sf = json["slowFall"];
    if (sf.HasMember("maxFallSpeed"))
      config.slowFall.maxFallSpeed = sf["maxFallSpeed"].GetFloat();
    if (sf.HasMember("fallDamping"))
      config.slowFall.fallDamping = sf["fallDamping"].GetFloat();
  }

  return true;
}

ecs::EntityId MonsterFactory::createMonster(ecs::World &world,
                                            const std::string &monsterId,
                                            float x, float y,
                                            cocos2d::Node *parentNode) {
  // 查找配置
  auto configIt = _configs.find(monsterId);
  if (configIt == _configs.end()) {
    CCLOG("MonsterFactory: Unknown monster ID: %s", monsterId.c_str());
    return ecs::INVALID_ENTITY;
  }

  const MonsterConfig &cfg = configIt->second;

  // 查找对应ID的创建器
  auto creatorIt = _creators.find(monsterId);
  if (creatorIt == _creators.end()) {
    CCLOG("MonsterFactory: No creator registered for '%s'", monsterId.c_str());
    return ecs::INVALID_ENTITY;
  }

  // 使用创建器创建怪物
  return creatorIt->second->create(world, cfg, x, y, parentNode);
}

std::vector<std::string> MonsterFactory::getMonsterIdsByType(const std::string &type) const {
  std::vector<std::string> ids;
  for (const auto &pair : _configs) {
    if (pair.second.type == type) {
      ids.push_back(pair.first);
    }
  }
  return ids;
}

// ==================== SlimeCreatorBase 实现 ====================

cocos2d::Color3B SlimeCreatorBase::getFallbackColor(const std::string &monsterId) {
  // 默认白色，子类会重写
  return Color3B::WHITE;
}

cocos2d::Sprite* SlimeCreatorBase::createSprite(const MonsterConfig &config,
                                                 float x, float y,
                                                 cocos2d::Node *parentNode) {
  Sprite *sprite = nullptr;
  std::string firstFramePath =
      config.display.spriteFolder + "/" + config.display.spritePrefix + "1.png";

  CCLOG("Creating sprite for %s, path: %s", config.id.c_str(), firstFramePath.c_str());
  
  sprite = Sprite::create(firstFramePath);
  if (!sprite) {
    // 使用备用方块
    CCLOG("  Failed to load sprite, using fallback color");
    sprite = Sprite::create();
    sprite->setTextureRect(Rect(0, 0, 40, 40));
    sprite->setColor(getFallbackColor(config.id));
  } else {
    CCLOG("  Sprite loaded successfully");
  }

  if (sprite) {
    sprite->retain();
    
    // 根据物理体宽度计算精灵缩放比例，使贴图完全贴合物理体
    float originalWidth = sprite->getContentSize().width;
    float originalHeight = sprite->getContentSize().height;
    float bodyWidth = config.physics.bodyWidth;
    float bodyHeight = config.physics.bodyHeight;
    
    // 计算缩放：让精灵宽度匹配物理体宽度
    float scale = bodyWidth / originalWidth;
    sprite->setScale(scale);
    
    // 保持默认锚点在中心(0.5, 0.5)
    sprite->setPosition(Vec2(x, y));

    // 加载动画帧
    Vector<SpriteFrame *> animFrames;
    for (int i = 1; i <= config.display.frameCount; i++) {
      std::string framePath = config.display.spriteFolder + "/" +
                              config.display.spritePrefix + std::to_string(i) +
                              ".png";
      auto texture =
          Director::getInstance()->getTextureCache()->addImage(framePath);
      if (texture) {
        auto frame = SpriteFrame::createWithTexture(
            texture, Rect(0, 0, texture->getContentSize().width,
                          texture->getContentSize().height));
        if (frame)
          animFrames.pushBack(frame);
      }
    }

    // 播放动画
    if (animFrames.size() >= 2) {
      auto animation =
          Animation::createWithSpriteFrames(animFrames, config.display.frameTime);
      sprite->runAction(RepeatForever::create(Animate::create(animation)));
    }

    // 添加到父节点
    if (parentNode) {
      parentNode->addChild(sprite, 1);
    }

    // 设置物理体 - 使用配置中的bodyWidth/bodyHeight
    PhysicsMaterial material(config.physics.mass, config.physics.restitution,
                             config.physics.friction);
    // 计算偏移量：使物理体底部与贴图底部对齐
    float scaledHeight = originalHeight * scale;
    float spriteHalfHeight = scaledHeight / 2.0f;
    float bodyHalfHeight = bodyHeight / 2.0f;
    float offsetY = -(spriteHalfHeight - bodyHalfHeight) + config.physics.bodyHeightOffset;
    Vec2 bodyOffset(0, offsetY);
    auto body = PhysicsBody::createBox(
        Size(bodyWidth, bodyHeight), material, bodyOffset);
    body->setDynamic(true);
    body->setMass(config.physics.mass);
    body->setRotationEnable(false);
    body->setCategoryBitmask(0x0002);        // 敌人类别
    body->setContactTestBitmask(0xFFFFFFFF); // 检测所有接触
    body->setCollisionBitmask(0xFFFFFFFB);   // 和所有物体碰撞，除了投射物(0x0004)
    body->setGroup(config.physics.collisionGroup);
    sprite->setPhysicsBody(body);
  }

  return sprite;
}

void SlimeCreatorBase::addBaseComponents(ecs::World &world, ecs::EntityId entity,
                                          const MonsterConfig &config,
                                          cocos2d::Sprite *sprite) {
  // 变换组件
  world.addComponent<ecs::TransformComponent>(entity,
      sprite->getPositionX(), sprite->getPositionY());

  // 精灵组件
  if (sprite) {
    auto &slimeSprite = world.addComponent<ecs::SlimeSpriteComponent>(entity);
    slimeSprite.sprite = sprite;
    slimeSprite.baseScale = sprite->getScale(); // 使用实际计算的缩放值
    slimeSprite.slimeType = config.id;
    slimeSprite.animationLoaded = true;

    ecs::NodeEntityMap::getInstance().registerNode(sprite, entity);
  }

  // 仇恨组件
  auto &aggro = world.addComponent<ecs::AggroComponent>(
      entity, config.ai.aggroRange, config.ai.deaggroRange);
  aggro.targetTag = config.ai.targetTag;

  // 地面检测组件
  auto &ground = world.addComponent<ecs::GroundDetectorComponent>(entity);
  ground.isOnGround = true;

  // 跳跃移动组件
  auto &jump = world.addComponent<ecs::JumpMovementComponent>(
      entity, config.movement.jumpCooldown, config.movement.horizontalImpulse,
      config.movement.verticalImpulse);
  jump.patrolImpulseRatio = config.movement.patrolImpulseRatio;
  jump.randomDirectionChangeChance = config.movement.directionChangeChance;

  // 生命值组件
  world.addComponent<ecs::HealthComponent>(entity, config.stats.maxHealth);

  // 战斗组件
  world.addComponent<ecs::CombatComponent>(
      entity, config.stats.attackRange, config.stats.attackDamage,
      config.stats.attackCooldown);

  // 敌人标记
  world.addComponent<ecs::EnemyTag>(entity, config.type);

  // 掉落物组件
  auto &loot = world.addComponent<ecs::LootComponent>(entity);
  for (const auto &item : config.loot) {
    loot.addDrop(item.itemId, item.minCount, item.maxCount, item.dropChance);
  }
}

ecs::EntityId SlimeCreatorBase::create(ecs::World &world, const MonsterConfig &config,
                                        float x, float y, cocos2d::Node *parentNode) {
  ecs::EntityId entity = world.createEntity();

  // 创建精灵
  Sprite *sprite = createSprite(config, x, y, parentNode);

  // 添加基础组件
  addBaseComponents(world, entity, config, sprite);

  // 添加特有组件 (子类重写)
  addSpecialComponents(world, entity, config);

  return entity;
}

// ==================== 冰雪史莱姆特有组件 ====================

/**
 * 冰雪史莱姆 - 冷冻效果
 * - 描述: 你的移动速度已降低
 * - 几率: 8.3% (如果未被冰冻)
 * - 持续时间: 10秒
 */
void IceSlimeCreator::addSpecialComponents(ecs::World &world, ecs::EntityId entity,
                                            const MonsterConfig &config) {
  // TODO: 添加冷冻效果组件 (等待玩家AI设计完成后实现)
  // world.addComponent<ChillEffectComponent>(entity, 0.083f, 10.0f);
  // 
  // ChillEffectComponent 预期字段:
  // - chillChance: 0.083f (8.3%)
  // - chillDuration: 10.0f (10秒)
  // - speedReduction: 0.5f (移动速度降低50%)
  // - canStack: false (不可叠加，未被冰冻时才触发)
}

/**
 * 史莱姆母体 - 死亡时生成1-3只史莱姆宝宝
 */
void MotherSlimeCreator::addSpecialComponents(ecs::World &world, ecs::EntityId entity,
                                               const MonsterConfig &config) {
  // 添加死亡生成组件
  world.addComponent<ecs::DeathSpawnComponent>(entity, "BabySlime", 1, 3);
  CCLOG("MotherSlime: Added DeathSpawnComponent (spawns 1-3 BabySlime on death)");
}

/**
 * 冰雪尖刺史莱姆 - 发射冰雪尖刺攻击
 * 
 * 特性:
 * - 当目标进入发射范围时，向周围发射4个冰雪尖刺
 * - 尖刺以抛物线轨迹飞行（受重力影响）
 * - 发射期间不会跳跃
 * - 必然造成冷冻减益，有几率造成冰冻减益
 */
void SpikedIceSlimeCreator::addSpecialComponents(ecs::World &world, ecs::EntityId entity,
                                                  const MonsterConfig &config) {
  // 添加投射物攻击组件
  auto &projAttack = world.addComponent<ecs::ProjectileAttackComponent>(entity);
  
  // 从配置加载投射物参数
  if (config.projectile.enabled) {
    projAttack.projectileSpritePath = config.projectile.spritePath;
    projAttack.projectileSpriteWidth = config.projectile.spriteWidth;
    projAttack.projectileSpriteHeight = config.projectile.spriteHeight;
    projAttack.projectileCount = config.projectile.count;
    projAttack.projectileDamage = config.projectile.damage;
    projAttack.projectileSpeed = config.projectile.speed;
    projAttack.projectileLifetime = config.projectile.lifetime;
    projAttack.fireInterval = config.projectile.fireInterval;
    projAttack.fireRange = config.projectile.fireRange;
    projAttack.horizontalSpread = config.projectile.horizontalSpread;
    projAttack.verticalImpulse = config.projectile.verticalImpulse;
    projAttack.useGravity = config.projectile.useGravity;
  }
  
  // 加载减益效果参数
  projAttack.chillChance = config.debuff.chillChance;
  projAttack.chillDuration = config.debuff.chillDuration;
  projAttack.chillSpeedReduction = config.debuff.chillSpeedReduction;
  projAttack.freezeChance = config.debuff.freezeChance;
  projAttack.freezeDuration = config.debuff.freezeDuration;
  
  CCLOG("SpikedIceSlime: Added ProjectileAttackComponent");
  CCLOG("  - Fires %d spikes every %.1fs when target within %.1f range",
        projAttack.projectileCount, projAttack.fireInterval, projAttack.fireRange);
  CCLOG("  - Chill: %.0f%% chance, %.1fs duration, %.0f%% slow",
        projAttack.chillChance * 100, projAttack.chillDuration, 
        projAttack.chillSpeedReduction * 100);
  CCLOG("  - Freeze: %.0f%% chance, %.1fs duration",
        projAttack.freezeChance * 100, projAttack.freezeDuration);
}

/**
 * 丛林尖刺史莱姆特化组件添加
 * 
 * 特殊行为:
 * - 当目标进入发射范围时，向周围发射4个丛林尖刺
 * - 尖刺以抛物线轨迹飞行（受重力影响）
 * - 发射期间不会跳跃
 * - 37.5%几率造成长时间中毒，25%几率造成短时间中毒
 */
void SpikedJungleSlimeCreator::addSpecialComponents(ecs::World &world, ecs::EntityId entity,
                                                     const MonsterConfig &config) {
  // 添加投射物攻击组件
  auto &projAttack = world.addComponent<ecs::ProjectileAttackComponent>(entity);
  
  // 从配置加载投射物参数
  if (config.projectile.enabled) {
    projAttack.projectileSpritePath = config.projectile.spritePath;
    projAttack.projectileSpriteWidth = config.projectile.spriteWidth;
    projAttack.projectileSpriteHeight = config.projectile.spriteHeight;
    projAttack.projectileCount = config.projectile.count;
    projAttack.projectileDamage = config.projectile.damage;
    projAttack.projectileSpeed = config.projectile.speed;
    projAttack.projectileLifetime = config.projectile.lifetime;
    projAttack.fireInterval = config.projectile.fireInterval;
    projAttack.fireRange = config.projectile.fireRange;
    projAttack.horizontalSpread = config.projectile.horizontalSpread;
    projAttack.verticalImpulse = config.projectile.verticalImpulse;
    projAttack.useGravity = config.projectile.useGravity;
  }
  
  // 加载毒素减益效果参数
  projAttack.poisonChance1 = config.debuff.poisonChance1;
  projAttack.poisonDuration1 = config.debuff.poisonDuration1;
  projAttack.poisonDamage1 = config.debuff.poisonDamage1;
  projAttack.poisonChance2 = config.debuff.poisonChance2;
  projAttack.poisonDuration2 = config.debuff.poisonDuration2;
  projAttack.poisonDamage2 = config.debuff.poisonDamage2;
  
  CCLOG("SpikedJungleSlime: Added ProjectileAttackComponent");
  CCLOG("  - Fires %d spikes every %.1fs when target within %.1f range",
        projAttack.projectileCount, projAttack.fireInterval, projAttack.fireRange);
  CCLOG("  - Poison1: %.1f%% chance, %.1fs duration, %.1f dps",
        projAttack.poisonChance1 * 100, projAttack.poisonDuration1, projAttack.poisonDamage1);
  CCLOG("  - Poison2: %.1f%% chance, %.1fs duration, %.1f dps",
        projAttack.poisonChance2 * 100, projAttack.poisonDuration2, projAttack.poisonDamage2);
}

/**
 * 伞史莱姆特化组件添加
 * 
 * 特殊行为:
 * - 下落时有空气阻力，像撑着伞一样缓缓飘落
 * - 最大下落速度受限
 */
void UmbrellaSlimeCreator::addSpecialComponents(ecs::World &world, ecs::EntityId entity,
                                                 const MonsterConfig &config) {
  // 添加缓降组件
  auto &slowFall = world.addComponent<ecs::SlowFallComponent>(entity);
  
  if (config.slowFall.enabled) {
    slowFall.maxFallSpeed = config.slowFall.maxFallSpeed;
    slowFall.fallDamping = config.slowFall.fallDamping;
  }
  slowFall.isActive = true;
  
  CCLOG("UmbrellaSlime: Added SlowFallComponent");
  CCLOG("  - Max fall speed: %.1f", slowFall.maxFallSpeed);
  CCLOG("  - Fall damping: %.2f", slowFall.fallDamping);
}

const MonsterConfig *
MonsterFactory::getConfig(const std::string &monsterId) const {
  auto it = _configs.find(monsterId);
  return it != _configs.end() ? &it->second : nullptr;
}

std::vector<std::string> MonsterFactory::getAllMonsterIds() const {
  std::vector<std::string> ids;
  for (const auto &pair : _configs) {
    ids.push_back(pair.first);
  }
  return ids;
}
