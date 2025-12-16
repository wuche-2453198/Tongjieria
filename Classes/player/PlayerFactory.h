#ifndef __PLAYER_FACTORY_H__
#define __PLAYER_FACTORY_H__

#include "cocos2d.h"
#include "entt/entt.hpp"
#include "PlayerComponents.h"

/**
 * @class PlayerFactory
 * @brief 玩家实体工厂（基于 EnTT）
 *
 * 职责：
 * - 创建完整的玩家实体（包含所有必要组件）
 * - 创建玩家精灵和物理体
 * - 加载玩家配置
 */
class PlayerFactory {
public:
    /**
     * @brief 创建玩家实体
     * @param registry EnTT 注册表
     * @param spawnPos 生成位置
     * @param parentNode 父节点（用于添加精灵）
     * @return 玩家实体
     */
    static entt::entity createPlayer(entt::registry& registry,
                                     const cocos2d::Vec2& spawnPos,
                                     cocos2d::Node* parentNode);

    /**
     * @brief 创建玩家精灵
     * @param spawnPos 生成位置
     * @param parentNode 父节点
     * @return 玩家精灵指针
     */
    static cocos2d::Sprite* createPlayerSprite(const cocos2d::Vec2& spawnPos,
                                               cocos2d::Node* parentNode);

    /**
     * @brief 为玩家精灵添加物理体
     * @param sprite 精灵指针
     */
    static void addPhysicsBody(cocos2d::Sprite* sprite);

    /**
     * @brief 加载玩家配置（从存档/配置文件）
     * @param stats 玩家属性组件
     */
    static void loadPlayerStats(ecs::PlayerStatsComponent& stats);

    /**
     * @brief 创建玩家UI（血条、魔法条等）
     * @param entity 玩家实体
     * @param registry EnTT 注册表
     * @param parentNode 父节点
     */
    static void createPlayerUI(entt::entity entity,
                               entt::registry& registry,
                               cocos2d::Node* parentNode);

private:
    PlayerFactory() = delete;
    ~PlayerFactory() = delete;
};

#endif // __PLAYER_FACTORY_H__
