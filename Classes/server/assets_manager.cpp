#include "assets_manager.h"
#include "entt/entt.hpp"
#include "json/rapidjson.h"
#include "json/writer.h"

AssetManager::AssetManager() 
{
    loadAllBlockJson();
};

cocos2d::Texture2D* const AssetManager::getTexture(const std::string& path) const {
    return cocos2d::Director::getInstance()->getTextureCache()->addImage(path);
}

const rapidjson::Document* const AssetManager::getBlockConfig(entt::id_type id) const {
    return _block_config.at(id);
}

bool AssetManager::init() {
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
            _block_config[entt::hashed_string(id.value().c_str())] = config;
        }
    }
}
