#ifndef __ECS_SPRITE_MANAGER_H__
#define __ECS_SPRITE_MANAGER_H__

#include "../AllComponents.h"
#include "cocos2d.h"
#include <unordered_map>
#include <string>
#include <memory>

namespace ecs {

/**
 * @brief 精灵管理器 - 管理所有Sprite的生命周期和资源
 * 
 * 职责：
 * - 创建和销毁Sprite
 * - 缓存精灵资源描述符
 * - 提供统一的精灵访问接口
 * - 管理精灵帧缓存
 * 
 * 设计模式：单例
 */
class SpriteManager {
public:
    static SpriteManager& getInstance() {
        static SpriteManager instance;
        return instance;
    }
    
    /**
     * @brief 注册精灵资源描述符
     * @param descriptor 资源描述符
     */
    void registerResource(const SpriteResourceDescriptor& descriptor);
    
    /**
     * @brief 创建精灵
     * @param resourceId 资源ID
     * @return 精灵指针，失败返回nullptr
     */
    cocos2d::Sprite* createSprite(const std::string& resourceId);
    
    /**
     * @brief 创建精灵并添加到父节点
     * @param resourceId 资源ID
     * @param parent 父节点
     * @param zOrder 层级
     * @return 精灵指针
     */
    cocos2d::Sprite* createSpriteAndAttach(const std::string& resourceId,
                                           cocos2d::Node* parent,
                                           int zOrder = 0);
    
    /**
     * @brief 获取动画帧
     * @param resourceId 资源ID
     * @return 动画帧数组
     */
    const cocos2d::Vector<cocos2d::SpriteFrame*>& getAnimationFrames(const std::string& resourceId);
    
    /**
     * @brief 预加载资源（加载所有帧到缓存）
     * @param resourceId 资源ID
     * @return 是否成功
     */
    bool preloadResource(const std::string& resourceId);
    
    /**
     * @brief 释放精灵
     * @param sprite 精灵指针
     */
    void releaseSprite(cocos2d::Sprite* sprite);
    
    /**
     * @brief 清空所有资源
     */
    void clearAll();
    
    /**
     * @brief 检查资源是否已注册
     */
    bool hasResource(const std::string& resourceId) const;
    
    /**
     * @brief 获取资源描述符
     */
    const SpriteResourceDescriptor* getResourceDescriptor(const std::string& resourceId) const;
    
private:
    SpriteManager() = default;
    ~SpriteManager() { clearAll(); }
    SpriteManager(const SpriteManager&) = delete;
    SpriteManager& operator=(const SpriteManager&) = delete;
    
    // 加载动画帧（懒加载）
    void loadAnimationFrames(const std::string& resourceId);
    
    // 资源描述符映射
    std::unordered_map<std::string, SpriteResourceDescriptor> _resourceDescriptors;
    
    // 动画帧缓存（resourceId -> frames）
    std::unordered_map<std::string, cocos2d::Vector<cocos2d::SpriteFrame*>> _animationFrameCache;
    
    // 精灵引用计数（用于管理释放）
    std::unordered_map<cocos2d::Sprite*, int> _spriteRefCount;
};

/**
 * @brief 精灵资源构建器 - 便捷创建资源描述符
 */
class SpriteResourceBuilder {
public:
    SpriteResourceBuilder(const std::string& id) : _descriptor() {
        _descriptor.resourceId = id;
    }
    
    SpriteResourceBuilder& withSpritePath(const std::string& path) {
        _descriptor.spritePath = path;
        return *this;
    }
    
    SpriteResourceBuilder& withFrames(const std::vector<std::string>& framePaths) {
        _descriptor.framePaths = framePaths;
        return *this;
    }
    
    SpriteResourceBuilder& withAnchorPoint(float x, float y) {
        _descriptor.anchorPoint = cocos2d::Vec2(x, y);
        return *this;
    }
    
    SpriteResourceDescriptor build() const {
        return _descriptor;
    }
    
    void registerToManager() const {
        SpriteManager::getInstance().registerResource(_descriptor);
    }
    
private:
    SpriteResourceDescriptor _descriptor;
};

} // namespace ecs

#endif // __ECS_SPRITE_MANAGER_H__
