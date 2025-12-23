#ifndef __ECS_ALL_COMPONENTS_H__
#define __ECS_ALL_COMPONENTS_H__

/**
 * @file AllComponents.h
 * @brief 聚合所有从components/目录拆分出来的独立组件
 * 
 * 使用方法：
 * #include "ecs/AllComponents.h"
 */

// ==================== 核心组件 ====================
#include "components/TransformComponent.h"
#include "components/HealthComponent.h"
#include "components/PlayerTag.h"
#include "components/EnemyTag.h"

// ==================== 战斗组件 ====================
#include "components/CombatComponent.h"
#include "components/AggroComponent.h"
#include "components/DebuffComponent.h"
#include "components/LootComponent.h"
#include "components/DeathSpawnComponent.h"

// ==================== 生命周期组件 ====================
#include "components/LifetimeComponent.h"

// ==================== 移动和物理组件 ====================
#include "components/GroundDetectorComponent.h"
#include "components/JumpMovementComponent.h"
#include "components/WarriorMovementComponent.h"
#include "components/DemonEyeMovementComponent.h"
#include "components/EaterOfSoulsMovementComponent.h"
#include "components/AntlionMovementComponent.h"
#include "components/SlowFallComponent.h"
#include "components/PhysicsBodyComponent.h"
#include "components/InitialVelocityComponent.h"

// ==================== 投射物组件 ====================
#include "components/ProjectileComponent.h"
#include "components/ProjectileAttackComponent.h"

// ==================== Boss组件 ====================
#include "components/KingSlimeComponent.h"

// ==================== 新解耦渲染架构组件 ====================
#include "components/RenderComponent.h"
#include "components/AnimationComponent.h"
#include "components/AnimationStateComponent.h"
#include "components/SpriteResourceDescriptor.h"
#include "components/ParentNodeComponent.h"
#include "components/SpriteStateComponent.h"
#include "components/SpriteComponent.h"

// ==================== 精灵组件（旧架构，包含Cocos2d对象） ====================
#include "components/SlimeSpriteComponent.h"
#include "components/MonsterSpriteComponent.h"
#include "components/ProjectileSpriteComponent.h"


#endif // __ECS_ALL_COMPONENTS_H__
