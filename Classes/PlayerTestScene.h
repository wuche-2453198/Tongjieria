#ifndef __PLAYER_TEST_SCENE_H__
#define __PLAYER_TEST_SCENE_H__

#include "cocos2d.h"
#include "entt/entt.hpp"

/**
 * @class PlayerTestScene
 * @brief 玩家系统测试场景
 *
 * 功能：
 * - 创建玩家实体
 * - 测试玩家移动、跳跃
 * - 显示调试信息
 * - 测试物品栏集成
 */
class PlayerTestScene : public cocos2d::Layer {
public:
    static cocos2d::Scene* createScene();

    virtual bool init();

    virtual void update(float delta) override;

    void menuBackCallback(cocos2d::Ref* pSender);

    CREATE_FUNC(PlayerTestScene);

private:
    // EnTT 注册表
    entt::registry _registry;

    // 玩家实体
    entt::entity _playerEntity;

    // UI layer (fixed to screen)
    cocos2d::Node* _uiLayer = nullptr;

    // 调试UI
    cocos2d::Label* _debugLabel = nullptr;
    cocos2d::Label* _controlsLabel = nullptr;

    /**
     * @brief 创建物理环境（地面、平台、墙壁）
     */
    void createPhysicsEnvironment();

    /**
     * @brief 创建玩家
     */
    void createPlayer();

    /**
     * @brief 创建调试UI
     */
    void createDebugUI();

    /**
     * @brief 更新玩家逻辑
     */
    void updatePlayer(float dt);

    /**
     * @brief 更新玩家UI（血条、魔法条）
     */
    void updatePlayerUI(float dt);

    /**
     * @brief 更新调试信息
     */
    void updateDebugInfo(float dt);

    /**
     * @brief 检测玩家是否在地面上
     */
    void detectGround();
};

#endif // __PLAYER_TEST_SCENE_H__
