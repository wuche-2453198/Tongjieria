#ifndef __ECS_BATCH_RENDER_MANAGER_H__
#define __ECS_BATCH_RENDER_MANAGER_H__

#include "cocos2d.h"
#include <unordered_map>
#include <string>

namespace ecs {

/**
 * @brief 批处理渲染管理器 - 管理SpriteBatchNode以减少Draw Calls
 * 
 * 核心优化：
 * - 将使用相同纹理的Sprite合并到SpriteBatchNode
 * - 大幅减少Draw Calls（从N个降到1个）
 * - 适用于大量同类型怪物的场景
 * 
 * 使用方法：
 * 1. 场景初始化时调用 initBatchNode() 创建批处理节点
 * 2. 创建Sprite时使用 createBatchedSprite() 代替 Sprite::create()
 * 3. 场景销毁时调用 cleanup() 清理资源
 */
class BatchRenderManager {
public:
    static BatchRenderManager& getInstance() {
        static BatchRenderManager instance;
        return instance;
    }
    
    /**
     * @brief 初始化批处理节点
     * @param textureKey 纹理标识符（用于区分不同类型的批处理）
     * @param texturePath 纹理路径（单张图或图集）
     * @param parent 父节点
     * @param zOrder 层级
     * @return 是否成功
     */
    bool initBatchNode(const std::string& textureKey, 
                       const std::string& texturePath,
                       cocos2d::Node* parent,
                       int zOrder = 1);
    
    /**
     * @brief 使用纹理图集初始化批处理节点
     * @param textureKey 纹理标识符
     * @param plistPath plist文件路径
     * @param texturePath 纹理图片路径
     * @param parent 父节点
     * @param zOrder 层级
     * @return 是否成功
     */
    bool initBatchNodeWithAtlas(const std::string& textureKey,
                                const std::string& plistPath,
                                const std::string& texturePath,
                                cocos2d::Node* parent,
                                int zOrder = 1);
    
    /**
     * @brief 创建批处理精灵
     * @param textureKey 纹理标识符
     * @param frameName 帧名称（用于图集）或空（用于整张纹理）
     * @return 精灵指针，失败返回nullptr
     */
    cocos2d::Sprite* createBatchedSprite(const std::string& textureKey,
                                         const std::string& frameName = "");
    
    /**
     * @brief 创建批处理精灵并设置帧
     * @param textureKey 纹理标识符
     * @param rect 纹理区域
     * @return 精灵指针
     */
    cocos2d::Sprite* createBatchedSpriteWithRect(const std::string& textureKey,
                                                  const cocos2d::Rect& rect);
    
    /**
     * @brief 获取批处理节点
     * @param textureKey 纹理标识符
     * @return 批处理节点指针
     */
    cocos2d::SpriteBatchNode* getBatchNode(const std::string& textureKey);
    
    /**
     * @brief 获取纹理
     * @param textureKey 纹理标识符
     * @return 纹理指针
     */
    cocos2d::Texture2D* getTexture(const std::string& textureKey);
    
    /**
     * @brief 检查是否已初始化批处理节点
     */
    bool hasBatchNode(const std::string& textureKey) const;
    
    /**
     * @brief 移除批处理节点
     */
    void removeBatchNode(const std::string& textureKey);
    
    /**
     * @brief 清理所有批处理节点
     */
    void cleanup();
    
    /**
     * @brief 获取当前Draw Call数量（调试用）
     */
    static unsigned long getDrawCallCount();
    
private:
    BatchRenderManager() = default;
    ~BatchRenderManager() { cleanup(); }
    BatchRenderManager(const BatchRenderManager&) = delete;
    BatchRenderManager& operator=(const BatchRenderManager&) = delete;
    
    // 批处理节点映射：textureKey -> SpriteBatchNode
    std::unordered_map<std::string, cocos2d::SpriteBatchNode*> _batchNodes;
    
    // 纹理映射：textureKey -> Texture2D
    std::unordered_map<std::string, cocos2d::Texture2D*> _textures;
};

} // namespace ecs

#endif // __ECS_BATCH_RENDER_MANAGER_H__
