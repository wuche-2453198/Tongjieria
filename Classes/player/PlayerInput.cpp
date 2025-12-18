#include "PlayerInput.h"

USING_NS_CC;

PlayerInput::PlayerInput() {
    // 初始化默认按键映射
    setKeyMapping("MoveLeft", EventKeyboard::KeyCode::KEY_A);
    setKeyMapping("MoveRight", EventKeyboard::KeyCode::KEY_D);
    setKeyMapping("Jump", EventKeyboard::KeyCode::KEY_SPACE);
    setKeyMapping("UseItem", EventKeyboard::KeyCode::KEY_LEFT_CTRL);
    setKeyMapping("Hook", EventKeyboard::KeyCode::KEY_E);
    setKeyMapping("QuickHeal", EventKeyboard::KeyCode::KEY_H);
    setKeyMapping("QuickMana", EventKeyboard::KeyCode::KEY_M);
    setKeyMapping("Mount", EventKeyboard::KeyCode::KEY_R);
    setKeyMapping("Inventory", EventKeyboard::KeyCode::KEY_ESCAPE);
}

PlayerInput::~PlayerInput() {
    cleanup();
}

void PlayerInput::initialize(Scene* scene) {
    if (!scene) {
        CCLOG("PlayerInput::initialize - scene is null");
        return;
    }

    cleanup(); // 清理旧的监听器

    // 创建键盘监听器
    _keyboardListener = EventListenerKeyboard::create();
    _keyboardListener->onKeyPressed = CC_CALLBACK_2(PlayerInput::onKeyPressed, this);
    _keyboardListener->onKeyReleased = CC_CALLBACK_2(PlayerInput::onKeyReleased, this);
    scene->getEventDispatcher()->addEventListenerWithSceneGraphPriority(_keyboardListener, scene);

    // 创建鼠标监听器
    _mouseListener = EventListenerMouse::create();
    _mouseListener->onMouseDown = CC_CALLBACK_1(PlayerInput::onMouseDown, this);
    _mouseListener->onMouseUp = CC_CALLBACK_1(PlayerInput::onMouseUp, this);
    _mouseListener->onMouseMove = CC_CALLBACK_1(PlayerInput::onMouseMove, this);
    scene->getEventDispatcher()->addEventListenerWithSceneGraphPriority(_mouseListener, scene);

    CCLOG("PlayerInput initialized");
}

void PlayerInput::cleanup() {
    if (_keyboardListener) {
        Director::getInstance()->getEventDispatcher()->removeEventListener(_keyboardListener);
        _keyboardListener = nullptr;
    }

    if (_mouseListener) {
        Director::getInstance()->getEventDispatcher()->removeEventListener(_mouseListener);
        _mouseListener = nullptr;
    }

    _keyStates.clear();
    _mouseStates.clear();
}

void PlayerInput::update(float dt) {
    // 重置单帧输入状态
    resetJustStates();
}

// ==================== 键盘输入查询 ====================

bool PlayerInput::isKeyPressed(EventKeyboard::KeyCode keyCode) const {
    auto it = _keyStates.find(keyCode);
    return (it != _keyStates.end()) && it->second.isPressed;
}

bool PlayerInput::isKeyJustPressed(EventKeyboard::KeyCode keyCode) const {
    auto it = _keyStates.find(keyCode);
    return (it != _keyStates.end()) && it->second.justPressed;
}

bool PlayerInput::isKeyJustReleased(EventKeyboard::KeyCode keyCode) const {
    auto it = _keyStates.find(keyCode);
    return (it != _keyStates.end()) && it->second.justReleased;
}

// ==================== 鼠标输入查询 ====================

bool PlayerInput::isMousePressed(EventMouse::MouseButton button) const {
    auto it = _mouseStates.find(button);
    return (it != _mouseStates.end()) && it->second.isPressed;
}

bool PlayerInput::isMouseJustPressed(EventMouse::MouseButton button) const {
    auto it = _mouseStates.find(button);
    return (it != _mouseStates.end()) && it->second.justPressed;
}

Vec2 PlayerInput::getMouseWorldPosition() const {
    return _mousePosition;
}

Vec2 PlayerInput::getMouseScreenPosition() const {
    // 将世界坐标转换为屏幕坐标
    auto director = Director::getInstance();
    auto glView = director->getOpenGLView();
    auto frameSize = glView->getFrameSize();
    auto visibleSize = director->getVisibleSize();
    auto origin = director->getVisibleOrigin();

    // 简单转换（假设没有缩放）
    return Vec2(
        (_mousePosition.x - origin.x) * (frameSize.width / visibleSize.width),
        (_mousePosition.y - origin.y) * (frameSize.height / visibleSize.height)
    );
}

// ==================== 输入映射 ====================

void PlayerInput::setKeyMapping(const std::string& action, EventKeyboard::KeyCode keyCode) {
    _keyMappings[action] = keyCode;
}

bool PlayerInput::isActionPressed(const std::string& action) const {
    auto it = _keyMappings.find(action);
    if (it != _keyMappings.end()) {
        return isKeyPressed(it->second);
    }
    return false;
}

bool PlayerInput::isActionJustPressed(const std::string& action) const {
    auto it = _keyMappings.find(action);
    if (it != _keyMappings.end()) {
        return isKeyJustPressed(it->second);
    }
    return false;
}

// ==================== 回调函数 ====================

void PlayerInput::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event) {
    auto& state = _keyStates[keyCode];
    if (!state.isPressed) {
        state.justPressed = true;
    }
    state.isPressed = true;
    state.justReleased = false;

    // 调试：输出空格键按下事件
    if (keyCode == EventKeyboard::KeyCode::KEY_SPACE) {
        CCLOG("!!! PlayerInput: SPACE key pressed event received !!!");
    }
}

void PlayerInput::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event) {
    auto& state = _keyStates[keyCode];
    state.isPressed = false;
    state.justPressed = false;
    state.justReleased = true;
}

void PlayerInput::onMouseDown(Event* event) {
    EventMouse* mouseEvent = static_cast<EventMouse*>(event);
    auto button = mouseEvent->getMouseButton();

    auto& state = _mouseStates[button];
    if (!state.isPressed) {
        state.justPressed = true;
    }
    state.isPressed = true;
    state.justReleased = false;

    // 更新鼠标位置
    _mousePosition.x = mouseEvent->getCursorX();
    _mousePosition.y = mouseEvent->getCursorY();

    // 转换为OpenGL坐标（左下角为原点）
    auto director = Director::getInstance();
    auto glView = director->getOpenGLView();
    auto frameSize = glView->getFrameSize();
    _mousePosition.y = frameSize.height - _mousePosition.y;
}

void PlayerInput::onMouseUp(Event* event) {
    EventMouse* mouseEvent = static_cast<EventMouse*>(event);
    auto button = mouseEvent->getMouseButton();

    auto& state = _mouseStates[button];
    state.isPressed = false;
    state.justPressed = false;
    state.justReleased = true;
}

void PlayerInput::onMouseMove(Event* event) {
    EventMouse* mouseEvent = static_cast<EventMouse*>(event);

    _mousePosition.x = mouseEvent->getCursorX();
    _mousePosition.y = mouseEvent->getCursorY();

    // 转换为OpenGL坐标
    auto director = Director::getInstance();
    auto glView = director->getOpenGLView();
    auto frameSize = glView->getFrameSize();
    _mousePosition.y = frameSize.height - _mousePosition.y;
}

// ==================== 辅助函数 ====================

void PlayerInput::resetJustStates() {
    // 重置所有"刚按下/刚释放"状态
    for (auto& pair : _keyStates) {
        pair.second.justPressed = false;
        pair.second.justReleased = false;
    }

    for (auto& pair : _mouseStates) {
        pair.second.justPressed = false;
        pair.second.justReleased = false;
    }
}
