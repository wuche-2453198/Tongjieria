#ifndef __ECS_GAME_ATLAS_MANAGER_H__
#define __ECS_GAME_ATLAS_MANAGER_H__

#include "SpriteManager.h"
#include <string>
#include <unordered_map>

namespace ecs {

/**
 * @brief 游戏图集管理器 - 统一管理所有精灵的图集帧映射
 * 
 * 优化原理（基于Cocos2d-x 3.x文档）：
 * - 将所有精灵放入同一张spritesheet
 * - 使用相同的着色器和混合函数（默认）
 * - Cocos2d-x渲染器会自动批处理连续的QuadCommand
 * 
 * 使用方法：
 * 1. 运行 tools/generate_game_atlas.py 生成图集
 * 2. 在场景初始化时调用 GameAtlasManager::initialize()
 * 3. 所有使用图集的精灵会自动批处理
 */
class GameAtlasManager {
public:
    static GameAtlasManager& getInstance() {
        static GameAtlasManager instance;
        return instance;
    }
    
    /**
     * @brief 初始化图集系统
     * @return 是否成功
     */
    bool initialize();
    
    /**
     * @brief 检查是否已初始化
     */
    bool isInitialized() const { return _initialized; }
    
    /**
     * @brief 获取图集帧名称
     * @param originalPath 原始路径（如 "picture/Projectile/Ice_Spike.png"）
     * @return 图集帧名称，如果不存在返回空字符串
     */
    std::string getFrameName(const std::string& originalPath) const;
    
    /**
     * @brief 注册怪物的图集帧映射
     * @param monsterId 怪物ID
     * @param dirName 目录名
     * @param prefix 文件前缀
     * @param frameCount 帧数
     */
    void registerMonsterFrames(const std::string& monsterId,
                               const std::string& dirName,
                               const std::string& prefix,
                               int frameCount);
    
    /**
     * @brief 注册投射物的图集帧映射
     * @param projectilePath 投射物原始路径
     */
    void registerProjectileFrame(const std::string& projectilePath);
    
    /**
     * @brief 注册所有已知的帧映射
     */
    void registerAllFrames();
    
private:
    GameAtlasManager() = default;
    
    bool _initialized = false;
    
    // 原始路径到图集帧名称的映射
    std::unordered_map<std::string, std::string> _pathToFrameMap;
};

} // namespace ecs

#endif // __ECS_GAME_ATLAS_MANAGER_H__
