#ifndef __ECS_SPRITE_MANAGER_H__
#define __ECS_SPRITE_MANAGER_H__

#include "components/AllComponents.h"
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
 * - 支持批处理渲染（减少Draw Calls）
 * 
 * 设计模式：单例
 * 
 * 批处理模式：
 * - 调用 enableBatchMode() 启用批处理
 * - 使用相同纹理的Sprite会被添加到SpriteBatchNode
 * - 可大幅减少Draw Calls（从N个降到1个）
 */
class SpriteManager {
public:
    static SpriteManager& getInstance() {
        static SpriteManager instance;
        return instance;
    }
    
    // ==================== 批处理模式 ====================
    
    /**
     * @brief 启用批处理模式
     * @param atlasPath 纹理图集plist路径（如 "atlas/slimes_atlas.plist"）
     * @param texturePath 纹理图片路径（如 "atlas/slimes_atlas.png"）
     * @param parent 父节点（批处理节点将添加到此节点）
     * @param zOrder 层级
     * @return 是否成功
     */
    bool enableBatchMode(const std::string& atlasPath,
                         const std::string& texturePath,
                         cocos2d::Node* parent,
                         int zOrder = 1);
    
    /**
     * @brief 禁用批处理模式
     */
    void disableBatchMode();
    
    /**
     * @brief 检查是否启用了批处理模式
     */
    bool isBatchModeEnabled() const { return _batchModeEnabled; }
    
    /**
     * @brief 启用/禁用多边形精灵（AutoPolygon）
     * 多边形精灵通过减少像素填充来提高性能
     * 适用于有大量透明区域的精灵
     */
    void setUsePolygonSprites(bool use) { _usePolygonSprites = use; }
    bool isUsingPolygonSprites() const { return _usePolygonSprites; }
    
    /**
     * @brief 注册资源到图集的帧名称映射
     * @param resourceId 资源ID（如 "GreenSlime_sprite"）
     * @param frameNames 图集中的帧名称列表（如 ["GreenSlime_Green_Slime1", "GreenSlime_Green_Slime2"]）
     */
    void registerAtlasFrames(const std::string& resourceId, 
                             const std::vector<std::string>& frameNames);
    
    // ==================== 原有接口 ====================
    
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
    
    // ==================== 批处理模式相关 ====================
    bool _batchModeEnabled = false;
    cocos2d::Node* _batchParent = nullptr;
    int _batchZOrder = 1;
    bool _usePolygonSprites = false;  // 是否使用多边形精灵（AutoPolygon）
    
    // 资源ID到图集帧名称的映射
    std::unordered_map<std::string, std::vector<std::string>> _atlasFrameMapping;
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
