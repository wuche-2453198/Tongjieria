#include "assets_manager.h"
#include "utils/tools.h"
#include "entt/entt.hpp"
#include "json/rapidjson.h"
#include "json/writer.h"


BlockConfig::BlockConfig() : _id(std::nullopt) {}

BlockConfig::BlockConfig(optional_id id, rapidjson::Document* config)
{
    _id = id;
    _config = config;

    auto states = tools::get_obj(*_config, "states");
    if (states)
    {
        state stateCode = 0;
        for (auto it = states->MemberBegin(); it != states->MemberEnd(); ++it)
        {
            stateCodeToName[stateCode] = it->name.GetString();
            stateCode++;
        }
    }
}

optional_id BlockConfig::id() const { return _id; }
const rapidjson::Value* BlockConfig::getOrigin() const
{
    if (!_config) {
        return nullptr;
    }
    return tools::get_obj(*_config, "origin");
}

const rapidjson::Value* BlockConfig::getStateConfig(state stateCode) const
{
    if (!_config) {
        return nullptr;
    }
    auto states = tools::get_obj(*_config, "states");
    if (states)
    {
        const auto it = stateCodeToName.find(stateCode);
        if (it == stateCodeToName.end()) {
            CCLOG("BlockConfig: Missing stateCode %u", static_cast<unsigned>(stateCode));
            return nullptr;
        }

        const auto& stateName = it->second;
        auto state = tools::get_obj(*states, stateName.c_str());
        return state;
    }
    return nullptr;
}

const rapidjson::Document& BlockConfig::getConfig() const
{
    if (_config) {
        return *_config;
    }
    static rapidjson::Document emptyDoc(rapidjson::kObjectType);
    return emptyDoc;
}
BlockConfig::operator bool() const { return _id.has_value(); }

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
    const auto it = _block_config.find(id);
    if (it == _block_config.end() || !it->second) {
        CCLOG("AssetManager: Missing BlockConfig for id=%u", static_cast<unsigned>(id));
        static rapidjson::Document emptyDoc(rapidjson::kObjectType);
        static BlockConfig emptyConfig(optional_id{}, &emptyDoc);
        return emptyConfig;
    }
    return *(it->second);
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
        auto id = tools::get<std::string>(*config, "id");

        if (id.has_value()) {
            auto id_hashed = entt::hashed_string(id.value().c_str());
            _block_config[id_hashed] = new BlockConfig(optional_id(id_hashed), config);
        }
    }
}
