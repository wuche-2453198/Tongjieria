#ifndef __ECS_ALL_SYSTEMS_H__
#define __ECS_ALL_SYSTEMS_H__

/**
 * @file AllSystems.h
 * @brief 聚合所有系统的便捷头文件
 * 
 * 使用方法：
 * #include "ecs/AllSystems.h"
 */

// 系统基类和优先级
#include "systems/core/ISystemEntt.h"
#include "systems/core/SystemPriority.h"

// 核心系统
#include "systems/core/LifetimeSystemEntt.h"
#include "systems/core/AnimationConfigLoader.h"
#include "systems/core/AnimationStateHelper.h"
#include "systems/core/ObjectPool.h"
#include "systems/core/EntityPoolManager.h"
#include "systems/core/EntityStateUpdateSystem.h"
#include "systems/core/EventSystem.h"              // 事件系统
#include "systems/core/EntityDestructionManager.h" // 延迟实体销毁管理器

// 战斗系统
#include "systems/combat/HealthSystemEntt.h"
#include "systems/combat/CombatSystemEntt.h"
#include "systems/combat/DebuffSystemEntt.h"

// 渲染系统
#include "systems/render/RenderSystem.h"
#include "systems/render/SpriteManager.h"
#include "systems/render/GameAtlasManager.h"       // 统一图集管理器
#include "systems/render/BatchRenderManager.h"
#include "systems/render/SlimeAtlasHelper.h"       // 史莱姆图集批处理辅助
#include "systems/render/AnimationCacheManager.h"  // 动画缓存管理器

// 物理系统
#include "systems/physics/PhysicsContactHandler.h"
#include "systems/physics/GroundDetectorSystemEntt.h"
#include "systems/physics/SlowFallSystemEntt.h"
#include "systems/physics/JumpMovementSystemEntt.h"
#include "systems/physics/PhysicsSyncSystemEntt.h"   // 统一物理同步系统

// NPC系统
#include "systems/npc/AggroSystemEntt.h"
#include "systems/npc/OptimizedAISystemBase.h"     // 优化的AI系统基类
#include "systems/npc/MonsterSpawnSystemEntt.h"

// NPC AI系统
#include "systems/npc/WarriorAISystemEntt.h"
#include "systems/npc/DemonEyeAISystemEntt.h"
#include "systems/npc/DemonAISystemEntt.h"
#include "systems/npc/EaterOfSoulsAISystemEntt.h"
#include "systems/npc/VultureAISystemEntt.h"
#include "systems/npc/AntlionAISystemEntt.h"
#include "systems/npc/BatAISystemEntt.h"
#include "systems/npc/AngryBonesAISystemEntt.h"
#include "systems/npc/KingSlimeAISystemEntt.h"

// 物品/投射物系统
#include "systems/item/ProjectileSystemEntt.h"
#include "systems/item/ProjectileAttackSystemEntt.h"
#include "systems/item/ProjectileCollisionSystemEntt.h"

// 系统管理器
#include "systems/core/SystemManagerEntt.h"

#endif // __ECS_ALL_SYSTEMS_H__
