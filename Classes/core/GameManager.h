#ifndef __GAME_MANAGER_H__
#define __GAME_MANAGER_H__

#include "entt/entt.hpp"

/**
 * @file GameManager.h
 * @brief 游戏管理器单例
 *
 * 功能：
 * - 管理全局 EnTT registry
 * - 持有玩家实体引用
 * - 提供全局访问点给UI层和其他系统
 */
class GameManager {
public:
    static GameManager* getInstance();

    // 获取 EnTT 注册表
    entt::registry& getRegistry() { return _registry; }
    const entt::registry& getRegistry() const { return _registry; }

    // 玩家实体管理
    entt::entity getPlayerEntity() const { return _playerEntity; }
    void setPlayerEntity(entt::entity entity) { _playerEntity = entity; }
    bool hasPlayerEntity() const { return _playerEntity != entt::null; }

    // 清理资源
    void reset();

private:
    GameManager() = default;
    ~GameManager() = default;

    // 禁止拷贝和赋值
    GameManager(const GameManager&) = delete;
    GameManager& operator=(const GameManager&) = delete;

    static GameManager* _instance;

    entt::registry _registry;
    entt::entity _playerEntity = entt::null;
};

#endif // __GAME_MANAGER_H__
