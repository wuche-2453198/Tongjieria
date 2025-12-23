#ifndef __KING_SLIME_TEST_SCENE_H__
#define __KING_SLIME_TEST_SCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "components/AllComponents.h"
#include "systems/AllSystems.h"
#include "components/render/SpriteComponent.h"
#include <unordered_map>

USING_NS_CC;
using namespace cocos2d::ui;

class KingSlimeTestScene : public cocos2d::Layer
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init();
    CREATE_FUNC(KingSlimeTestScene);

private:
    // ECS系统管理
    ecs::SystemManagerEntt _systemManager;
    entt::registry _registry;
    ecs::ProjectileCollisionSystemEntt* _projectileCollisionSystem;

    // 假玩家相关
    Sprite* _fakePlayer;
    Label* _playerLabel;
    ecs::EntityId _fakePlayerEntity;
    bool _fakePlayerVisible;
    
    // 玩家物理和移动
    bool _playerOnGround;
    int _playerGroundContactCount;
    float _playerMoveSpeed;
    float _playerJumpForce;
    
    // 伤害按钮
    Button* _damageButton;
    
    // 史莱姆王Boss相关
    ecs::EntityId _kingSlimeBossEntity;
    Label* _bossNameLabel;
    Sprite* _bossHealthBarBg;
    Sprite* _bossHealthBarFg;
    
    // 输入处理
    std::unordered_map<EventKeyboard::KeyCode, bool> _keysPressed;
    EventListenerKeyboard* _keyboardListener;
    
    // 碰撞监听
    EventListenerPhysicsContact* _sharedContactListener;
    
    // 摄像机跟随
    void updateCameraFollow();

    // 初始化方法
    void createPhysicsEnvironment();
    void setupEcsSystems();
    void setupSharedContactListener();
    void createFakePlayerEntity();
    void setupInputListeners();
    
    // Boss相关方法
    void spawnKingSlimeBoss();
    void createBossHealthBar();
    void updateBossHealthBar();
    
    // 更新方法
    void update(float delta) override;
    void updateFakePlayerMovement(float delta);
    void createDamageButton();
    void onDamageButtonClicked(Ref* sender);
    
    // 回调方法
    void menuBackCallback(Ref* pSender);
    void onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event);
    void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event);
};

#endif // __KING_SLIME_TEST_SCENE_H__
