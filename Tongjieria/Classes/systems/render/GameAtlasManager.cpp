#include "GameAtlasManager.h"
#include "cocos2d.h"

namespace ecs {

bool GameAtlasManager::initialize() {
    CCLOG("GameAtlasManager::initialize() called, _initialized=%d", _initialized ? 1 : 0);
    
    if (_initialized) {
        CCLOG("GameAtlasManager: Already initialized, returning true");
        return true;
    }
    
    const std::string atlasPath = "atlas/game_atlas.plist";
    const std::string texturePath = "atlas/game_atlas.png";
    
    CCLOG("GameAtlasManager: Checking atlas file: %s", atlasPath.c_str());
    
    // 检查图集文件是否存在
    if (!cocos2d::FileUtils::getInstance()->isFileExist(atlasPath)) {
        CCLOG("GameAtlasManager: Atlas not found: %s", atlasPath.c_str());
        CCLOG("GameAtlasManager: Run tools/generate_game_atlas.py to generate");
        return false;
    }
    
    CCLOG("GameAtlasManager: Atlas file exists, loading...");
    
    // 加载图集到SpriteFrameCache（全局缓存，只加载一次）
    auto* cache = cocos2d::SpriteFrameCache::getInstance();
    cache->addSpriteFramesWithFile(atlasPath, texturePath);
    
    // 预加载纹理到TextureCache
    auto* texture = cocos2d::Director::getInstance()->getTextureCache()->addImage(texturePath);
    if (!texture) {
        CCLOG("GameAtlasManager: Failed to load texture: %s", texturePath.c_str());
        return false;
    }
    
    // 注册所有帧映射
    registerAllFrames();
    
    _initialized = true;
    CCLOG("GameAtlasManager: Initialized with %zu frame mappings", _pathToFrameMap.size());
    CCLOG("GameAtlasManager: SpriteFrameCache loaded - sprites will auto-batch!");
    
    return true;
}

std::string GameAtlasManager::getFrameName(const std::string& originalPath) const {
    auto it = _pathToFrameMap.find(originalPath);
    if (it != _pathToFrameMap.end()) {
        return it->second;
    }
    return "";
}

void GameAtlasManager::registerMonsterFrames(const std::string& monsterId,
                                             const std::string& dirName,
                                             const std::string& prefix,
                                             int frameCount) {
    std::string resourceId = monsterId + "_sprite";
    std::vector<std::string> frameNames;
    
    for (int i = 1; i <= frameCount; i++) {
        // 原始路径格式
        std::string originalPath = "picture/Minor_monster/" + dirName + "/" + 
                                   prefix + std::to_string(i) + ".png";
        // 图集帧名称格式
        std::string frameName = dirName + "_" + prefix + std::to_string(i);
        
        _pathToFrameMap[originalPath] = frameName;
        frameNames.push_back(frameName);
    }
    
    // 注册到SpriteManager
    SpriteManager::getInstance().registerAtlasFrames(resourceId, frameNames);
}

void GameAtlasManager::registerProjectileFrame(const std::string& projectilePath) {
    // 从路径提取文件名作为帧名称
    size_t lastSlash = projectilePath.find_last_of("/\\");
    std::string filename = (lastSlash != std::string::npos) ? 
                           projectilePath.substr(lastSlash + 1) : projectilePath;
    
    // 移除.png扩展名
    size_t dotPos = filename.find_last_of('.');
    if (dotPos != std::string::npos) {
        filename = filename.substr(0, dotPos);
    }
    
    // 图集帧名称格式：Projectile_文件名
    std::string frameName = "Projectile_" + filename;
    _pathToFrameMap[projectilePath] = frameName;
    
    // 注册投射物资源到SpriteManager
    std::string resourceId = "projectile_" + projectilePath;
    std::vector<std::string> frameNames = {frameName};
    SpriteManager::getInstance().registerAtlasFrames(resourceId, frameNames);
}


void GameAtlasManager::registerAllFrames() {
    // 注册所有史莱姆
    // 格式：monsterId, dirName, prefix, frameCount
    struct SlimeConfig {
        const char* monsterId;
        const char* dirName;
        const char* prefix;
        int frameCount;
    };
    
    static const SlimeConfig slimes[] = {
        {"GreenSlime",      "GreenSlime",      "Green_Slime",      2},
        {"BlueSlime",       "BlueSlime",       "Blue_Slime",       2},
        {"RedSlime",        "RedSlime",        "Red_Slime",        2},
        {"YellowSlime",     "YellowSlime",     "Yellow_Slime",     2},
        {"PurpleSlime",     "PurpleSlime",     "Purple_Slime",     2},
        {"PinkSlime",       "PinkSlime",       "Pink_Slime",       2},
        {"IceSlime",        "IceSlime",        "Ice_Slime",        2},
        {"BlackSlime",      "BlackSlime",      "Black_Slime",      2},
        {"JungleSlime",     "JungleSlime",     "Jungle_Slime",     2},
        {"BabySlime",       "BabySlime",       "Baby_Slime",       2},
        {"MotherSlime",     "MotherSlime",     "Mother_Slime",     2},
        {"UmbrellaSlime",   "UmbrellaSlime",   "Umbrella_Slime",   2},
        {"SpikedSlime",     "Spiked_Slime",    "Spiked_Slime",     2},
        {"SpikedIceSlime",  "Spiked_IceSlime", "Spiked_Ice_Slime", 2},
        {"SpikedJungleSlime", "Spiked_JungleSlime", "Spiked_Jungle_Slime", 2},
    };
    
    for (const auto& slime : slimes) {
        registerMonsterFrames(slime.monsterId, slime.dirName, slime.prefix, slime.frameCount);
    }
    
    // 注册所有投射物（注意：路径大小写必须与配置文件一致）
    static const char* projectiles[] = {
        "picture/Projectile/Ice_Spike.png",
        "picture/Projectile/Jungle_Spike.png",
        "picture/Projectile/Slime_Spike.png",
        "picture/Projectile/Sand_Ball.png",
        // 同时注册大写版本以兼容旧配置
        "Picture/Projectile/Ice_Spike.png",
        "Picture/Projectile/Jungle_Spike.png",
        "Picture/Projectile/Slime_Spike.png",
        "Picture/Projectile/Sand_Ball.png",
    };
    
    for (const auto& proj : projectiles) {
        registerProjectileFrame(proj);
    }
    
    CCLOG("GameAtlasManager: Registered %d slime types and %d projectile types",
          (int)(sizeof(slimes) / sizeof(slimes[0])),
          (int)(sizeof(projectiles) / sizeof(projectiles[0])));
}

} // namespace ecs
