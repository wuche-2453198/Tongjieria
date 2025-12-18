#include "MonsterFactory.h"
#include "platform/CCFileUtils.h"

USING_NS_CC;

// ==================== MonsterFactory 实现 ====================

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
  
  std::vector<std::string> knownFiles;
  
  // 根据目录名选择已知文件列表
  if (dirPath.find("slimes") != std::string::npos) {
    knownFiles = {
      "GreenSlime.json",
      "BlueSlime.json", 
      "RedSlime.json",
      "YellowSlime.json",
      "PurpleSlime.json",
      "PinkSlime.json",
      "IceSlime.json",
      "SpikedIceSlime.json",
      "SpikedJungleSlime.json",
      "UmbrellaSlime.json",
      "MotherSlime.json",
      "BabySlime.json"
    };
  } else if (dirPath.find("zombies") != std::string::npos) {
    knownFiles = {
      "Zombie.json",
      "31px-Zombie.json",
      "Bigger-Zombie.json",
      "BaldZombie.json",
      "29px-BaldZombie.json",
      "Bigger-BaldZombie.json",
      "PincushionZombie.json",
      "32px-PincushionZombie.json",
      "Bigger-PincushionZombie.json"
    };
  } else if (dirPath.find("eyes") != std::string::npos) {
    knownFiles = {
      "DemonEye.json",
      "Bigger-DemonEye.json",
      "PurpleEye.json",
      "GreenEye.json",
      "CataractEye.json",
      "DilatedEye.json"
    };
  } else {
    CCLOG("MonsterFactory: Unknown directory type '%s'", dirPath.c_str());
  }
  
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
    // 解析帧序列
    if (display.HasMember("frameSequence") && display["frameSequence"].IsArray()) {
      const auto &seq = display["frameSequence"];
      for (rapidjson::SizeType i = 0; i < seq.Size(); i++) {
        config.display.frameSequence.push_back(seq[i].GetInt());
      }
    }
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
    // 行走类型专用参数
    if (movement.HasMember("walkSpeed"))
      config.movement.walkSpeed = movement["walkSpeed"].GetFloat();
    if (movement.HasMember("jumpForce"))
      config.movement.jumpForce = movement["jumpForce"].GetFloat();
    if (movement.HasMember("obstacleJumpEnabled"))
      config.movement.obstacleJumpEnabled = movement["obstacleJumpEnabled"].GetBool();
    if (movement.HasMember("targetJumpEnabled"))
      config.movement.targetJumpEnabled = movement["targetJumpEnabled"].GetBool();
    if (movement.HasMember("targetJumpReactionTime"))
      config.movement.targetJumpReactionTime = movement["targetJumpReactionTime"].GetFloat();
    if (movement.HasMember("patrolDirectionChangeInterval"))
      config.movement.patrolDirectionChangeInterval = movement["patrolDirectionChangeInterval"].GetFloat();
    // 飞行类型专用参数
    if (movement.HasMember("flySpeed"))
      config.movement.flySpeed = movement["flySpeed"].GetFloat();
    if (movement.HasMember("maxSpeed"))
      config.movement.maxSpeed = movement["maxSpeed"].GetFloat();
    if (movement.HasMember("acceleration"))
      config.movement.acceleration = movement["acceleration"].GetFloat();
    if (movement.HasMember("turnRate"))
      config.movement.turnRate = movement["turnRate"].GetFloat();
    if (movement.HasMember("wobbleAmplitude"))
      config.movement.wobbleAmplitude = movement["wobbleAmplitude"].GetFloat();
    if (movement.HasMember("wobbleFrequency"))
      config.movement.wobbleFrequency = movement["wobbleFrequency"].GetFloat();
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
    if (sf.HasMember("horizontalDamping"))
      config.slowFall.horizontalDamping = sf["horizontalDamping"].GetFloat();
  }

  return true;
}

// ==================== 怪物创建 ====================

ecs::EntityId MonsterFactory::createMonster(entt::registry &registry,
                                               const std::string &monsterId,
                                               float x, float y,
                                               cocos2d::Node *parentNode) {
  using namespace cocos2d;
  
  // 查找配置
  auto configIt = _configs.find(monsterId);
  if (configIt == _configs.end()) {
    CCLOG("MonsterFactory(EnTT): Unknown monster ID: %s", monsterId.c_str());
    return ecs::INVALID_ENTITY;
  }

  const MonsterConfig &cfg = configIt->second;
  
  // 创建EnTT实体
  auto entity = registry.create();
  
  // ==================== 通用：创建精灵 ====================
  Sprite *sprite = nullptr;
  std::string firstFramePath = cfg.display.spriteFolder + "/" + cfg.display.spritePrefix + "1.png";
  sprite = Sprite::create(firstFramePath);
  
  if (!sprite) {
    CCLOG("Failed to load sprite from %s, using fallback", firstFramePath.c_str());
    sprite = Sprite::create();
    sprite->setTextureRect(Rect(0, 0, 40, 25));
    sprite->setColor(cfg.display.fallbackColor);
  }
  
  if (sprite && parentNode) {
    sprite->retain();
    sprite->setScale(cfg.display.scale);
    sprite->setPosition(Vec2(x, y));
    parentNode->addChild(sprite, 1);
    
    // ==================== 通用：设置物理体 ====================
    PhysicsBody* body = nullptr;
    
    if (cfg.type == "DemonEye") {
      // 恶魔眼：圆形物理体，无重力，有弹性 - 提供更自然的碰撞回弹
      // 物理体大小需要考虑精灵的缩放因子
      PhysicsMaterial material(cfg.physics.mass, cfg.physics.restitution, cfg.physics.friction);
      float scaledWidth = cfg.physics.bodyWidth * cfg.display.scale;
      float scaledHeight = cfg.physics.bodyHeight * cfg.display.scale;
      float scaledHeightOffset = cfg.physics.bodyHeightOffset * cfg.display.scale;
      // 使用圆形物理体，半径为贴图短边长度的一半（确保圆形物理体不超出贴图边界）
      float radius = std::min(scaledWidth, scaledHeight) / 2.0f;
      body = PhysicsBody::createCircle(radius, material, Vec2(scaledHeightOffset, 0));
      body->setDynamic(true);
      body->setMass(cfg.physics.mass);
      body->setRotationEnable(false);
      body->setGravityEnable(false);  // 关键：无重力
      body->setVelocityLimit(500.0f);  // 提高速度上限
      body->setLinearDamping(0.05f);   // 降低阻尼，保持惯性
      body->setAngularDamping(0.3f);   // 允许适度旋转，提高转弯灵活性
    } else {
      // 史莱姆需要更高的摩擦力来防止滑行，完全无弹力
      float friction = (cfg.type == "Slime") ? 0.9f : cfg.physics.friction;
      float restitution = (cfg.type == "Slime") ? 0.0f : cfg.physics.restitution;
      
      // PhysicsMaterial参数顺序: (density, restitution, friction)
      PhysicsMaterial material(cfg.physics.mass, restitution, friction);
      // 物理体大小需要考虑精灵的缩放因子
      float scaledWidth = cfg.physics.bodyWidth * cfg.display.scale;
      float scaledHeight = cfg.physics.bodyHeight * cfg.display.scale;
      float scaledHeightOffset = cfg.physics.bodyHeightOffset * cfg.display.scale;
      body = PhysicsBody::createBox(Size(scaledWidth, scaledHeight), material, Vec2(0, scaledHeightOffset));
      body->setDynamic(true);
      body->setMass(cfg.physics.mass);
      body->setRotationEnable(false);
      body->setGravityEnable(cfg.physics.useGravity);
      body->setVelocityLimit(500.0f);
      
      // 史莱姆阻尼由GroundDetectorSystem动态管理
      if (cfg.type == "Slime") {
        body->setLinearDamping(0.3f);
        body->setAngularDamping(0.9f);
      }
    }
    
    body->setCategoryBitmask(0x0002);        // 敌人类别
    body->setContactTestBitmask(0xFFFFFFFF); // 检测所有接触
    body->setCollisionBitmask(0xFFFFFFFB);   // 与所有碰撞，排除玩家(0x0004)
    body->setGroup(cfg.physics.collisionGroup);
    sprite->setPhysicsBody(body);
    
    // ==================== 根据类型添加SpriteComponent ====================
    if (cfg.type == "Slime") {
      // 史莱姆：使用SlimeSpriteComponent
      auto &spriteComp = registry.emplace<ecs::SlimeSpriteComponent>(entity);
      spriteComp.sprite = sprite;
      spriteComp.slimeType = monsterId;  // 设置史莱姆类型
      spriteComp.frameTime = cfg.display.frameTime;
      spriteComp.baseScale = cfg.display.scale;
      
      // 加载所有动画帧
      for (int i = 1; i <= cfg.display.frameCount; i++) {
        std::string framePath = cfg.display.spriteFolder + "/" +
                                cfg.display.spritePrefix + std::to_string(i) + ".png";
        auto texture = Director::getInstance()->getTextureCache()->addImage(framePath);
        if (texture) {
          auto frame = SpriteFrame::createWithTexture(
              texture, Rect(0, 0, texture->getContentSize().width,
                            texture->getContentSize().height));
          if (frame) {
            spriteComp.animFrames.pushBack(frame);
          }
        }
      }
      spriteComp.animationLoaded = spriteComp.animFrames.size() >= 2;
      
      CCLOG("  Loaded %d animation frames for slime %s (frameTime=%.2f)", 
            (int)spriteComp.animFrames.size(), monsterId.c_str(), spriteComp.frameTime);
            
    } else if (cfg.type == "Zombie") {
      // 僵尸：使用MonsterSpriteComponent
      auto &spriteComp = registry.emplace<ecs::MonsterSpriteComponent>(entity);
      spriteComp.sprite = sprite;
      spriteComp.monsterType = monsterId;
      spriteComp.frameTime = cfg.display.frameTime;
      spriteComp.baseScale = cfg.display.scale;
      
      // 加载所有动画帧
      for (int i = 1; i <= cfg.display.frameCount; i++) {
        std::string framePath = cfg.display.spriteFolder + "/" +
                                cfg.display.spritePrefix + std::to_string(i) + ".png";
        auto texture = Director::getInstance()->getTextureCache()->addImage(framePath);
        if (texture) {
          auto frame = SpriteFrame::createWithTexture(
              texture, Rect(0, 0, texture->getContentSize().width,
                            texture->getContentSize().height));
          if (frame) {
            spriteComp.animFrames.pushBack(frame);
          }
        }
      }
      
      // 设置帧序列：如果配置中有自定义序列则使用，否则按顺序播放
      if (!cfg.display.frameSequence.empty()) {
        spriteComp.frameSequence = cfg.display.frameSequence;
      } else if (spriteComp.animFrames.size() > 0) {
        // 自动生成帧序列：1, 2, 3, ..., frameCount
        for (int i = 1; i <= cfg.display.frameCount; i++) {
          spriteComp.frameSequence.push_back(i);
        }
      }
      
      spriteComp.animationLoaded = spriteComp.animFrames.size() > 0 && !spriteComp.frameSequence.empty();
      
      CCLOG("  Loaded %d animation frames for %s (sequence size: %zu, animationLoaded: %s)", 
            (int)spriteComp.animFrames.size(), monsterId.c_str(), spriteComp.frameSequence.size(),
            spriteComp.animationLoaded ? "true" : "false");
    } else if (cfg.type == "DemonEye") {
      // 恶魔眼：使用MonsterSpriteComponent
      auto &spriteComp = registry.emplace<ecs::MonsterSpriteComponent>(entity);
      spriteComp.sprite = sprite;
      spriteComp.monsterType = monsterId;
      spriteComp.frameTime = cfg.display.frameTime;
      spriteComp.baseScale = cfg.display.scale;
      
      // 加载所有动画帧
      for (int i = 1; i <= cfg.display.frameCount; i++) {
        std::string framePath = cfg.display.spriteFolder + "/" +
                                cfg.display.spritePrefix + std::to_string(i) + ".png";
        auto texture = Director::getInstance()->getTextureCache()->addImage(framePath);
        if (texture) {
          auto frame = SpriteFrame::createWithTexture(
              texture, Rect(0, 0, texture->getContentSize().width,
                            texture->getContentSize().height));
          if (frame) {
            spriteComp.animFrames.pushBack(frame);
          }
        }
      }
      
      // 设置帧序列：如果配置中有自定义序列则使用，否则按顺序播放
      if (!cfg.display.frameSequence.empty()) {
        spriteComp.frameSequence = cfg.display.frameSequence;
      } else if (spriteComp.animFrames.size() > 0) {
        // 自动生成帧序列：1, 2, 3, ..., frameCount
        for (int i = 1; i <= cfg.display.frameCount; i++) {
          spriteComp.frameSequence.push_back(i);
        }
      }
      
      spriteComp.animationLoaded = spriteComp.animFrames.size() > 0 && !spriteComp.frameSequence.empty();
      
      CCLOG("  Loaded %d animation frames for DemonEye %s (sequence size: %zu, animationLoaded: %s)", 
            (int)spriteComp.animFrames.size(), monsterId.c_str(), spriteComp.frameSequence.size(),
            spriteComp.animationLoaded ? "true" : "false");
    }
    
    // ==================== 通用：添加基础组件 ====================
    auto &transform = registry.emplace<ecs::TransformComponent>(entity);
    transform.position = Vec2(x, y);
    
    auto &health = registry.emplace<ecs::HealthComponent>(entity);
    health.maxHealth = cfg.stats.maxHealth;
    health.currentHealth = cfg.stats.maxHealth;
    
    auto &aggro = registry.emplace<ecs::AggroComponent>(entity);
    aggro.targetTag = "Player";
    aggro.aggroRange = cfg.ai.aggroRange;
    aggro.deaggroRange = cfg.ai.deaggroRange;
    
    // 飞行类怪物不需要地面检测组件
    if (cfg.type != "DemonEye") {
      auto &ground = registry.emplace<ecs::GroundDetectorComponent>(entity);
      ground.isOnGround = true;
    }
    
    // ==================== 根据移动类型添加移动组件 ====================
    if (cfg.movement.type == "walk") {
      auto &walk = registry.emplace<ecs::WalkMovementComponent>(entity);
      walk.walkSpeed = cfg.movement.walkSpeed;
      walk.jumpForce = cfg.movement.jumpForce;
      walk.obstacleJumpEnabled = cfg.movement.obstacleJumpEnabled;
      walk.targetJumpEnabled = cfg.movement.targetJumpEnabled;
      walk.targetJumpReactionTime = cfg.movement.targetJumpReactionTime;
      walk.patrolDirectionChangeInterval = cfg.movement.patrolDirectionChangeInterval;
      walk.initialized = true;
    } else if (cfg.movement.type == "jump") {
      auto &jump = registry.emplace<ecs::JumpMovementComponent>(entity);
      jump.jumpCooldown = cfg.movement.jumpCooldown;
      jump.maxHorizontalImpulse = cfg.movement.horizontalImpulse;
      jump.maxVerticalImpulse = cfg.movement.verticalImpulse;
      jump.patrolImpulseRatio = cfg.movement.patrolImpulseRatio;
      jump.randomDirectionChangeChance = cfg.movement.directionChangeChance;
      jump.jumpTimer = cfg.movement.jumpCooldown;  // 初始化为冷却完成
    } else if (cfg.movement.type == "fly") {
      // 飞行类型：恶魔眼等
      auto &fly = registry.emplace<ecs::DemonEyeMovementComponent>(entity);
      fly.flySpeed = cfg.movement.flySpeed;
      fly.maxSpeed = cfg.movement.maxSpeed;
      fly.acceleration = cfg.movement.acceleration;
      fly.turnRate = cfg.movement.turnRate;
      fly.wobbleAmplitude = cfg.movement.wobbleAmplitude;
      fly.wobbleFrequency = cfg.movement.wobbleFrequency;
      CCLOG("  Added DemonEyeMovementComponent (flySpeed=%.1f, turnRate=%.2f)", 
            fly.flySpeed, fly.turnRate);
    }
    
    // ==================== 特殊史莱姆：添加特殊组件 ====================
    CCLOG("MonsterFactory: Checking special components for '%s' (type='%s')", 
          monsterId.c_str(), cfg.type.c_str());
    
    if (cfg.type == "Slime") {
      // 伞史莱姆：缓降能力
      if (monsterId.find("Umbrella") != std::string::npos) {
        auto &slowFall = registry.emplace<ecs::SlowFallComponent>(entity);
        slowFall.maxFallSpeed = 80.0f;
        slowFall.fallDamping = 0.7f;
        slowFall.horizontalDamping = 0.98f;
        CCLOG("  Added SlowFallComponent for UmbrellaSlime");
      }
      
      // 尖刺史莱姆：投射物攻击能力（使用配置文件中的参数）
      if (cfg.projectile.enabled || monsterId.find("Spiked") != std::string::npos) {
        auto &projectileAttack = registry.emplace<ecs::ProjectileAttackComponent>(entity);
        
        // 使用配置文件中的投射物参数
        projectileAttack.projectileSpritePath = cfg.projectile.spritePath;
        projectileAttack.projectileSpriteWidth = cfg.projectile.spriteWidth;
        projectileAttack.projectileSpriteHeight = cfg.projectile.spriteHeight;
        projectileAttack.projectileCount = cfg.projectile.count;
        projectileAttack.fireInterval = cfg.projectile.fireInterval;
        projectileAttack.fireRange = cfg.projectile.fireRange;
        projectileAttack.projectileSpeed = cfg.projectile.speed;
        projectileAttack.projectileDamage = cfg.projectile.damage;
        projectileAttack.projectileLifetime = cfg.projectile.lifetime;
        projectileAttack.horizontalSpread = cfg.projectile.horizontalSpread;
        projectileAttack.verticalImpulse = cfg.projectile.verticalImpulse;
        projectileAttack.useGravity = cfg.projectile.useGravity;
        
        // 使用配置文件中的减益效果参数
        projectileAttack.chillChance = cfg.debuff.chillChance;
        projectileAttack.chillDuration = cfg.debuff.chillDuration;
        projectileAttack.chillSpeedReduction = cfg.debuff.chillSpeedReduction;
        projectileAttack.freezeChance = cfg.debuff.freezeChance;
        projectileAttack.freezeDuration = cfg.debuff.freezeDuration;
        projectileAttack.poisonChance1 = cfg.debuff.poisonChance1;
        projectileAttack.poisonDuration1 = cfg.debuff.poisonDuration1;
        projectileAttack.poisonDamage1 = cfg.debuff.poisonDamage1;
        projectileAttack.poisonChance2 = cfg.debuff.poisonChance2;
        projectileAttack.poisonDuration2 = cfg.debuff.poisonDuration2;
        projectileAttack.poisonDamage2 = cfg.debuff.poisonDamage2;
        
        CCLOG("  Added ProjectileAttackComponent for %s (sprite=%s)",
              monsterId.c_str(), cfg.projectile.spritePath.c_str());
      }
    }
    
    // 注册到NodeEntityMap
    ecs::EntityId entityId = entt::to_integral(entity);
    ecs::NodeEntityMap::getInstance().registerNode(sprite, entityId);
    
    CCLOG("MonsterFactory(EnTT): Created %s (%s) at (%.1f, %.1f), entity=%u",
          monsterId.c_str(), cfg.type.c_str(), x, y, entityId);
    
    return entityId;
  }
  
  // 如果创建失败，销毁实体
  registry.destroy(entity);
  CCLOG("MonsterFactory(EnTT): Failed to create %s", monsterId.c_str());
  return ecs::INVALID_ENTITY;
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
