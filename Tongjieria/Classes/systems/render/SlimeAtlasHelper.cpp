#include "SlimeAtlasHelper.h"

namespace ecs {

const std::unordered_map<std::string, SlimeAtlasHelper::SlimeTypeConfig>& 
SlimeAtlasHelper::getSlimeTypes() {
    // 静态配置表：怪物ID -> (目录名, 文件前缀, 帧数)
    static const std::unordered_map<std::string, SlimeTypeConfig> slimeTypes = {
        // 基础史莱姆
        {"GreenSlime",      {"GreenSlime",      "Green_Slime",      2}},
        {"BlueSlime",       {"BlueSlime",       "Blue_Slime",       2}},
        {"RedSlime",        {"RedSlime",        "Red_Slime",        2}},
        {"YellowSlime",     {"YellowSlime",     "Yellow_Slime",     2}},
        {"PurpleSlime",     {"PurpleSlime",     "Purple_Slime",     2}},
        {"PinkSlime",       {"PinkSlime",       "Pink_Slime",       2}},
        {"IceSlime",        {"IceSlime",        "Ice_Slime",        2}},
        {"BlackSlime",      {"BlackSlime",      "Black_Slime",      2}},
        {"JungleSlime",     {"JungleSlime",     "Jungle_Slime",     2}},
        
        // 特殊史莱姆
        {"BabySlime",       {"BabySlime",       "Baby_Slime",       2}},
        {"MotherSlime",     {"MotherSlime",     "Mother_Slime",     2}},
        {"UmbrellaSlime",   {"UmbrellaSlime",   "Umbrella_Slime",   2}},
        
        // 尖刺史莱姆
        {"SpikedSlime",     {"Spiked_Slime",    "Spiked_Slime",     2}},
        {"SpikedIceSlime",  {"Spiked_IceSlime", "Spiked_Ice_Slime", 2}},
        {"SpikedJungleSlime", {"Spiked_JungleSlime", "Spiked_Jungle_Slime", 2}},
    };
    
    return slimeTypes;
}

bool SlimeAtlasHelper::setupBatchMode(cocos2d::Node* parent,
                                      const std::string& atlasPath,
                                      const std::string& texturePath) {
    auto& manager = SpriteManager::getInstance();
    
    // 检查图集文件是否存在
    if (!cocos2d::FileUtils::getInstance()->isFileExist(atlasPath)) {
        CCLOG("SlimeAtlasHelper: Atlas file not found: %s", atlasPath.c_str());
        CCLOG("SlimeAtlasHelper: Run tools/generate_slime_atlas.py to generate the atlas");
        return false;
    }
    
    // 启用批处理模式
    if (!manager.enableBatchMode(atlasPath, texturePath, parent)) {
        CCLOG("SlimeAtlasHelper: Failed to enable batch mode");
        return false;
    }
    
    // 注册所有史莱姆的图集帧映射
    registerAllSlimeFrames();
    
    CCLOG("SlimeAtlasHelper: Batch mode enabled with %zu slime types",
          getSlimeTypes().size());
    
    return true;
}

void SlimeAtlasHelper::registerSlimeFrames(const std::string& monsterId,
                                           const std::string& dirName,
                                           const std::string& prefix,
                                           int frameCount) {
    // 构建资源ID（与MonsterFactory中的命名一致）
    std::string resourceId = monsterId + "_sprite";
    
    // 构建帧名称列表
    // 图集帧命名规则：目录名_文件前缀+帧号
    std::vector<std::string> frameNames;
    for (int i = 1; i <= frameCount; i++) {
        std::string frameName = dirName + "_" + prefix + std::to_string(i);
        frameNames.push_back(frameName);
    }
    
    // 注册到SpriteManager
    SpriteManager::getInstance().registerAtlasFrames(resourceId, frameNames);
}

void SlimeAtlasHelper::registerAllSlimeFrames() {
    const auto& slimeTypes = getSlimeTypes();
    
    for (const auto& pair : slimeTypes) {
        const std::string& monsterId = pair.first;
        const SlimeTypeConfig& config = pair.second;
        
        registerSlimeFrames(monsterId, config.dirName, config.prefix, config.frameCount);
    }
    
    CCLOG("SlimeAtlasHelper: Registered %zu slime frame mappings", slimeTypes.size());
}

} // namespace ecs
