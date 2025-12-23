#include "input_manager.h"
#include "cocos2d.h"

InputManager::InputManager(entt::dispatcher& dispatcher)
    : _dispatcher(dispatcher) {}
InputManager::~InputManager() {
    if (_mouseListener) {
        cocos2d::Director::getInstance()->
            getEventDispatcher()->removeEventListener(_mouseListener);
    }
}

void InputManager::init(cocos2d::Node* listenNode) {
    if (_mouseListener) return; // 避免重复注册

    _mouseListener = cocos2d::EventListenerMouse::create();
    _mouseListener->onMouseDown =   CC_CALLBACK_1(InputManager::onMouseEvent, this);
    _mouseListener->onMouseUp =     CC_CALLBACK_1(InputManager::onMouseEvent, this);
    _mouseListener->onMouseMove =   CC_CALLBACK_1(InputManager::onMouseEvent, this);
    _mouseListener->onMouseScroll = CC_CALLBACK_1(InputManager::onMouseEvent, this);

    cocos2d::Director::getInstance()->
        getEventDispatcher()->addEventListenerWithSceneGraphPriority(_mouseListener, listenNode);
}

entt::dispatcher& InputManager::getDispatcher() {
    return _dispatcher;
}

void InputManager::onMouseEvent(cocos2d::Event* event) {
    auto mouseEvent = dynamic_cast<cocos2d::EventMouse*>(event);
    if (!mouseEvent) return;
    
    MouseEvent mevent;
    mevent.button = mouseEvent->getMouseButton();
    mevent.screenPos = cocos2d::Vec2(mouseEvent->getCursorX(), mouseEvent->getCursorY());
    // 计算世界坐标
    auto camera = cocos2d::Camera::getVisitingCamera();
    if (!camera) camera = cocos2d::Camera::getDefaultCamera();
    auto& size = cocos2d::Director::getInstance()->getWinSize();
    cocos2d::Vec3 screenPos3D(mevent.screenPos.x, mevent.screenPos.y, 0.9935f);
    cocos2d::Vec3 worldPos3D;
    camera->unprojectGL(size, &screenPos3D, &worldPos3D);
    mevent.worldPos = cocos2d::Vec2(worldPos3D.x, worldPos3D.y);

    _dispatcher.trigger(mevent);
}
