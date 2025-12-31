#ifndef __PLAYER_INPUT_H__
#define __PLAYER_INPUT_H__

#include "cocos2d.h"
#include "entt/entt.hpp"
#include <map>

/**
 * @class PlayerInput
 * @brief 玩家输入管理器（单例）
 *
 * 职责：
 * - 监听键盘/鼠标/手柄输入
 * - 提供统一的输入查询接口
 * - 支持输入映射配置
 */
class PlayerInput {
public:
    static PlayerInput& getInstance() {
        static PlayerInput instance;
        return instance;
    }

    // ==================== 初始化与清理 ====================

    /**
     * @brief 初始化输入监听器
     * @param scene 场景节点
     */
    void initialize(cocos2d::Scene* scene);

    /**
     * @brief 清理监听器
     */
    void cleanup();

    /**
     * @brief 每帧更新（用于处理JustPressed状态）
     */
    void update(float dt);

    /**
     * @brief 设置registry指针用于处理物品丢弃
     */
    void setRegistry(entt::registry* registry);

    // ==================== 键盘输入查询 ====================

    /**
     * @brief 检查按键是否按下（持续）
     */
    bool isKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode) const;

    /**
     * @brief 检查按键是否刚按下（单帧）
     */
    bool isKeyJustPressed(cocos2d::EventKeyboard::KeyCode keyCode) const;

    /**
     * @brief 检查按键是否刚释放（单帧）
     */
    bool isKeyJustReleased(cocos2d::EventKeyboard::KeyCode keyCode) const;

    // ==================== 鼠标输入查询 ====================

    /**
     * @brief 检查鼠标按键是否按下
     */
    bool isMousePressed(cocos2d::EventMouse::MouseButton button = cocos2d::EventMouse::MouseButton::BUTTON_LEFT) const;

    /**
     * @brief 检查鼠标按键是否刚按下
     */
    bool isMouseJustPressed(cocos2d::EventMouse::MouseButton button = cocos2d::EventMouse::MouseButton::BUTTON_LEFT) const;

    /**
     * @brief 获取鼠标世界坐标
     */
    cocos2d::Vec2 getMouseWorldPosition() const;

    /**
     * @brief 获取鼠标屏幕坐标
     */
    cocos2d::Vec2 getMouseScreenPosition() const;

    // ==================== 输入映射 ====================

    /**
     * @brief 设置按键映射
     */
    void setKeyMapping(const std::string& action, cocos2d::EventKeyboard::KeyCode keyCode);

    /**
     * @brief 检查动作是否触发
     */
    bool isActionPressed(const std::string& action) const;
    bool isActionJustPressed(const std::string& action) const;

private:
    PlayerInput();
    ~PlayerInput();
    PlayerInput(const PlayerInput&) = delete;
    PlayerInput& operator=(const PlayerInput&) = delete;

    // ==================== 内部状态 ====================

    struct KeyState {
        bool isPressed = false;
        bool justPressed = false;
        bool justReleased = false;
    };

    struct MouseState {
        bool isPressed = false;
        bool justPressed = false;
        bool justReleased = false;
    };

    std::map<cocos2d::EventKeyboard::KeyCode, KeyState> _keyStates;
    std::map<cocos2d::EventMouse::MouseButton, MouseState> _mouseStates;

    cocos2d::Vec2 _mousePosition = {0, 0};

    cocos2d::EventListenerKeyboard* _keyboardListener = nullptr;
    cocos2d::EventListenerMouse* _mouseListener = nullptr;
    cocos2d::EventListenerCustom* _itemDropListener = nullptr;

    // 按键映射
    std::map<std::string, cocos2d::EventKeyboard::KeyCode> _keyMappings;

    // Registry pointer for item drop handling
    entt::registry* _registry = nullptr;

    // ==================== 回调函数 ====================

    void onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);
    void onKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);

    void onMouseDown(cocos2d::Event* event);
    void onMouseUp(cocos2d::Event* event);
    void onMouseMove(cocos2d::Event* event);

    // ==================== 辅助函数 ====================

    void resetJustStates();
};

#endif // __PLAYER_INPUT_H__
