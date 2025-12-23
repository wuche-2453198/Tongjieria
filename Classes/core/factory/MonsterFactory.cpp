#include "MonsterFactory.h"
#include "components/AllComponents.h"
#include "systems/render/SpriteManager.h"
#include "systems/core/AnimationConfigLoader.h"
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
      "SpikedSlime.json",
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
  } else if (dirPath.find("eaters") != std::string::npos) {
    knownFiles = {
      "EaterOfSouls_Small.json",
      "EaterOfSouls_Medium.json",
      "EaterOfSouls_Large.json",
      "Crimera_Small.json",
      "Crimera_Medium.json",
      "Crimera_Large.json"
    };
  } else if (dirPath.find("desert") != std::string::npos) {
    knownFiles = {
      "Antlion.json"
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
    if (display.HasMember("anchorY"))
      config.display.anchorY = display["anchorY"].GetFloat();
    if (display.HasMember("zOrder"))
      config.display.zOrder = display["zOrder"].GetInt();
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
    // 蚁狮类型专用参数
    if (movement.HasMember("detectionRange"))
      config.movement.detectionRange = movement["detectionRange"].GetFloat();
    if (movement.HasMember("shootInterval"))
      config.movement.shootInterval = movement["shootInterval"].GetFloat();
    if (movement.HasMember("projectileSpeed"))
      config.movement.projectileSpeed = movement["projectileSpeed"].GetFloat();
    if (movement.HasMember("rotationSpeed"))
      config.movement.rotationSpeed = movement["rotationSpeed"].GetFloat();
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

  // 解析kingSlime（史莱姆王特定配置）
  if (json.HasMember("kingSlime") && json["kingSlime"].IsObject()) {
    const auto &ks = json["kingSlime"];
    if (ks.HasMember("baseScale"))
      config.kingSlime.baseScale = ks["baseScale"].GetFloat();
    if (ks.HasMember("minScale"))
      config.kingSlime.minScale = ks["minScale"].GetFloat();
    
    // 解析jumpPattern
    if (ks.HasMember("jumpPattern") && ks["jumpPattern"].IsObject()) {
      const auto &jp = ks["jumpPattern"];
      if (jp.HasMember("smallJumps"))
        config.kingSlime.smallJumpsPerCycle = jp["smallJumps"].GetInt();
      if (jp.HasMember("bigJumpMultiplier"))
        config.kingSlime.bigJumpMultiplier = jp["bigJumpMultiplier"].GetFloat();
    }
    
    // 解析teleport
    if (ks.HasMember("teleport") && ks["teleport"].IsObject()) {
      const auto &tp = ks["teleport"];
      if (tp.HasMember("interval"))
        config.kingSlime.teleportInterval = tp["interval"].GetFloat();
      if (tp.HasMember("range"))
        config.kingSlime.teleportRange = tp["range"].GetFloat();
    }
    
    // 解析spawning
    if (ks.HasMember("spawning") && ks["spawning"].IsObject()) {
      const auto &sp = ks["spawning"];
      if (sp.HasMember("totalSlimes"))
        config.kingSlime.totalSlimesToSpawn = sp["totalSlimes"].GetInt();
      if (sp.HasMember("healthInterval"))
        config.kingSlime.spawnHealthInterval = sp["healthInterval"].GetFloat();
      if (sp.HasMember("maxSpawnPerInterval"))
        config.kingSlime.maxSpawnPerInterval = sp["maxSpawnPerInterval"].GetInt();
    }
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
  
  // ==================== 注册精灵资源到SpriteManager ====================
  std::string resourceId = monsterId + "_sprite";
  
  // 构建帧路径列表
  std::vector<std::string> framePaths;
  for (int i = 1; i <= cfg.display.frameCount; i++) {
    std::string framePath = cfg.display.spriteFolder + "/" +
                            cfg.display.spritePrefix + std::to_string(i) + ".png";
    framePaths.push_back(framePath);
  }
  
  // 注册资源到SpriteManager
  ecs::SpriteResourceDescriptor descriptor;
  descriptor.resourceId = resourceId;
  descriptor.spritePath = framePaths.empty() ? "" : framePaths[0];
  descriptor.framePaths = framePaths;
  descriptor.anchorPoint = Vec2(0.5f, cfg.display.anchorY);
  ecs::SpriteManager::getInstance().registerResource(descriptor);
  
  // ==================== 新架构：配置物理体组件 ====================
  auto &physics = registry.emplace<ecs::PhysicsBodyComponent>(entity);
  
  // 物理体大小需要考虑精灵的缩放因子
  float scaledWidth = cfg.physics.bodyWidth * cfg.display.scale;
  float scaledHeight = cfg.physics.bodyHeight * cfg.display.scale;
  float scaledHeightOffset = cfg.physics.bodyHeightOffset * cfg.display.scale;
  
  if (cfg.type == "DemonEye") {
    // 恶魔眼：圆形物理体，无重力，有弹性
    physics.shape = ecs::PhysicsBodyComponent::BodyShape::Circle;
    physics.radius = std::min(scaledWidth, scaledHeight) / 2.0f;
    physics.offset = Vec2(scaledHeightOffset, 0);
    physics.density = cfg.physics.mass;
    physics.restitution = cfg.physics.restitution;
    physics.friction = cfg.physics.friction;
    physics.gravityEnabled = false;
    physics.velocityLimit = 500.0f;
    physics.linearDamping = 0.05f;
    physics.angularDamping = 0.3f;
  } else if (cfg.type == "EaterOfSouls") {
    // 噬魂怪：矩形物理体，无重力，有弹性
    physics.shape = ecs::PhysicsBodyComponent::BodyShape::Box;
    physics.width = scaledWidth;
    physics.height = scaledHeight;
    physics.offset = Vec2(0, scaledHeightOffset);
    physics.density = cfg.physics.mass;
    physics.restitution = cfg.physics.restitution;
    physics.friction = cfg.physics.friction;
    physics.gravityEnabled = false;
    physics.velocityLimit = 500.0f;
    physics.linearDamping = 0.05f;
    physics.angularDamping = 0.3f;
  } else if (cfg.type == "Antlion") {
    // 蚁狮：圆形物理体，位于中心，有重力，落地后静止
    physics.shape = ecs::PhysicsBodyComponent::BodyShape::Circle;
    physics.radius = std::min(scaledWidth, scaledHeight) / 2.0f; // 半径设为贴图的1/2
    physics.offset = Vec2(0, 0); // 物理体位于中心
    physics.density = cfg.physics.mass;
    physics.restitution = 0.0f; // 无弹性
    physics.friction = 10.0f; // 极高摩擦力，落地后不滑动
    physics.gravityEnabled = true; // 允许垂直重力
    physics.velocityLimit = 500.0f; // 允许下落，但限制最大速度
    physics.linearDamping = 5.0f; // 中等阻尼，落地后快速停止
    physics.angularDamping = 10.0f; // 防止旋转
    physics.rotationEnabled = false; // 锁定旋转
  } else {
    // 调整各怪物摩擦力和弹力以适应统一地形参数
    float friction = cfg.physics.friction;
    float restitution = cfg.physics.restitution;
    
    if (cfg.type == "Slime") {
      friction = 2.25f;
      restitution = 0.0f;
    } else if (cfg.type == "Zombie") {
      friction = 0.5f;
      restitution = 0.0f;
    }
    
    CCLOG("MonsterFactory: %s physics adjusted - friction: %.2f->%.2f, restitution: %.2f->%.2f", 
          cfg.type.c_str(), cfg.physics.friction, friction, cfg.physics.restitution, restitution);
    
    physics.shape = ecs::PhysicsBodyComponent::BodyShape::Box;
    physics.width = scaledWidth;
    physics.height = scaledHeight;
    physics.offset = Vec2(0, scaledHeightOffset);
    physics.density = cfg.physics.mass;
    physics.restitution = restitution;
    physics.friction = friction;
    physics.gravityEnabled = cfg.physics.useGravity;
    
    if (cfg.type == "KingSlime") {
      physics.velocityLimit = 1200.0f;
    } else {
      physics.velocityLimit = 500.0f;
    }
    
    if (cfg.type == "Slime") {
      physics.linearDamping = 0.3f;
      physics.angularDamping = 0.9f;
    }
  }
  
  physics.dynamic = true;
  physics.rotationEnabled = false;
  physics.categoryBitmask = 0x0002;
  physics.contactTestBitmask = 0xFFFFFFFF;
  physics.collisionBitmask = 0xFFFFFFFB;
  physics.group = cfg.physics.collisionGroup;
  
  // ==================== 新架构：添加渲染组件 ====================
  // 1. RenderComponent - 纯数据渲染配置
  auto &render = registry.emplace<ecs::RenderComponent>(entity);
  render.spriteResourceId = resourceId;
  render.scale = cfg.display.scale;
  render.flipX = false;
  render.visible = true;
  render.zOrder = cfg.display.zOrder;
  render.color = Color3B::WHITE;
  render.opacity = 255;
  
  // 2. ParentNodeComponent - 父节点引用
  auto &parentComp = registry.emplace<ecs::ParentNodeComponent>(entity);
  parentComp.parentNode = parentNode;
  parentComp.attachedToParent = false;  // RenderSystem会设置为true
  
  // 3. AnimationComponent - 帧动画配置
  if (cfg.display.frameCount > 1) {
    auto &anim = registry.emplace<ecs::AnimationComponent>(entity);
    anim.animationSetId = resourceId;  // 使用相同的资源ID
    anim.frameTime = cfg.display.frameTime;
    anim.isPlaying = true;
    anim.loop = true;
    
    // 设置帧序列
    if (!cfg.display.frameSequence.empty()) {
      anim.frameSequence = cfg.display.frameSequence;
    } else {
      // 自动生成帧序列：1, 2, 3, ..., frameCount
      for (int i = 1; i <= cfg.display.frameCount; i++) {
        anim.frameSequence.push_back(i);
      }
    }
    
    CCLOG("  Added AnimationComponent with %zu frames (frameTime=%.2f)",
          anim.frameSequence.size(), anim.frameTime);
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
  
  // 飞行类和静止类怪物不需要地面检测组件
  if (cfg.type != "DemonEye" && cfg.type != "EaterOfSouls" && cfg.type != "Antlion") {
    auto &ground = registry.emplace<ecs::GroundDetectorComponent>(entity);
    ground.isOnGround = true;
  }
  
  // ==================== 根据移动类型添加移动组件 ====================
  if (cfg.movement.type == "walk") {
    auto &walk = registry.emplace<ecs::WarriorMovementComponent>(entity);
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
    jump.jumpTimer = cfg.movement.jumpCooldown;  // 初始化为冷却完成，准备跳跃
    jump.readyToJump = true;  
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
  } else if (cfg.movement.type == "circle") {
    // 绕圈飞行类型：噬魂怪
    // 根据怪物ID判断尺寸变体
    ecs::EaterOfSoulsMovementComponent::SizeVariant variant = ecs::EaterOfSoulsMovementComponent::MEDIUM;
    if (monsterId.find("Small") != std::string::npos) {
      variant = ecs::EaterOfSoulsMovementComponent::SMALL;
    } else if (monsterId.find("Large") != std::string::npos) {
      variant = ecs::EaterOfSoulsMovementComponent::LARGE;
    }
    
    auto &eater = registry.emplace<ecs::EaterOfSoulsMovementComponent>(entity, variant);
    // 可以从配置文件覆盖默认值
    if (cfg.movement.flySpeed > 0) eater.flySpeed = cfg.movement.flySpeed;
    if (cfg.movement.maxSpeed > 0) eater.maxSpeed = cfg.movement.maxSpeed;
    if (cfg.movement.acceleration > 0) eater.acceleration = cfg.movement.acceleration;
    if (cfg.movement.turnRate > 0) eater.turnRate = cfg.movement.turnRate;
    CCLOG("  Added EaterOfSoulsMovementComponent (variant=%d, flySpeed=%.1f)", 
          variant, eater.flySpeed);
  } else if (cfg.movement.type == "antlion") {
    // 蚁狮类型：静止射击怪物
    auto &antlion = registry.emplace<ecs::AntlionMovementComponent>(entity);
    
    // 从配置文件读取蚁狮参数
    if (cfg.movement.detectionRange > 0) antlion.detectionRange = cfg.movement.detectionRange;
    if (cfg.movement.shootInterval > 0) antlion.shootInterval = cfg.movement.shootInterval;
    if (cfg.movement.projectileSpeed > 0) antlion.projectileSpeed = cfg.movement.projectileSpeed;
    if (cfg.movement.rotationSpeed > 0) antlion.rotationSpeed = cfg.movement.rotationSpeed;
    
    CCLOG("  Added AntlionMovementComponent (detectionRange=%.1f, shootInterval=%.1f)", 
          antlion.detectionRange, antlion.shootInterval);
    
    // 为蚁狮注册多个动画资源ID（idle/tracking/shooting）
    ecs::SpriteResourceDescriptor idleDesc = descriptor;
    idleDesc.resourceId = "antlion_idle";
    ecs::SpriteManager::getInstance().registerResource(idleDesc);
    
    ecs::SpriteResourceDescriptor trackingDesc = descriptor;
    trackingDesc.resourceId = "antlion_tracking";
    ecs::SpriteManager::getInstance().registerResource(trackingDesc);
    
    ecs::SpriteResourceDescriptor shootingDesc = descriptor;
    shootingDesc.resourceId = "antlion_shooting";
    ecs::SpriteManager::getInstance().registerResource(shootingDesc);
    
    CCLOG("  Registered animation resources: antlion_idle, antlion_tracking, antlion_shooting");
    
    // 添加AnimationStateComponent并加载动画配置
    auto &animState = registry.emplace<ecs::AnimationStateComponent>(entity);
    if (ecs::AnimationConfigLoader::loadFromJson("animations/antlion_animations.json", animState)) {
      animState.enableStateDriven = true;
      CCLOG("  Loaded AnimationStateComponent for antlion from JSON");
    } else {
      CCLOG("  WARNING: Failed to load antlion animation config");
    }
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
  
  // ==================== 史莱姆王Boss：添加Boss组件 ====================
  if (cfg.type == "KingSlime") {
    auto &kingSlime = registry.emplace<ecs::KingSlimeComponent>(entity);
    
    // 从JSON配置中读取KingSlime特殊参数
    kingSlime.baseScale = cfg.kingSlime.baseScale;
    kingSlime.minScale = cfg.kingSlime.minScale;
    kingSlime.smallJumpsPerCycle = cfg.kingSlime.smallJumpsPerCycle;
    kingSlime.bigJumpMultiplier = cfg.kingSlime.bigJumpMultiplier;
    kingSlime.teleportInterval = cfg.kingSlime.teleportInterval;
    kingSlime.teleportRange = cfg.kingSlime.teleportRange;
    kingSlime.totalSlimesToSpawn = cfg.kingSlime.totalSlimesToSpawn;
    kingSlime.spawnHealthInterval = cfg.kingSlime.spawnHealthInterval;
    kingSlime.maxSpawnPerInterval = cfg.kingSlime.maxSpawnPerInterval;
    
    // 从JSON读取跳跃参数
    kingSlime.baseHorizontalImpulse = cfg.movement.horizontalImpulse;
    kingSlime.baseVerticalImpulse = cfg.movement.verticalImpulse;
    
    // 初始化缩放：根据满血状态（healthPercent = 1.0）计算
    // 这样确保满血时的缩放是正确的
    kingSlime.updateStage(1.0f);  // 满血时调用updateStage
    
    // 设置初始缩放到RenderComponent
    render.scale = kingSlime.currentScale;
    
    CCLOG("  Added KingSlimeComponent (baseScale=%.1f, bigJumpMult=%.1f, spawns=%d slimes)", 
          kingSlime.baseScale, kingSlime.bigJumpMultiplier, kingSlime.totalSlimesToSpawn);
  }
    
  // 返回实体ID
  ecs::EntityId entityId = entt::to_integral(entity);
  CCLOG("MonsterFactory: Created %s at (%.1f, %.1f) with EntityId %u", 
        monsterId.c_str(), x, y, entityId);
  
  return entityId;
}

const MonsterConfig *MonsterFactory::getConfig(const std::string &monsterId) const {
  auto it = _configs.find(monsterId);
  if (it != _configs.end()) {
    return &it->second;
  }
  return nullptr;
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
