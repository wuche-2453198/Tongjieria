#include "BaseMonsterFactory.h"
#include "components/AllComponents.h"
#include "systems/render/SpriteManager.h"
#include "platform/CCFileUtils.h"
#include "json/document.h"

USING_NS_CC;

// ==================== IMonsterFactory 接口实现 ====================

bool BaseMonsterFactory::supports(const std::string &monsterId) const {
  return _configs.find(monsterId) != _configs.end();
}

const MonsterConfig *BaseMonsterFactory::getConfig(const std::string &monsterId) const {
  auto it = _configs.find(monsterId);
  if (it != _configs.end()) {
    return &it->second;
  }
  return nullptr;
}

std::vector<std::string> BaseMonsterFactory::getSupportedIds() const {
  std::vector<std::string> ids;
  ids.reserve(_configs.size());
  for (const auto &pair : _configs) {
    ids.push_back(pair.first);
  }
  return ids;
}

// ==================== 配置加载方法 ====================

int BaseMonsterFactory::loadConfigs(const std::string &dirPath) {
  int loadedCount = 0;
  std::vector<std::string> knownFiles;

  if (dirPath.find("slimes") != std::string::npos) {
    knownFiles = {
      "GreenSlime.json", "BlueSlime.json", "RedSlime.json",
      "YellowSlime.json", "PurpleSlime.json", "PinkSlime.json",
      "IceSlime.json", "SpikedSlime.json", "SpikedIceSlime.json",
      "SpikedJungleSlime.json", "UmbrellaSlime.json",
      "MotherSlime.json", "BabySlime.json"
    };
  } else if (dirPath.find("zombies") != std::string::npos) {
    knownFiles = {
      "Zombie.json", "31px-Zombie.json", "Bigger-Zombie.json",
      "BaldZombie.json", "29px-BaldZombie.json", "Bigger-BaldZombie.json",
      "PincushionZombie.json", "32px-PincushionZombie.json", "Bigger-PincushionZombie.json",
      "Face_Monster.json"
    };
  } else if (dirPath.find("eyes") != std::string::npos) {
    knownFiles = {
      "DemonEye.json", "PurpleEye.json", "GreenEye.json",
      "CataractEye.json", "DilatedEye.json", "SleepyEye.json",
      "Bigger-DemonEye.json", "Bigger-Crimera.json", "Bigger-Eater_of_Souls.json",
      "Bigger-CataractEye.json", "Bigger-SleepyEye.json"
    };
  } else if (dirPath.find("skeletons") != std::string::npos) {
    knownFiles = {
      "Skeleton.json",
      "Angry_Bones_1.json", "Angry_Bones_2.json", "Angry_Bones_3.json", "Angry_Bones_4.json",
      "27px-Angry_Bones_1.json"
    };
  } else if (dirPath.find("eaters") != std::string::npos) {
    knownFiles = {
      "EaterOfSouls_Small.json", "EaterOfSouls_Medium.json", "EaterOfSouls_Large.json",
      "Crimera_Small.json", "Crimera_Medium.json", "Crimera_Large.json",
      "36px-Eater_of_Souls.json", "32px-Crimera.json"
    };
  } else if (dirPath.find("desert") != std::string::npos) {
    knownFiles = {"Antlion.json", "Vulture.json"};
  } else if (dirPath.find("bosses") != std::string::npos) {
    knownFiles = {"KingSlime.json"};
  }

  for (const auto &filename : knownFiles) {
    std::string filePath = dirPath + "/" + filename;
    if (loadSingleConfig(filePath)) {
      loadedCount++;
    }
  }

  CCLOG("BaseMonsterFactory: Loaded %d configs from '%s'", loadedCount, dirPath.c_str());
  return loadedCount;
}


bool BaseMonsterFactory::loadSingleConfig(const std::string &filePath) {
  std::string fullPath = FileUtils::getInstance()->fullPathForFilename(filePath);
  std::string content = FileUtils::getInstance()->getStringFromFile(fullPath);

  if (content.empty()) {
    CCLOG("BaseMonsterFactory: Failed to load file: %s", filePath.c_str());
    return false;
  }

  rapidjson::Document doc;
  doc.Parse(content.c_str());

  if (doc.HasParseError()) {
    CCLOG("BaseMonsterFactory: JSON parse error in %s", filePath.c_str());
    return false;
  }

  if (!doc.HasMember("id") || !doc["id"].IsString()) {
    CCLOG("BaseMonsterFactory: Missing 'id' field in %s", filePath.c_str());
    return false;
  }

  MonsterConfig config;
  config.id = doc["id"].GetString();

  // 解析type
  if (doc.HasMember("type") && doc["type"].IsString()) {
    config.type = doc["type"].GetString();
  }

  // 解析display
  if (doc.HasMember("display") && doc["display"].IsObject()) {
    const auto &display = doc["display"];
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
    if (display.HasMember("frameSequence") && display["frameSequence"].IsArray()) {
      const auto &seq = display["frameSequence"];
      for (rapidjson::SizeType i = 0; i < seq.Size(); i++) {
        config.display.frameSequence.push_back(seq[i].GetInt());
      }
    }
  }

  // 解析physics
  if (doc.HasMember("physics") && doc["physics"].IsObject()) {
    const auto &physics = doc["physics"];
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
    if (physics.HasMember("useGravity"))
      config.physics.useGravity = physics["useGravity"].GetBool();
  }

  // 解析movement
  if (doc.HasMember("movement") && doc["movement"].IsObject()) {
    const auto &movement = doc["movement"];
    if (movement.HasMember("type"))
      config.movement.type = movement["type"].GetString();
    if (movement.HasMember("jumpCooldown"))
      config.movement.jumpCooldown = movement["jumpCooldown"].GetFloat();
    if (movement.HasMember("horizontalImpulse"))
      config.movement.horizontalImpulse = movement["horizontalImpulse"].GetFloat();
    if (movement.HasMember("verticalImpulse"))
      config.movement.verticalImpulse = movement["verticalImpulse"].GetFloat();
    if (movement.HasMember("patrolImpulseRatio"))
      config.movement.patrolImpulseRatio = movement["patrolImpulseRatio"].GetFloat();
    if (movement.HasMember("directionChangeChance"))
      config.movement.directionChangeChance = movement["directionChangeChance"].GetFloat();
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
    // 秃鹰类型专用参数
    if (movement.HasMember("activationRange"))
      config.movement.activationRange = movement["activationRange"].GetFloat();
    if (movement.HasMember("hoverDistance"))
      config.movement.hoverDistance = movement["hoverDistance"].GetFloat();
    if (movement.HasMember("dashTriggerDistance"))
      config.movement.dashTriggerDistance = movement["dashTriggerDistance"].GetFloat();
    if (movement.HasMember("dashReturnDistance"))
      config.movement.dashReturnDistance = movement["dashReturnDistance"].GetFloat();
  }

  // 解析ai
  if (doc.HasMember("ai") && doc["ai"].IsObject()) {
    const auto &ai = doc["ai"];
    if (ai.HasMember("aggroRange"))
      config.ai.aggroRange = ai["aggroRange"].GetFloat();
    if (ai.HasMember("deaggroRange"))
      config.ai.deaggroRange = ai["deaggroRange"].GetFloat();
    if (ai.HasMember("targetTag"))
      config.ai.targetTag = ai["targetTag"].GetString();
  }

  // 解析stats
  if (doc.HasMember("stats") && doc["stats"].IsObject()) {
    const auto &stats = doc["stats"];
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
  if (doc.HasMember("loot") && doc["loot"].IsArray()) {
    const auto &lootArray = doc["loot"];
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

  // 解析projectile
  if (doc.HasMember("projectile") && doc["projectile"].IsObject()) {
    config.projectile.enabled = true;
    const auto &proj = doc["projectile"];
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

  // 解析debuff
  if (doc.HasMember("debuff") && doc["debuff"].IsObject()) {
    const auto &debuff = doc["debuff"];
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

  // 解析slowFall
  if (doc.HasMember("slowFall") && doc["slowFall"].IsObject()) {
    config.slowFall.enabled = true;
    const auto &sf = doc["slowFall"];
    if (sf.HasMember("maxFallSpeed"))
      config.slowFall.maxFallSpeed = sf["maxFallSpeed"].GetFloat();
    if (sf.HasMember("fallDamping"))
      config.slowFall.fallDamping = sf["fallDamping"].GetFloat();
    if (sf.HasMember("horizontalDamping"))
      config.slowFall.horizontalDamping = sf["horizontalDamping"].GetFloat();
  }

  // 解析kingSlime
  if (doc.HasMember("kingSlime") && doc["kingSlime"].IsObject()) {
    const auto &ks = doc["kingSlime"];
    if (ks.HasMember("baseScale"))
      config.kingSlime.baseScale = ks["baseScale"].GetFloat();
    if (ks.HasMember("minScale"))
      config.kingSlime.minScale = ks["minScale"].GetFloat();
    if (ks.HasMember("jumpPattern") && ks["jumpPattern"].IsObject()) {
      const auto &jp = ks["jumpPattern"];
      if (jp.HasMember("smallJumps"))
        config.kingSlime.smallJumpsPerCycle = jp["smallJumps"].GetInt();
      if (jp.HasMember("bigJumpMultiplier"))
        config.kingSlime.bigJumpMultiplier = jp["bigJumpMultiplier"].GetFloat();
    }
    if (ks.HasMember("teleport") && ks["teleport"].IsObject()) {
      const auto &tp = ks["teleport"];
      if (tp.HasMember("interval"))
        config.kingSlime.teleportInterval = tp["interval"].GetFloat();
      if (tp.HasMember("range"))
        config.kingSlime.teleportRange = tp["range"].GetFloat();
    }
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

  _configs[config.id] = config;
  CCLOG("BaseMonsterFactory: Loaded config '%s'", config.id.c_str());
  return true;
}


// ==================== 通用组件附加方法 ====================

std::string BaseMonsterFactory::registerSpriteResource(const MonsterConfig &config) {
  std::string resourceId = config.id + "_sprite";

  // 构建帧路径列表
  std::vector<std::string> framePaths;
  for (int i = 1; i <= config.display.frameCount; i++) {
    std::string framePath = config.display.spriteFolder + "/" +
                            config.display.spritePrefix + std::to_string(i) + ".png";
    framePaths.push_back(framePath);
  }

  // 注册资源到SpriteManager
  ecs::SpriteResourceDescriptor descriptor;
  descriptor.resourceId = resourceId;
  descriptor.spritePath = framePaths.empty() ? "" : framePaths[0];
  descriptor.framePaths = framePaths;
  descriptor.anchorPoint = Vec2(0.5f, config.display.anchorY);
  ecs::SpriteManager::getInstance().registerResource(descriptor);

  return resourceId;
}

void BaseMonsterFactory::attachCoreComponents(entt::registry &registry, entt::entity entity,
                                              const MonsterConfig &config, float x, float y) {
  // 1. TransformComponent - 位置信息（高频访问）
  auto &transform = registry.emplace<ecs::TransformComponent>(entity);
  transform.position = Vec2(x, y);

  // 2. EntityStateFlags - 状态标记（高频访问）
  auto &stateFlags = registry.emplace<ecs::EntityStateFlags>(entity);
  stateFlags.isOnScreen = true;
  stateFlags.isActive = true;
  stateFlags.isIdle = false;
  stateFlags.distanceToPlayer = 0.0f;

  (void)config; // 暂时未使用
}

void BaseMonsterFactory::attachPhysicsComponents(entt::registry &registry, entt::entity entity,
                                                 const MonsterConfig &config) {
  auto &physics = registry.emplace<ecs::PhysicsBodyComponent>(entity);

  // 物理体大小需要考虑精灵的缩放因子
  float scaledWidth = config.physics.bodyWidth * config.display.scale;
  float scaledHeight = config.physics.bodyHeight * config.display.scale;
  float scaledHeightOffset = config.physics.bodyHeightOffset * config.display.scale;

  // 根据怪物类型配置物理体
  if (config.type == "DemonEye") {
    // 恶魔眼：圆形物理体，无重力
    physics.shape = ecs::PhysicsBodyComponent::BodyShape::Circle;
    physics.radius = std::min(scaledWidth, scaledHeight) / 2.0f;
    physics.offset = Vec2(scaledHeightOffset, 0);
    physics.density = config.physics.mass;
    physics.restitution = config.physics.restitution;
    physics.friction = config.physics.friction;
    physics.gravityEnabled = false;
    physics.velocityLimit = 500.0f;
    physics.linearDamping = 0.05f;
    physics.angularDamping = 0.3f;
  } else if (config.type == "EaterOfSouls") {
    // 噬魂怪：矩形物理体，无重力
    physics.shape = ecs::PhysicsBodyComponent::BodyShape::Box;
    physics.width = scaledWidth;
    physics.height = scaledHeight;
    physics.offset = Vec2(0, scaledHeightOffset);
    physics.density = config.physics.mass;
    physics.restitution = config.physics.restitution;
    physics.friction = config.physics.friction;
    physics.gravityEnabled = false;
    physics.velocityLimit = 500.0f;
    physics.linearDamping = 0.05f;
    physics.angularDamping = 0.3f;
  } else if (config.type == "Antlion") {
    // 蚁狮：圆形物理体，有重力，高摩擦力
    physics.shape = ecs::PhysicsBodyComponent::BodyShape::Circle;
    physics.radius = std::min(scaledWidth, scaledHeight) / 2.0f;
    physics.offset = Vec2(0, 0);
    physics.density = config.physics.mass;
    physics.restitution = 0.0f;
    physics.friction = 10.0f;
    physics.gravityEnabled = true;
    physics.velocityLimit = 500.0f;
    physics.linearDamping = 5.0f;
    physics.angularDamping = 10.0f;
    physics.rotationEnabled = false;
  } else if (config.type == "Vulture") {
    // 秃鹰：矩形物理体，无重力
    physics.shape = ecs::PhysicsBodyComponent::BodyShape::Box;
    physics.width = scaledWidth;
    physics.height = scaledHeight;
    physics.offset = Vec2(0, 0);
    physics.density = config.physics.mass;
    physics.restitution = config.physics.restitution;
    physics.friction = config.physics.friction;
    physics.gravityEnabled = false;
    physics.velocityLimit = 500.0f;
    physics.linearDamping = 0.05f;
    physics.angularDamping = 0.3f;
  } else {
    // 默认：矩形物理体（史莱姆、僵尸等）
    float friction = config.physics.friction;
    float restitution = config.physics.restitution;

    // 调整各怪物摩擦力和弹力
    if (config.type == "Slime") {
      friction = 0.8f;
      restitution = 0.0f;
    } else if (config.type == "Zombie") {
      friction = 0.5f;
      restitution = 0.0f;
    }

    physics.shape = ecs::PhysicsBodyComponent::BodyShape::Box;
    if (config.type == "Slime") {
      float w = scaledWidth - 2.0f * config.display.scale;
      if (w < 1.0f) w = 1.0f;
      physics.width = w;
    } else {
      physics.width = scaledWidth;
    }
    physics.height = scaledHeight;
    physics.offset = Vec2(0, scaledHeightOffset);
    physics.density = config.physics.mass;
    physics.restitution = restitution;
    physics.friction = friction;
    physics.gravityEnabled = config.physics.useGravity;

    if (config.type == "KingSlime") {
      physics.velocityLimit = 1200.0f;
    } else {
      physics.velocityLimit = 500.0f;
    }

    if (config.type == "Slime") {
      physics.linearDamping = 0.3f;
      physics.angularDamping = 0.9f;
    }
  }

  // 通用物理属性
  physics.dynamic = true;
  physics.rotationEnabled = false;
  physics.categoryBitmask = 0x0002;
  physics.contactTestBitmask = 0xFFFFFFFF;
  physics.collisionBitmask = 0x0001;
  physics.group = config.physics.collisionGroup;
}

void BaseMonsterFactory::attachRenderComponents(entt::registry &registry, entt::entity entity,
                                                const MonsterConfig &config,
                                                cocos2d::Node *parentNode) {
  // 注册精灵资源
  std::string resourceId = registerSpriteResource(config);

  // 1. RenderComponent - 渲染配置
  auto &render = registry.emplace<ecs::RenderComponent>(entity);
  render.spriteResourceId = resourceId;
  render.scale = config.display.scale;
  render.flipX = false;
  render.visible = true;
  render.zOrder = config.display.zOrder;
  render.color = Color3B::WHITE;
  render.opacity = 255;

  // 2. ParentNodeComponent - 父节点引用
  auto &parentComp = registry.emplace<ecs::ParentNodeComponent>(entity);
  parentComp.parentNode = parentNode;
  parentComp.attachedToParent = false;

  // 3. AnimationComponent - 帧动画配置
  if (config.display.frameCount > 1) {
    auto &anim = registry.emplace<ecs::AnimationComponent>(entity);
    anim.animationSetId = resourceId;
    anim.frameTime = config.display.frameTime;
    anim.isPlaying = true;
    anim.loop = true;

    // 设置帧序列
    if (!config.display.frameSequence.empty()) {
      anim.frameSequence = config.display.frameSequence;
    } else {
      for (int i = 1; i <= config.display.frameCount; i++) {
        anim.frameSequence.push_back(i);
      }
    }
  }
}

void BaseMonsterFactory::attachCombatComponents(entt::registry &registry, entt::entity entity,
                                                const MonsterConfig &config) {
  // 1. HealthComponent - 生命值
  auto &health = registry.emplace<ecs::HealthComponent>(entity);
  health.maxHealth = config.stats.maxHealth;
  health.currentHealth = config.stats.maxHealth;

  auto &combat = registry.emplace<ecs::CombatComponent>(entity);
  combat.attackRange = config.stats.attackRange;
  combat.attackDamage = config.stats.attackDamage;
  combat.attackCooldown = config.stats.attackCooldown;
  combat.attackTimer = 0.0f;
  combat.canAttack = true;

  // 2. AggroComponent - 仇恨
  auto &aggro = registry.emplace<ecs::AggroComponent>(entity);
  aggro.targetTag = config.ai.targetTag;
  aggro.aggroRange = config.ai.aggroRange;
  aggro.deaggroRange = config.ai.deaggroRange;
}
