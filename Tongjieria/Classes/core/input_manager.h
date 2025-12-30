#pragma once
#include "cocos2d.h"
#include "entt/entt.hpp"

/**
 * @brief 鼠标基础事件，仅包含屏幕坐标和世界坐标
 */
struct MouseEvent {
    cocos2d::EventMouse::MouseButton button;   ///< 鼠标按键
    cocos2d::Vec2 worldPos;   ///< 世界坐标
    cocos2d::Vec2 screenPos;  ///< 屏幕坐标
};

class InputManager {
public:
    InputManager(entt::dispatcher& dispatcher);
    ~InputManager();
    // 初始化，注册事件监听
    void init(cocos2d::Node* listenNode);

    // 获取 dispatcher
    entt::dispatcher& getDispatcher();

private:
    void onMouseEvent(cocos2d::Event* event);

    entt::dispatcher& _dispatcher;
    cocos2d::EventListenerMouse* _mouseListener = nullptr;
};
