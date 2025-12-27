#include "BatchRenderManager.h"

USING_NS_CC;

namespace ecs {

bool BatchRenderManager::initBatchNode(const std::string& textureKey,
                                       const std::string& texturePath,
                                       Node* parent,
                                       int zOrder) {
    if (!parent) {
        CCLOG("BatchRenderManager: Cannot init batch node, parent is null");
        return false;
    }
    
    // 如果已存在，先移除
    if (_batchNodes.find(textureKey) != _batchNodes.end()) {
        removeBatchNode(textureKey);
    }
    
    // 加载纹理
    auto* texture = Director::getInstance()->getTextureCache()->addImage(texturePath);
    if (!texture) {
        CCLOG("BatchRenderManager: Failed to load texture: %s", texturePath.c_str());
        return false;
    }
    
    // 创建批处理节点
    auto* batchNode = SpriteBatchNode::createWithTexture(texture);
    if (!batchNode) {
        CCLOG("BatchRenderManager: Failed to create batch node for: %s", textureKey.c_str());
        return false;
    }
    
    parent->addChild(batchNode, zOrder);
    
    _batchNodes[textureKey] = batchNode;
    _textures[textureKey] = texture;
    
    CCLOG("BatchRenderManager: Created batch node '%s' with texture '%s'",
          textureKey.c_str(), texturePath.c_str());
    
    return true;
}

bool BatchRenderManager::initBatchNodeWithAtlas(const std::string& textureKey,
                                                const std::string& plistPath,
                                                const std::string& texturePath,
                                                Node* parent,
                                                int zOrder) {
    if (!parent) {
        CCLOG("BatchRenderManager: Cannot init batch node, parent is null");
        return false;
    }
    
    // 如果已存在，先移除
    if (_batchNodes.find(textureKey) != _batchNodes.end()) {
        removeBatchNode(textureKey);
    }
    
    // 加载精灵帧缓存
    SpriteFrameCache::getInstance()->addSpriteFramesWithFile(plistPath, texturePath);
    
    // 加载纹理
    auto* texture = Director::getInstance()->getTextureCache()->addImage(texturePath);
    if (!texture) {
        CCLOG("BatchRenderManager: Failed to load atlas texture: %s", texturePath.c_str());
        return false;
    }
    
    // 创建批处理节点
    auto* batchNode = SpriteBatchNode::createWithTexture(texture);
    if (!batchNode) {
        CCLOG("BatchRenderManager: Failed to create batch node for atlas: %s", textureKey.c_str());
        return false;
    }
    
    parent->addChild(batchNode, zOrder);
    
    _batchNodes[textureKey] = batchNode;
    _textures[textureKey] = texture;
    
    CCLOG("BatchRenderManager: Created batch node '%s' with atlas '%s'",
          textureKey.c_str(), plistPath.c_str());
    
    return true;
}

Sprite* BatchRenderManager::createBatchedSprite(const std::string& textureKey,
                                                const std::string& frameName) {
    auto it = _batchNodes.find(textureKey);
    if (it == _batchNodes.end()) {
        CCLOG("BatchRenderManager: Batch node '%s' not found", textureKey.c_str());
        return nullptr;
    }
    
    Sprite* sprite = nullptr;
    
    if (frameName.empty()) {
        // 使用整张纹理
        sprite = Sprite::createWithTexture(it->second->getTexture());
    } else {
        // 使用帧名称
        sprite = Sprite::createWithSpriteFrameName(frameName);
    }
    
    if (sprite) {
        it->second->addChild(sprite);
    }
    
    return sprite;
}

Sprite* BatchRenderManager::createBatchedSpriteWithRect(const std::string& textureKey,
                                                        const Rect& rect) {
    auto it = _batchNodes.find(textureKey);
    if (it == _batchNodes.end()) {
        CCLOG("BatchRenderManager: Batch node '%s' not found", textureKey.c_str());
        return nullptr;
    }
    
    auto* sprite = Sprite::createWithTexture(it->second->getTexture(), rect);
    if (sprite) {
        it->second->addChild(sprite);
    }
    
    return sprite;
}

SpriteBatchNode* BatchRenderManager::getBatchNode(const std::string& textureKey) {
    auto it = _batchNodes.find(textureKey);
    return (it != _batchNodes.end()) ? it->second : nullptr;
}

Texture2D* BatchRenderManager::getTexture(const std::string& textureKey) {
    auto it = _textures.find(textureKey);
    return (it != _textures.end()) ? it->second : nullptr;
}

bool BatchRenderManager::hasBatchNode(const std::string& textureKey) const {
    return _batchNodes.find(textureKey) != _batchNodes.end();
}

void BatchRenderManager::removeBatchNode(const std::string& textureKey) {
    auto it = _batchNodes.find(textureKey);
    if (it != _batchNodes.end()) {
        if (it->second) {
            it->second->removeFromParent();
        }
        _batchNodes.erase(it);
    }
    
    _textures.erase(textureKey);
    
    CCLOG("BatchRenderManager: Removed batch node '%s'", textureKey.c_str());
}

void BatchRenderManager::cleanup() {
    for (auto& pair : _batchNodes) {
        if (pair.second) {
            pair.second->removeFromParent();
        }
    }
    _batchNodes.clear();
    _textures.clear();
    
    CCLOG("BatchRenderManager: Cleaned up all batch nodes");
}

unsigned long BatchRenderManager::getDrawCallCount() {
    return Director::getInstance()->getRenderer()->getDrawnBatches();
}

} // namespace ecs
