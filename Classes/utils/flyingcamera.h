#pragma once

#include "cocos2d.h"

class FlyCamera2D : public cocos2d::Node
{
public:
    static FlyCamera2D* createWithTarget(cocos2d::Node* target);
    bool initWithTarget(cocos2d::Node* target);

    void update(float dt) override;
    void onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);
    void onKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);

    void setSpeed(float speed) { _speed = speed; }
    float getSpeed() const { return _speed; }

    void setActive(bool active);
    bool isActive() const { return _active; }


    cocos2d::Node* _target = nullptr;    // 要控制的摄像机节点
    float _speed = 10.0f;              // 移动速度（像素/秒）
    cocos2d::Vec2 _velocity;             // 当前速度向量
    bool _active = true;                 // 是否激活控制

    // 按键状态
    bool _keyW = false;
    bool _keyA = false;
    bool _keyS = false;
    bool _keyD = false;
};

USING_NS_CC;

FlyCamera2D* FlyCamera2D::createWithTarget(cocos2d::Node* target)
{
    FlyCamera2D* camera = new (std::nothrow) FlyCamera2D();
    if (camera && camera->initWithTarget(target))
    {
        camera->autorelease();
        return camera;
    }
    CC_SAFE_DELETE(camera);
    return nullptr;
}

bool FlyCamera2D::initWithTarget(cocos2d::Node* target)
{
    if (!Node::init())
        return false;

    _target = target;

    // 设置更新调度
    this->scheduleUpdate();

    // 注册键盘事件监听器
    auto listener = EventListenerKeyboard::create();
    listener->onKeyPressed = CC_CALLBACK_2(FlyCamera2D::onKeyPressed, this);
    listener->onKeyReleased = CC_CALLBACK_2(FlyCamera2D::onKeyReleased, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    return true;
}

void FlyCamera2D::update(float dt)
{
    if (!_active || !_target) return;

    // 根据按键状态计算速度向量
    _velocity = Vec2::ZERO;

    if (_keyW) _velocity.y += 1.0f;
    if (_keyS) _velocity.y -= 1.0f;
    if (_keyA) _velocity.x -= 1.0f;
    if (_keyD) _velocity.x += 1.0f;

    // 标准化速度向量（保持对角线移动速度一致）
    if (_velocity.lengthSquared() > 0)
    {
        _velocity.normalize();
        _velocity *= _speed * dt;

        // 移动摄像机
        Vec2 newPosition = _target->getPosition() + _velocity;
        _target->setPosition(newPosition);
    }
}

void FlyCamera2D::onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event)
{
    if (!_active) return;

    switch (keyCode)
    {
        case EventKeyboard::KeyCode::KEY_W:
            _keyW = true;
            break;
        case EventKeyboard::KeyCode::KEY_A:
            _keyA = true;
            break;
        case EventKeyboard::KeyCode::KEY_S:
            _keyS = true;
            break;
        case EventKeyboard::KeyCode::KEY_D:
            _keyD = true;
            break;
        default:
            break;
    }
}

void FlyCamera2D::onKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event)
{
    switch (keyCode)
    {
        case EventKeyboard::KeyCode::KEY_W:
            _keyW = false;
            break;
        case EventKeyboard::KeyCode::KEY_A:
            _keyA = false;
            break;
        case EventKeyboard::KeyCode::KEY_S:
            _keyS = false;
            break;
        case EventKeyboard::KeyCode::KEY_D:
            _keyD = false;
            break;
        default:
            break;
    }
}

void FlyCamera2D::setActive(bool active)
{
    _active = active;

    // 激活时重置按键状态
    if (active)
    {
        _keyW = _keyA = _keyS = _keyD = false;
        _velocity = Vec2::ZERO;
    }
}