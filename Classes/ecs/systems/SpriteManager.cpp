#include "SpriteManager.h"

namespace ecs {

void SpriteManager::registerResource(const SpriteResourceDescriptor& descriptor) {
    _resourceDescriptors[descriptor.resourceId] = descriptor;
    CCLOG("SpriteManager: Registered resource '%s' (path=%s, frames=%zu)",
          descriptor.resourceId.c_str(),
          descriptor.spritePath.c_str(),
          descriptor.framePaths.size());
}

cocos2d::Sprite* SpriteManager::createSprite(const std::string& resourceId) {
    auto it = _resourceDescriptors.find(resourceId);
    if (it == _resourceDescriptors.end()) {
        CCLOG("SpriteManager: Resource '%s' not found", resourceId.c_str());
        return nullptr;
    }
    
    const auto& desc = it->second;
    cocos2d::Sprite* sprite = nullptr;
    
    // 如果有单帧路径，使用单帧创建
    if (!desc.spritePath.empty()) {
        sprite = cocos2d::Sprite::create(desc.spritePath);
        if (!sprite) {
            CCLOG("SpriteManager: Failed to create sprite from '%s'", desc.spritePath.c_str());
            return nullptr;
        }
    }
    // 如果有多帧，使用第一帧创建
    else if (!desc.framePaths.empty()) {
        sprite = cocos2d::Sprite::create(desc.framePaths[0]);
        if (!sprite) {
            CCLOG("SpriteManager: Failed to create sprite from frame '%s'", desc.framePaths[0].c_str());
            return nullptr;
        }
    }
    else {
        CCLOG("SpriteManager: Resource '%s' has no sprite path", resourceId.c_str());
        return nullptr;
    }
    
    // 设置锚点
    sprite->setAnchorPoint(desc.anchorPoint);
    
    // retain以便外部管理
    sprite->retain();
    
    // 记录引用计数
    _spriteRefCount[sprite] = 1;
    
    return sprite;
}

cocos2d::Sprite* SpriteManager::createSpriteAndAttach(const std::string& resourceId,
                                                      cocos2d::Node* parent,
                                                      int zOrder) {
    if (!parent) {
        CCLOG("SpriteManager: Cannot attach sprite, parent is null");
        return nullptr;
    }
    
    auto* sprite = createSprite(resourceId);
    if (sprite) {
        parent->addChild(sprite, zOrder);
    }
    
    return sprite;
}

const cocos2d::Vector<cocos2d::SpriteFrame*>& SpriteManager::getAnimationFrames(const std::string& resourceId) {
    // 检查缓存
    auto cacheIt = _animationFrameCache.find(resourceId);
    if (cacheIt != _animationFrameCache.end()) {
        return cacheIt->second;
    }
    
    // 懒加载动画帧
    loadAnimationFrames(resourceId);
    
    return _animationFrameCache[resourceId];
}

void SpriteManager::loadAnimationFrames(const std::string& resourceId) {
    auto it = _resourceDescriptors.find(resourceId);
    if (it == _resourceDescriptors.end()) {
        CCLOG("SpriteManager: Cannot load frames for unknown resource '%s'", resourceId.c_str());
        return;
    }
    
    const auto& desc = it->second;
    cocos2d::Vector<cocos2d::SpriteFrame*> frames;
    
    // 加载所有帧
    for (const auto& framePath : desc.framePaths) {
        auto* texture = cocos2d::Director::getInstance()->getTextureCache()->addImage(framePath);
        if (texture) {
            auto* frame = cocos2d::SpriteFrame::createWithTexture(
                texture,
                cocos2d::Rect(0, 0, texture->getContentSize().width, texture->getContentSize().height)
            );
            if (frame) {
                frames.pushBack(frame);
            }
        } else {
            CCLOG("SpriteManager: Failed to load frame '%s'", framePath.c_str());
        }
    }
    
    _animationFrameCache[resourceId] = frames;
    
    CCLOG("SpriteManager: Loaded %zu frames for resource '%s'",
          frames.size(), resourceId.c_str());
}

bool SpriteManager::preloadResource(const std::string& resourceId) {
    auto it = _resourceDescriptors.find(resourceId);
    if (it == _resourceDescriptors.end()) {
        return false;
    }
    
    // 如果有多帧，预加载动画帧
    if (!it->second.framePaths.empty()) {
        loadAnimationFrames(resourceId);
        return _animationFrameCache.find(resourceId) != _animationFrameCache.end();
    }
    
    // 如果是单帧，预加载纹理
    if (!it->second.spritePath.empty()) {
        auto* texture = cocos2d::Director::getInstance()->getTextureCache()->addImage(it->second.spritePath);
        return texture != nullptr;
    }
    
    return false;
}

void SpriteManager::releaseSprite(cocos2d::Sprite* sprite) {
    if (!sprite) return;
    
    auto it = _spriteRefCount.find(sprite);
    if (it != _spriteRefCount.end()) {
        it->second--;
        if (it->second <= 0) {
            sprite->removeFromParent();
            sprite->release();
            _spriteRefCount.erase(it);
        }
    } else {
        // 如果没有在引用计数中，直接释放
        sprite->removeFromParent();
        sprite->release();
    }
}

void SpriteManager::clearAll() {
    // 释放所有精灵
    for (auto& pair : _spriteRefCount) {
        if (pair.first) {
            pair.first->removeFromParent();
            pair.first->release();
        }
    }
    _spriteRefCount.clear();
    
    // 清空缓存
    _animationFrameCache.clear();
    _resourceDescriptors.clear();
    
    CCLOG("SpriteManager: Cleared all resources");
}

bool SpriteManager::hasResource(const std::string& resourceId) const {
    return _resourceDescriptors.find(resourceId) != _resourceDescriptors.end();
}

const SpriteResourceDescriptor* SpriteManager::getResourceDescriptor(const std::string& resourceId) const {
    auto it = _resourceDescriptors.find(resourceId);
    if (it != _resourceDescriptors.end()) {
        return &it->second;
    }
    return nullptr;
}

} // namespace ecs
