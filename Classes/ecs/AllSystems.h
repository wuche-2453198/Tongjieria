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
#include "systems/ISystemEntt.h"
#include "systems/SystemPriority.h"

// 核心系统
#include "systems/HealthSystemEntt.h"
#include "systems/CombatSystemEntt.h"
#include "systems/LifetimeSystemEntt.h"

// 渲染和动画系统
#include "systems/MonsterAnimationSystemEntt.h"
#include "systems/SlimeRenderSystemEntt.h"
#include "systems/SlimeSyncSystemEntt.h"
#include "systems/MonsterSyncSystemEntt.h"

// 物理系统
#include "systems/GroundDetectorSystemEntt.h"
#include "systems/MonsterGroundDetectorSystemEntt.h"
#include "systems/SlowFallSystemEntt.h"

// AI系统
#include "systems/AggroSystemEntt.h"
#include "systems/JumpMovementSystemEntt.h"
#include "systems/WarriorAISystemEntt.h"
#include "systems/DemonEyeAISystemEntt.h"
#include "systems/EaterOfSoulsAISystemEntt.h"
#include "systems/AntlionAISystemEntt.h"
#include "systems/KingSlimeAISystemEntt.h"

// 投射物系统
#include "systems/ProjectileSystemEntt.h"
#include "systems/ProjectileAttackSystemEntt.h"
#include "systems/ProjectileCollisionSystemEntt.h"

// 效果系统
#include "systems/DebuffSystemEntt.h"

// 系统管理器
#include "systems/SystemManagerEntt.h"

// ==================== 新解耦渲染架构系统 ====================
// 这些系统配合新的纯数据渲染组件使用
#include "systems/RenderSystem.h"

#endif // __ECS_ALL_SYSTEMS_H__
