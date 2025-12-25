#include "cocos2d.h"
#include "entt/entt.hpp"
#include "json/document.h"
#pragma once

class BlockConfig
{
public:
    BlockConfig(rapidjson::Document* const config);
    const std::string name() const;
    const std::string texturePath() const;
    cocos2d::Texture2D* texture() const;
    bool isReplacable() const;
    bool isRenderble() const;
    bool isInteractable() const;
    bool hasCollision() const;
    rapidjson::Document* const getConfig() const;
private:
    const std::string& _name;
    const std::string& _texturePath;
    cocos2d::Texture2D* _texture;
    bool _replacable;
    bool _renderable;
    bool _interactable;
    bool _collision;
    rapidjson::Document* _config;
};

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
    const BlockConfig& getBlockConfig(entt::id_type id) const;

    /**
    * @brief 初始化，此时读取所有的json文件。
    */
    bool init();
private:
    
    /**
    * @brief 读取所有方块。
    */
    void loadAllBlockJson();

    std::unordered_map<entt::id_type, BlockConfig*> _block_config; ///< 方块配置
};