#ifndef __PLAYER_SYSTEMS_H__
#define __PLAYER_SYSTEMS_H__

#include "cocos2d.h"
#include "entt/entt.hpp"
#include "components/player/PlayerComponents.h"

/**
 * @file PlayerSystems.h
 * @brief 玩家系统集合
 *
 * 包含所有玩家相关的 ECS 系统：
 * - PlayerInputSystem: 处理输入，更新组件
 * - PlayerMovementSystem: 处理移动逻辑和物理同步
 * - PlayerGroundDetectionSystem: 检测地面状态
 * - PlayerAnimationSystem: 更新动画状态
 * - PlayerHealthSystem: 处理生命回复和UI更新
 */

// ==================== PlayerInputSystem ====================
/**
 * @class PlayerInputSystem
 * @brief 处理玩家输入
 *
 * 职责：
 * - 读取 PlayerInput 单例的输入状态
 * - 更新 PlayerMovementComponent 的输入标志
 * - 处理快捷栏切换
 *
 * 优先级：0（最先执行）
 */
class PlayerInputSystem {
public:
    /**
     * @brief 更新输入系统
     * @param registry EnTT注册表
     * @param dt 帧时间
     */
    static void update(entt::registry& registry, float dt);

private:
    PlayerInputSystem() = delete;
};

// ==================== PlayerMovementSystem ====================
/**
 * @class PlayerMovementSystem
 * @brief 处理玩家移动逻辑
 *
 * 职责：
 * - 根据输入标志计算速度
 * - 应用加速度、摩擦力
 * - 处理跳跃
 * - 同步到物理引擎
 *
 * 优先级：10
 */
class PlayerMovementSystem {
public:
    /**
     * @brief 更新移动系统
     * @param registry EnTT注册表
     * @param dt 帧时间
     */
    static void update(entt::registry& registry, float dt);

private:
    PlayerMovementSystem() = delete;

    /**
     * @brief 处理水平移动
     */
    static void processHorizontalMovement(
        ecs::PlayerMovementComponent& movement,
        ecs::PlayerStatsComponent& stats,
        float dt
    );

    /**
     * @brief 处理跳跃
     */
    static void processJump(
        ecs::PlayerMovementComponent& movement,
        ecs::PlayerStatsComponent& stats,
        ecs::PlayerSpriteComponent& sprite
    );

    /**
     * @brief 同步到物理引擎
     */
    static void syncPhysics(
        ecs::PlayerMovementComponent& movement,
        ecs::TransformComponent& transform,
        ecs::PlayerSpriteComponent& sprite
    );

    /**
     * @brief 更新精灵方向
     */
    static void updateSpriteDirection(
        ecs::PlayerMovementComponent& movement,
        ecs::PlayerSpriteComponent& sprite
    );
};

// ==================== PlayerGroundDetectionSystem ====================
/**
 * @class PlayerGroundDetectionSystem
 * @brief 检测玩家是否在地面上
 *
 * 职责：
 * - 使用射线检测或物理碰撞检测地面
 * - 更新 PlayerStatsComponent::isOnGround
 * - 重置跳跃计数
 *
 * 优先级：5
 */
class PlayerGroundDetectionSystem {
public:
    /**
     * @brief 更新地面检测系统
     * @param registry EnTT注册表
     * @param dt 帧时间
     */
    static void update(entt::registry& registry, float dt);

private:
    PlayerGroundDetectionSystem() = delete;

    /**
     * @brief 使用射线检测地面
     * @param sprite 玩家精灵
     * @param scene 场景（用于物理世界查询）
     * @return 是否在地面上
     */
    static bool raycastGround(cocos2d::Sprite* sprite, cocos2d::Scene* scene);
};

// ==================== PlayerAnimationSystem ====================
/**
 * @class PlayerAnimationSystem
 * @brief 管理玩家动画
 *
 * 职责：
 * - 根据速度和状态判断动画类型（IDLE/WALK/JUMP/FALL）
 * - 播放对应动画
 * - 更新动画帧
 *
 * 优先级：100
 */
class PlayerAnimationSystem {
public:
    /**
     * @brief 更新动画系统
     * @param registry EnTT注册表
     * @param dt 帧时间
     */
    static void update(entt::registry& registry, float dt);

private:
    PlayerAnimationSystem() = delete;

    /**
     * @brief 确定当前应该播放的动画状态
     */
    static ecs::PlayerAnimationComponent::AnimState determineAnimationState(
        const ecs::PlayerMovementComponent& movement,
        const ecs::PlayerStatsComponent& stats,
        const ecs::PlayerSpriteComponent& sprite
    );

    /**
     * @brief 切换到新的动画状态
     */
    static void transitionToState(
        ecs::PlayerAnimationComponent& animation,
        ecs::PlayerAnimationComponent::AnimState newState
    );
};

// ==================== PlayerHealthSystem ====================
/**
 * @class PlayerHealthSystem
 * @brief 管理玩家生命和UI
 *
 * 职责：
 * - 处理生命回复
 * - 处理魔法回复
 * - 更新血条和魔法条UI
 * - 检查死亡
 *
 * 优先级：30
 */
class PlayerHealthSystem {
public:
    /**
     * @brief 更新生命系统
     * @param registry EnTT注册表
     * @param dt 帧时间
     */
    static void update(entt::registry& registry, float dt);

private:
    PlayerHealthSystem() = delete;

    /**
     * @brief 处理生命回复
     */
    static void processHealthRegen(
        ecs::PlayerStatsComponent& stats,
        float dt
    );

    /**
     * @brief 处理魔法回复
     */
    static void processManaRegen(
        ecs::PlayerStatsComponent& stats,
        float dt
    );

    /**
     * @brief 更新UI条
     */
    static void updateUI(
        const ecs::PlayerStatsComponent& stats,
        ecs::PlayerSpriteComponent& sprite
    );
};

// ==================== PlayerSystemsManager ====================
/**
 * @class PlayerSystemsManager
 * @brief 玩家系统管理器
 *
 * 提供便捷接口，按正确顺序更新所有玩家系统
 */
class PlayerSystemsManager {
public:
    /**
     * @brief 更新所有玩家系统
     * @param registry EnTT注册表
     * @param dt 帧时间
     */
    static void updateAllSystems(entt::registry& registry, float dt);

    /**
     * @brief 设置场景引用（用于射线检测等）
     */
    static void setScene(cocos2d::Scene* scene);

    /**
     * @brief 获取场景引用
     */
    static cocos2d::Scene* getScene();

private:
    PlayerSystemsManager() = delete;

    static cocos2d::Scene* s_scene;
};

#endif // __PLAYER_SYSTEMS_H__
