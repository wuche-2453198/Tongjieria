#include "SpriteManager.h"

namespace ecs {

// ==================== 图集模式实现 ====================

bool SpriteManager::enableBatchMode(const std::string& atlasPath,
                                    const std::string& texturePath,
                                    cocos2d::Node* parent,
                                    int zOrder) {
    if (!parent) {
        CCLOG("SpriteManager: Cannot enable batch mode, parent is null");
        return false;
    }
    
    _batchModeEnabled = false;
    _batchParent = nullptr;
    _batchZOrder = 1;
    
    // 加载图集到SpriteFrameCache（全局缓存）
    auto* cache = cocos2d::SpriteFrameCache::getInstance();
    cache->addSpriteFramesWithFile(atlasPath, texturePath);
    
    // 预加载纹理到TextureCache
    auto* texture = cocos2d::Director::getInstance()->getTextureCache()->addImage(texturePath);
    if (!texture) {
        CCLOG("SpriteManager: Failed to load texture: %s", texturePath.c_str());
        return false;
    }
    
    _batchModeEnabled = true;
    _batchParent = parent;
    _batchZOrder = zOrder;
    
    CCLOG("SpriteManager: Atlas loaded to SpriteFrameCache - auto-batching enabled!");
    return true;
}

void SpriteManager::disableBatchMode() {
    _batchModeEnabled = false;
    _batchParent = nullptr;
    _atlasFrameMapping.clear();
}

void SpriteManager::registerAtlasFrames(const std::string& resourceId,
                                        const std::vector<std::string>& frameNames) {
    _atlasFrameMapping[resourceId] = frameNames;
}

// ==================== 精灵创建 ====================

void SpriteManager::registerResource(const SpriteResourceDescriptor& descriptor) {
    _resourceDescriptors[descriptor.resourceId] = descriptor;
}

cocos2d::Sprite* SpriteManager::createSprite(const std::string& resourceId) {
    auto it = _resourceDescriptors.find(resourceId);
    if (it == _resourceDescriptors.end()) {
        CCLOG("SpriteManager: Resource '%s' not found in descriptors", resourceId.c_str());
        return nullptr;
    }
    
    const auto& desc = it->second;
    cocos2d::Sprite* sprite = nullptr;
    
    CCLOG("SpriteManager: Creating sprite for '%s', batchMode=%d", 
          resourceId.c_str(), _batchModeEnabled ? 1 : 0);
    
    // 优先使用SpriteFrameCache中的图集帧
    if (_batchModeEnabled) {
        auto atlasIt = _atlasFrameMapping.find(resourceId);
        if (atlasIt != _atlasFrameMapping.end() && !atlasIt->second.empty()) {
            const std::string& frameName = atlasIt->second[0];
            CCLOG("SpriteManager: Found atlas frame mapping: '%s' -> '%s'", 
                  resourceId.c_str(), frameName.c_str());
            
            // 使用createWithSpriteFrameName直接从缓存创建（最高效）
            sprite = cocos2d::Sprite::createWithSpriteFrameName(frameName);
            if (sprite) {
                CCLOG("SpriteManager: Created sprite from atlas frame '%s'", frameName.c_str());
                sprite->setAnchorPoint(desc.anchorPoint);
                sprite->retain();
                _spriteRefCount[sprite] = 1;
                return sprite;
            } else {
                CCLOG("SpriteManager: Failed to create sprite from frame name '%s'", frameName.c_str());
            }
            
            // 备用：从SpriteFrameCache获取SpriteFrame
            auto* frame = cocos2d::SpriteFrameCache::getInstance()->getSpriteFrameByName(frameName);
            if (frame) {
                sprite = cocos2d::Sprite::createWithSpriteFrame(frame);
                if (sprite) {
                    CCLOG("SpriteManager: Created sprite from SpriteFrame '%s'", frameName.c_str());
                    sprite->setAnchorPoint(desc.anchorPoint);
                    sprite->retain();
                    _spriteRefCount[sprite] = 1;
                    return sprite;
                }
            } else {
                CCLOG("SpriteManager: SpriteFrame '%s' not found in cache", frameName.c_str());
            }
        } else {
            CCLOG("SpriteManager: No atlas frame mapping for '%s'", resourceId.c_str());
        }
    }
    
    // 非图集模式：从文件创建
    CCLOG("SpriteManager: Falling back to file-based sprite creation for '%s'", resourceId.c_str());
    if (!desc.spritePath.empty()) {
        CCLOG("SpriteManager: Using spritePath: '%s'", desc.spritePath.c_str());
        // 尝试使用AutoPolygon创建多边形精灵（减少像素填充）
        if (_usePolygonSprites) {
            auto pinfo = cocos2d::AutoPolygon::generatePolygon(desc.spritePath);
            sprite = cocos2d::Sprite::create(pinfo);
        } else {
            sprite = cocos2d::Sprite::create(desc.spritePath);
        }
    }
    else if (!desc.framePaths.empty()) {
        CCLOG("SpriteManager: Using framePaths[0]: '%s'", desc.framePaths[0].c_str());
        if (_usePolygonSprites) {
            auto pinfo = cocos2d::AutoPolygon::generatePolygon(desc.framePaths[0]);
            sprite = cocos2d::Sprite::create(pinfo);
        } else {
            sprite = cocos2d::Sprite::create(desc.framePaths[0]);
        }
    }
    
    if (!sprite) {
        CCLOG("SpriteManager: Failed to create sprite for '%s'", resourceId.c_str());
        return nullptr;
    }
    
    sprite->setAnchorPoint(desc.anchorPoint);
    sprite->retain();
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


// ==================== 动画帧获取 ====================

const cocos2d::Vector<cocos2d::SpriteFrame*>& SpriteManager::getAnimationFrames(const std::string& resourceId) {
    // 检查缓存
    auto cacheIt = _animationFrameCache.find(resourceId);
    if (cacheIt != _animationFrameCache.end()) {
        return cacheIt->second;
    }
    
    // 优先从SpriteFrameCache获取图集帧
    if (_batchModeEnabled) {
        auto atlasIt = _atlasFrameMapping.find(resourceId);
        if (atlasIt != _atlasFrameMapping.end() && !atlasIt->second.empty()) {
            cocos2d::Vector<cocos2d::SpriteFrame*> frames;
            auto* cache = cocos2d::SpriteFrameCache::getInstance();
            
            for (const auto& frameName : atlasIt->second) {
                auto* frame = cache->getSpriteFrameByName(frameName);
                if (frame) {
                    frames.pushBack(frame);
                }
            }
            
            if (!frames.empty()) {
                _animationFrameCache[resourceId] = frames;
                return _animationFrameCache[resourceId];
            }
        }
    }
    
    // 从文件加载
    loadAnimationFrames(resourceId);
    return _animationFrameCache[resourceId];
}

void SpriteManager::loadAnimationFrames(const std::string& resourceId) {
    auto it = _resourceDescriptors.find(resourceId);
    if (it == _resourceDescriptors.end()) {
        return;
    }
    
    const auto& desc = it->second;
    cocos2d::Vector<cocos2d::SpriteFrame*> frames;
    
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
        }
    }
    
    _animationFrameCache[resourceId] = frames;
}

bool SpriteManager::preloadResource(const std::string& resourceId) {
    auto it = _resourceDescriptors.find(resourceId);
    if (it == _resourceDescriptors.end()) {
        return false;
    }
    
    if (!it->second.framePaths.empty()) {
        loadAnimationFrames(resourceId);
        return _animationFrameCache.find(resourceId) != _animationFrameCache.end();
    }
    
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
        sprite->removeFromParent();
        sprite->release();
    }
}

void SpriteManager::clearAll() {
    disableBatchMode();
    
    for (auto& pair : _spriteRefCount) {
        if (pair.first) {
            pair.first->removeFromParent();
            pair.first->release();
        }
    }
    _spriteRefCount.clear();
    _animationFrameCache.clear();
    _resourceDescriptors.clear();
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
