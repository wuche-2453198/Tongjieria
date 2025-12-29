#ifndef __PLAYER_SYSTEMS_H__
#define __PLAYER_SYSTEMS_H__

#include "cocos2d.h"
#include "entt/entt.hpp"
#include "components/player/PlayerComponents.h"

// Forward declaration for block system components
class Position;

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
        ecs::PlayerSpriteComponent& sprite,
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
     * @brief 同步到物理引擎和方块系统
     */
    static void syncPhysics(
        entt::registry& registry,
        entt::entity entity,
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

    /**
     * @brief 使用射线检测地面
     * @param sprite 玩家精灵
     * @param scene 场景（用于物理世界查询）
     * @return 是否在地面上
     */
    static bool raycastGround(cocos2d::Sprite* sprite, cocos2d::Scene* scene);

    /**
     * @brief 使用射线检测左侧墙体
     * @param sprite 玩家精灵
     * @param scene 场景（用于物理世界查询）
     * @param checkDistance 检测距离（像素）
     * @return 是否碰到左侧墙体
     */
    static bool raycastLeftWall(cocos2d::Sprite* sprite, cocos2d::Scene* scene, float checkDistance = 2.0f);

    /**
     * @brief 使用射线检测右侧墙体
     * @param sprite 玩家精灵
     * @param scene 场景（用于物理世界查询）
     * @param checkDistance 检测距离（像素）
     * @return 是否碰到右侧墙体
     */
    static bool raycastRightWall(cocos2d::Sprite* sprite, cocos2d::Scene* scene, float checkDistance = 2.0f);

private:
    PlayerGroundDetectionSystem() = delete;
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

    /**
     * @brief 切换到新的动画状态（公有方法，供其他系统调用）
     */
    static void transitionToState(
        ecs::PlayerAnimationComponent& animation,
        ecs::PlayerAnimationComponent::AnimState newState
    );

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
     * @brief 隐藏所有缓存的动画帧，防止重影
     */
    static void hideAllCachedFrames();
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

// ==================== PlayerCameraSystem ====================
/**
 * @class PlayerCameraSystem
 * @brief 摄像机跟随系统
 *
 * 职责：
 * - 让摄像机平滑跟随玩家位置
 * - 支持可配置的跟随速度和死区
 *
 * 优先级：200（最后执行，确保位置已更新）
 */
class PlayerCameraSystem {
public:
    /**
     * @brief 更新摄像机系统
     * @param registry EnTT注册表
     * @param dt 帧时间
     */
    static void update(entt::registry& registry, float dt);

    /**
     * @brief 设置摄像机跟随速度
     * @param speed 跟随速度 (0-1, 1表示立即跟随)
     */
    static void setFollowSpeed(float speed);

    /**
     * @brief 获取当前跟随速度
     */
    static float getFollowSpeed();

private:
    PlayerCameraSystem() = delete;

    static float s_followSpeed;  // 摄像机跟随速度
};

// ==================== PlayerCraftingSystem ====================
// Forward declaration
class PlayerCraftingSystem;

// ==================== PlayerEquipmentSyncSystem ====================
/**
 * @class PlayerEquipmentSyncSystem
 * @brief 装备同步系统
 *
 * 职责：
 * - 监听 Event_EquipmentChanged 事件
 * - 将 Inventory 的装备槽同步到 PlayerEquipmentComponent
 * - 自动触发装备属性重新计算
 *
 * 优先级：N/A（事件驱动）
 */
class PlayerEquipmentSyncSystem {
public:
    /**
     * @brief 初始化装备同步系统（注册事件监听器）
     * @param registry EnTT注册表
     */
    static void initialize(entt::registry& registry);

    /**
     * @brief 清理装备同步系统（移除事件监听器）
     */
    static void shutdown();

    /**
     * @brief 同步装备槽数据：Inventory -> PlayerEquipmentComponent
     * @param registry EnTT注册表
     * @param playerEntity 玩家实体
     */
    static void syncEquipmentToPlayer(entt::registry& registry, entt::entity playerEntity);

private:
    PlayerEquipmentSyncSystem() = delete;

    static cocos2d::EventListenerCustom* s_equipmentListener;
    static entt::registry* s_registry;
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
