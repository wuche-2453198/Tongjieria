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
#include "components/core/TransformComponent.h"
#include "components/core/LifetimeComponent.h"
#include "components/core/EntityStateFlags.h"
#include "components/core/PooledEntity.h"

// ==================== 玩家组件 ====================
#include "components/player/PlayerTag.h"

// ==================== 战斗组件 ====================
#include "components/combat/CombatComponent.h"
#include "components/combat/HealthComponent.h"
#include "components/combat/DebuffComponent.h"

// ==================== 物理组件 ====================
#include "components/physics/PhysicsBodyComponent.h"
#include "components/physics/GroundDetectorComponent.h"
#include "components/physics/InitialVelocityComponent.h"
#include "components/physics/JumpMovementComponent.h"
#include "components/physics/SlowFallComponent.h"

// ==================== 渲染组件 ====================
#include "components/render/RenderComponent.h"
#include "components/render/AnimationComponent.h"
#include "components/render/SpriteComponent.h"
#include "components/render/SpriteResourceDescriptor.h"
#include "components/render/SpriteStateComponent.h"
#include "components/render/ParentNodeComponent.h"
#include "components/render/SlimeSpriteComponent.h"
#include "components/render/SharedAnimationComponent.h"

// ==================== NPC组件 ====================
#include "components/npc/EnemyTag.h"
#include "components/npc/AggroComponent.h"
#include "components/npc/LootComponent.h"
#include "components/npc/DeathSpawnComponent.h"
#include "components/npc/AnimationStateComponent.h"
#include "components/npc/MonsterSpriteComponent.h"
#include "components/npc/WarriorMovementComponent.h"
#include "components/npc/DemonEyeMovementComponent.h"
#include "components/npc/BatMovementComponent.h"
#include "components/npc/PounceAttackComponent.h"
#include "components/npc/VultureMovementComponent.h"
#include "components/npc/EaterOfSoulsMovementComponent.h"
#include "components/npc/AntlionMovementComponent.h"
#include "components/npc/DemonMovementComponent.h"
#include "components/npc/DemonAttackComponent.h"
#include "components/npc/KingSlimeComponent.h"

// ==================== 物品/投射物组件 ====================
#include "components/item/ProjectileComponent.h"
#include "components/item/ProjectileAttackComponent.h"
#include "components/item/ProjectileSpriteComponent.h"
#include "components/item/DelayedAccelerationComponent.h"
#include "components/item/NoVelocityRotationTag.h"


#endif // __ECS_ALL_COMPONENTS_H__
