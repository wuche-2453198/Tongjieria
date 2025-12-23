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

// 战斗系统
#include "systems/combat/HealthSystemEntt.h"
#include "systems/combat/CombatSystemEntt.h"
#include "systems/combat/DebuffSystemEntt.h"

// 渲染系统
#include "systems/render/RenderSystem.h"
#include "systems/render/SpriteManager.h"
#include "systems/render/SlimeRenderSystemEntt.h"
#include "systems/render/SlimeSyncSystemEntt.h"

// 物理系统
#include "systems/physics/PhysicsContactHandler.h"
#include "systems/physics/GroundDetectorSystemEntt.h"
#include "systems/physics/SlowFallSystemEntt.h"
#include "systems/physics/JumpMovementSystemEntt.h"

// NPC系统
#include "systems/npc/AggroSystemEntt.h"
#include "systems/npc/MonsterAnimationSystemEntt.h"
#include "systems/npc/MonsterGroundDetectorSystemEntt.h"
#include "systems/npc/MonsterSyncSystemEntt.h"

// NPC AI系统
#include "systems/npc/WarriorAISystemEntt.h"
#include "systems/npc/DemonEyeAISystemEntt.h"
#include "systems/npc/EaterOfSoulsAISystemEntt.h"
#include "systems/npc/AntlionAISystemEntt.h"
#include "systems/npc/KingSlimeAISystemEntt.h"

// 物品/投射物系统
#include "systems/item/ProjectileSystemEntt.h"
#include "systems/item/ProjectileAttackSystemEntt.h"
#include "systems/item/ProjectileCollisionSystemEntt.h"

// 系统管理器
#include "systems/core/SystemManagerEntt.h"

#endif // __ECS_ALL_SYSTEMS_H__
