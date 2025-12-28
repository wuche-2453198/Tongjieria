#ifndef __MONSTER_CONFIG_H__
#define __MONSTER_CONFIG_H__

#include "cocos2d.h"
#include <string>
#include <vector>

struct MonsterConfig {
  std::string id;
  std::string type;

  struct Display {
    std::string spriteFolder;
    std::string spritePrefix;
    int frameCount = 2;
    std::vector<int> frameSequence;
    float frameTime = 0.15f;
    float scale = 1.0f;
    float anchorY = 0.5f;
    int zOrder = 1;
    cocos2d::Color3B fallbackColor = cocos2d::Color3B::WHITE;
  } display;

  struct Physics {
    float bodyWidth = 40.0f;
    float bodyHeight = 25.0f;
    float bodyHeightOffset = 0.0f;
    float mass = 1.0f;
    float friction = 1.0f;
    float restitution = 0.0f;
    int collisionGroup = -1;
    bool useGravity = true;
  } physics;

  struct Movement {
    std::string type = "jump";
    float speed = 100.0f;
    float jumpCooldown = 2.0f;
    float horizontalImpulse = 300.0f;
    float verticalImpulse = 500.0f;
    float patrolImpulseRatio = 0.5f;
    float directionChangeChance = 0.3f;

    float walkSpeed = 80.0f;
    float jumpForce = 450.0f;
    bool obstacleJumpEnabled = true;
    bool targetJumpEnabled = true;
    float targetJumpReactionTime = 0.1f;
    float patrolDirectionChangeInterval = 3.0f;

    float flySpeed = 120.0f;
    float maxSpeed = 200.0f;
    float acceleration = 80.0f;
    float turnRate = 1.5f;
    float wobbleAmplitude = 0.3f;
    float wobbleFrequency = 2.0f;

    float detectionRange = 300.0f;
    float shootInterval = 5.0f;
    float projectileSpeed = 400.0f;
    float rotationSpeed = 2.0f;

    float activationRange = 200.0f;
    float hoverDistance = 120.0f;
    float dashTriggerDistance = 80.0f;
    float dashReturnDistance = 120.0f;
  } movement;

  struct AI {
    std::string behavior = "aggressive";
    float aggroRange = 300.0f;
    float deaggroRange = 400.0f;
    std::string targetTag = "Player";
  } ai;

  struct Stats {
    float maxHealth = 50.0f;
    float attackDamage = 10.0f;
    float attackRange = 50.0f;
    float attackCooldown = 1.0f;
    float defense = 0.0f;
  } stats;

  struct LootItem {
    std::string itemId;
    int minCount;
    int maxCount;
    float dropChance;
  };
  std::vector<LootItem> loot;

  struct Projectile {
    bool enabled = false;
    std::string spritePath;
    float spriteWidth = 10.0f;
    float spriteHeight = 20.0f;
    int count = 4;
    float damage = 8.0f;
    float speed = 300.0f;
    float lifetime = 3.0f;
    float fireInterval = 0.8f;
    float fireRange = 200.0f;
    float horizontalSpread = 80.0f;
    float verticalImpulse = 350.0f;
    bool useGravity = true;
  } projectile;

  struct Debuff {
    float chillChance = 0.0f;
    float chillDuration = 0.0f;
    float chillSpeedReduction = 0.0f;
    float freezeChance = 0.0f;
    float freezeDuration = 0.0f;
    float poisonChance1 = 0.0f;
    float poisonDuration1 = 0.0f;
    float poisonDamage1 = 0.0f;
    float poisonChance2 = 0.0f;
    float poisonDuration2 = 0.0f;
    float poisonDamage2 = 0.0f;
  } debuff;

  struct SlowFall {
    bool enabled = false;
    float maxFallSpeed = 100.0f;
    float fallDamping = 0.85f;
    float horizontalDamping = 0.95f;
  } slowFall;

  struct KingSlimeConfig {
    float baseScale = 2.0f;
    float minScale = 0.8f;
    int smallJumpsPerCycle = 3;
    float bigJumpMultiplier = 2.0f;
    float teleportInterval = 15.0f;
    float teleportRange = 300.0f;
    int totalSlimesToSpawn = 80;
    float spawnHealthInterval = 0.012f;
    int maxSpawnPerInterval = 2;
  } kingSlime;
};

#endif // __MONSTER_CONFIG_H__
