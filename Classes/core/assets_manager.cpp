#include "assets_manager.h"
#include "utils/tools.h"
#include "entt/entt.hpp"
#include "json/rapidjson.h"
#include "json/writer.h"


BlockConfig::BlockConfig(optional_id id, rapidjson::Document* config)
    : 
    _id(id),
    _config(*config),
    _name(tools::get_str_or(_config, "name", "error")),
    _texturePath(tools::get_str_or(_config, "texture", "error")),
    _texture(cocos2d::Director::getInstance()->getTextureCache()->addImage(_texturePath)),
    _replacable(tools::get_bool_or(_config, "replacable", false)),
    _renderable(tools::get_bool_or(_config, "renderable", false)),
    _interactable(tools::get_bool_or(_config, "interactable", false)),
    _collision(tools::get_bool_or(_config, "collision", false))
{}
const optional_id BlockConfig::id() const
{
    return _id;
};

const std::string BlockConfig::name() const
{
    return _name;
}

const std::string BlockConfig::texturePath() const
{
    return _texturePath;
}

cocos2d::Texture2D* BlockConfig::texture() const
{
    return _texture;
}

bool BlockConfig::isReplacable() const
{
    return _replacable;
}

bool BlockConfig::isRenderble() const
{
    return _renderable;
}

bool BlockConfig::isInteractable() const
{
    return _interactable;
}

bool BlockConfig::hasCollision() const
{
    return _collision;
}

const rapidjson::Document& BlockConfig::getConfig() const
{
    return _config;
}

BlockConfig::operator bool() const
{
    return _id.has_value();
}

AssetManager::AssetManager() 
{
    loadAllBlockJson();
};

cocos2d::Texture2D* const AssetManager::getTexture(const std::string& path) const 
{
    return cocos2d::Director::getInstance()->getTextureCache()->addImage(path);
}

const BlockConfig& AssetManager::getBlockConfig(entt::id_type id) const 
{
    return *(_block_config.at(id));
}

bool AssetManager::init() 
{
    loadAllBlockJson();
    return true;
}

void AssetManager::loadAllBlockJson() {
    auto folder_name = "blocks/configs";
    auto json_files = tools::get_all_json(folder_name);
    auto file_utils = cocos2d::FileUtils::getInstance();
    for (const auto& json : json_files) {
        const std::string json_str = file_utils->getStringFromFile(json);
        
        rapidjson::Document* config = new rapidjson::Document();
        config->Parse(json_str.c_str());
        auto id = tools::get_str(*config, "id");

        if (id.has_value()) {
            auto id_hashed = entt::hashed_string(id.value().c_str());
            _block_config[id_hashed] = new BlockConfig(optional_id(id_hashed), config);
        }
    }
}
