#ifndef __ECS_SLIME_ATLAS_HELPER_H__
#define __ECS_SLIME_ATLAS_HELPER_H__

#include "SpriteManager.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace ecs {

/**
 * @brief 史莱姆图集帮助类 - 自动注册史莱姆的图集帧映射
 * 
 * 使用方法：
 * 1. 在场景初始化时调用 SlimeAtlasHelper::setupBatchMode(this)
 * 2. 之后创建的史莱姆会自动使用批处理渲染
 * 
 * 图集帧命名规则：
 * - 目录名_文件名前缀+帧号
 * - 例如：GreenSlime_Green_Slime1, GreenSlime_Green_Slime2
 */
class SlimeAtlasHelper {
public:
    /**
     * @brief 设置批处理模式并注册所有史莱姆的图集帧映射
     * @param parent 父节点
     * @param atlasPath 图集plist路径（默认 "atlas/slimes_atlas.plist"）
     * @param texturePath 图集纹理路径（默认 "atlas/slimes_atlas.png"）
     * @return 是否成功
     */
    static bool setupBatchMode(cocos2d::Node* parent,
                               const std::string& atlasPath = "atlas/slimes_atlas.plist",
                               const std::string& texturePath = "atlas/slimes_atlas.png");
    
    /**
     * @brief 注册单个史莱姆类型的图集帧映射
     * @param monsterId 怪物ID（如 "GreenSlime"）
     * @param dirName 目录名（如 "GreenSlime"）
     * @param prefix 文件名前缀（如 "Green_Slime"）
     * @param frameCount 帧数
     */
    static void registerSlimeFrames(const std::string& monsterId,
                                    const std::string& dirName,
                                    const std::string& prefix,
                                    int frameCount);
    
    /**
     * @brief 注册所有已知史莱姆类型的图集帧映射
     */
    static void registerAllSlimeFrames();
    
private:
    // 史莱姆类型配置
    struct SlimeTypeConfig {
        std::string dirName;    // 目录名
        std::string prefix;     // 文件名前缀
        int frameCount;         // 帧数
    };
    
    // 所有已知的史莱姆类型
    static const std::unordered_map<std::string, SlimeTypeConfig>& getSlimeTypes();
};

} // namespace ecs

#endif // __ECS_SLIME_ATLAS_HELPER_H__
