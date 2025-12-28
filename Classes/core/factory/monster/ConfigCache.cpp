#include "ConfigCache.h"
#include "platform/CCFileUtils.h"
#include "json/document.h"
#include <vector>

USING_NS_CC;

const MonsterConfig *ConfigCache::getConfig(const std::string &filePath) {
  // 如果之前加载失败，直接返回
  if (_failedFiles.find(filePath) != _failedFiles.end()) {
    return nullptr;
  }

  // 若已缓存，直接返回
  auto itId = _fileToId.find(filePath);
  if (itId != _fileToId.end()) {
    auto itCfg = _cache.find(itId->second);
    if (itCfg != _cache.end()) {
      return &itCfg->second;
    }
  }

  // 尝试加载
  if (!loadSingleConfig(filePath)) {
    _failedFiles.insert(filePath);
    return nullptr;
  }

  const auto idIt = _fileToId.find(filePath);
  if (idIt == _fileToId.end()) {
    return nullptr;
  }
  const auto cfgIt = _cache.find(idIt->second);
  return cfgIt == _cache.end() ? nullptr : &cfgIt->second;
}

int ConfigCache::preloadDirectory(const std::string &dirPath) {
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
      "PincushionZombie.json", "32px-PincushionZombie.json", "Bigger-PincushionZombie.json"
    };
  } else if (dirPath.find("eyes") != std::string::npos) {
    knownFiles = {
      "DemonEye.json", "Bigger-DemonEye.json", "PurpleEye.json",
      "GreenEye.json", "CataractEye.json", "DilatedEye.json"
    };
  } else if (dirPath.find("eaters") != std::string::npos) {
    knownFiles = {
      "EaterOfSouls_Small.json", "EaterOfSouls_Medium.json", "EaterOfSouls_Large.json",
      "Crimera_Small.json", "Crimera_Medium.json", "Crimera_Large.json"
    };
  } else if (dirPath.find("desert") != std::string::npos) {
    knownFiles = {"Antlion.json", "Vulture.json"};
  } else if (dirPath.find("bosses") != std::string::npos) {
    knownFiles = {"KingSlime.json"};
  }

  int loadedCount = 0;
  for (const auto &filename : knownFiles) {
    std::string path = dirPath + "/" + filename;
    if (getConfig(path)) {
      loadedCount++;
    }
  }
  return loadedCount;
}

void ConfigCache::clear() {
  _cache.clear();
  _fileToId.clear();
  _failedFiles.clear();
}

bool ConfigCache::isLoaded() const {
  return !_cache.empty();
}

size_t ConfigCache::getCacheSize() const {
  return _cache.size();
}

const MonsterConfig *ConfigCache::getCachedConfigById(const std::string &monsterId) const {
  auto it = _cache.find(monsterId);
  return it == _cache.end() ? nullptr : &it->second;
}

bool ConfigCache::loadSingleConfig(const std::string &filePath) {
  std::string fullPath = FileUtils::getInstance()->fullPathForFilename(filePath);
  std::string content = FileUtils::getInstance()->getStringFromFile(fullPath);

  if (content.empty()) {
    CCLOG("ConfigCache: Failed to load file: %s", filePath.c_str());
    return false;
  }

  rapidjson::Document doc;
  doc.Parse(content.c_str());
  if (doc.HasParseError()) {
    CCLOG("ConfigCache: JSON parse error in %s", filePath.c_str());
    return false;
  }

  if (!doc.HasMember("id") || !doc["id"].IsString()) {
    CCLOG("ConfigCache: Missing 'id' field in %s", filePath.c_str());
    return false;
  }

  MonsterConfig config;
  config.id = doc["id"].GetString();

  if (!parseMonsterConfig(doc, config)) {
    CCLOG("ConfigCache: Failed to parse config fields in %s", filePath.c_str());
    return false;
  }

  _cache[config.id] = config;
  _fileToId[filePath] = config.id;
  return true;
}

bool ConfigCache::parseMonsterConfig(rapidjson::Document &doc, MonsterConfig &config) {
  // type
  if (doc.HasMember("type") && doc["type"].IsString()) {
    config.type = doc["type"].GetString();
  }

  // display
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

  // physics
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

  // movement
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
    if (movement.HasMember("detectionRange"))
      config.movement.detectionRange = movement["detectionRange"].GetFloat();
    if (movement.HasMember("shootInterval"))
      config.movement.shootInterval = movement["shootInterval"].GetFloat();
    if (movement.HasMember("projectileSpeed"))
      config.movement.projectileSpeed = movement["projectileSpeed"].GetFloat();
    if (movement.HasMember("rotationSpeed"))
      config.movement.rotationSpeed = movement["rotationSpeed"].GetFloat();
    if (movement.HasMember("activationRange"))
      config.movement.activationRange = movement["activationRange"].GetFloat();
    if (movement.HasMember("hoverDistance"))
      config.movement.hoverDistance = movement["hoverDistance"].GetFloat();
    if (movement.HasMember("dashTriggerDistance"))
      config.movement.dashTriggerDistance = movement["dashTriggerDistance"].GetFloat();
    if (movement.HasMember("dashReturnDistance"))
      config.movement.dashReturnDistance = movement["dashReturnDistance"].GetFloat();
  }

  // ai
  if (doc.HasMember("ai") && doc["ai"].IsObject()) {
    const auto &ai = doc["ai"];
    if (ai.HasMember("aggroRange"))
      config.ai.aggroRange = ai["aggroRange"].GetFloat();
    if (ai.HasMember("deaggroRange"))
      config.ai.deaggroRange = ai["deaggroRange"].GetFloat();
    if (ai.HasMember("targetTag"))
      config.ai.targetTag = ai["targetTag"].GetString();
  }

  // stats
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
    if (stats.HasMember("defense"))
      config.stats.defense = stats["defense"].GetFloat();
  }

  // loot
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

  // projectile
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

  // debuff
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

  // slowFall
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

  // kingSlime
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

  return true;
}
