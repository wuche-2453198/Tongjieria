#include "cocos2d.h"
#include "entt/entt.hpp"
#include "json/document.h"
#pragma once

/**
* @brief 资源缓存中心。
*/
class AssetManager {
public:
    AssetManager();

    /**
    * @brief 根据路径获取贴图。
    */
    cocos2d::Texture2D* const getTexture(const std::string& path) const;

    /**
    * @brief 根据id获取方块配置。
    */
    const rapidjson::Document* const getBlockConfig(entt::id_type id) const;

    /**
    * @brief 初始化，此时读取所有的json文件。
    */
    bool init() ;
private:
    
    /**
    * @brief 读取所有方块。
    */
    void loadAllBlockJson();

    std::unordered_map<entt::id_type, rapidjson::Document*> _block_config; ///< 方块配置
};