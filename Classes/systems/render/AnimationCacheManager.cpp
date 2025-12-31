#include "AnimationCacheManager.h"

namespace ecs {

AnimationCacheManager& AnimationCacheManager::getInstance() {
    static AnimationCacheManager instance;
    return instance;
}

AnimationCacheManager::~AnimationCacheManager() {
    clearAll();
}

cocos2d::Animation* AnimationCacheManager::getAnimation(const std::string& animationId) {
    auto it = _animationCache.find(animationId);
    if (it != _animationCache.end()) {
        return it->second;
    }
    return nullptr;
}

bool AnimationCacheManager::preloadAnimation(const std::string& animationId,
                                              const std::vector<std::string>& frameNames,
                                              float frameTime,
                                              int loops) {
    // 检查是否已缓存
    if (hasAnimation(animationId)) {
        CCLOG("AnimationCacheManager: Animation '%s' already cached", animationId.c_str());
        return true;
    }
    
    // 获取精灵帧
    cocos2d::Vector<cocos2d::SpriteFrame*> frames;
    auto* frameCache = cocos2d::SpriteFrameCache::getInstance();
    
    for (const auto& frameName : frameNames) {
        auto* frame = frameCache->getSpriteFrameByName(frameName);
        if (frame) {
            frames.pushBack(frame);
        } else {
            CCLOG("AnimationCacheManager: Frame '%s' not found for animation '%s'",
                  frameName.c_str(), animationId.c_str());
        }
    }
    
    if (frames.empty()) {
        CCLOG("AnimationCacheManager: No valid frames for animation '%s'", animationId.c_str());
        return false;
    }
    
    // 创建动画
    auto* animation = cocos2d::Animation::createWithSpriteFrames(frames, frameTime);
    if (!animation) {
        CCLOG("AnimationCacheManager: Failed to create animation '%s'", animationId.c_str());
        return false;
    }
    
    // 设置循环次数
    animation->setLoops(loops);
    
    // retain 并缓存
    animation->retain();
    _animationCache[animationId] = animation;
    
    CCLOG("AnimationCacheManager: Preloaded animation '%s' with %zu frames",
          animationId.c_str(), frames.size());
    
    return true;
}

bool AnimationCacheManager::preloadAnimationWithRange(const std::string& animationId,
                                                       const std::string& framePrefix,
                                                       int startIndex,
                                                       int endIndex,
                                                       float frameTime,
                                                       int loops) {
    std::vector<std::string> frameNames;
    for (int i = startIndex; i <= endIndex; ++i) {
        frameNames.push_back(framePrefix + std::to_string(i));
    }
    return preloadAnimation(animationId, frameNames, frameTime, loops);
}

bool AnimationCacheManager::hasAnimation(const std::string& animationId) const {
    return _animationCache.find(animationId) != _animationCache.end();
}

void AnimationCacheManager::removeAnimation(const std::string& animationId) {
    auto it = _animationCache.find(animationId);
    if (it != _animationCache.end()) {
        it->second->release();
        _animationCache.erase(it);
        CCLOG("AnimationCacheManager: Removed animation '%s'", animationId.c_str());
    }
}

void AnimationCacheManager::cleanup() {
    // 移除引用计数为1的动画（只有缓存持有）
    std::vector<std::string> toRemove;
    
    for (const auto& pair : _animationCache) {
        if (pair.second->getReferenceCount() == 1) {
            toRemove.push_back(pair.first);
        }
    }
    
    for (const auto& id : toRemove) {
        removeAnimation(id);
    }
    
    if (!toRemove.empty()) {
        CCLOG("AnimationCacheManager: Cleaned up %zu unused animations", toRemove.size());
    }
}

void AnimationCacheManager::clearAll() {
    for (auto& pair : _animationCache) {
        pair.second->release();
    }
    _animationCache.clear();
    CCLOG("AnimationCacheManager: Cleared all animations");
}

void AnimationCacheManager::printCacheStatus() const {
    CCLOG("AnimationCacheManager: Cache status - %zu animations", _animationCache.size());
    for (const auto& pair : _animationCache) {
        CCLOG("  - '%s': refCount=%u", 
              pair.first.c_str(), 
              pair.second->getReferenceCount());
    }
}

} // namespace ecs
