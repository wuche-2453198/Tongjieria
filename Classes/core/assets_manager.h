#pragma once
#include "cocos2d.h"
#include "entt/entt.hpp"
#include "json/document.h"
#include "utils/tools.h"

using optional_id = std::optional<entt::id_type>;
using state = uint32_t;

struct CollisionBody
{
    cocos2d::Vec2 size;
    cocos2d::Vec2 offset;
};

class BlockConfig
{
public:
    BlockConfig();
    BlockConfig(optional_id id, rapidjson::Document* config);
    optional_id id() const;
    const rapidjson::Value* getOrigin() const;
    const rapidjson::Value* getStateConfig(state stateCode) const;

    const rapidjson::Value* getValueObject(const rapidjson::Value& config, const std::string& firstTag, const std::string& secondTag)
    {
        auto first = tools::get_obj(config, firstTag);
        if (!first)
        {
            return nullptr;
        }
        return  tools::get_obj(*first, secondTag);
    }

    template <typename T>
    std::optional<T> getVal(const rapidjson::Value& config, const std::string& firstTag, const std::string& secondTag) const
    {
        auto first = tools::get_obj(config, firstTag);
        if (!first)
        {
            return std::nullopt;
        }
        return  tools::get<T>(*first, secondTag.c_str());
    }

    template <typename T>
    std::optional<T> getOriginVal(const std::string& firstTag, const std::string& secondTag) const
    {
        return getVal<T>(*getOrigin(), firstTag, secondTag);
    }

    template <typename T>
    T getOriginValOr(const std::string& firstTag, const std::string& secondTag, T defaultValue) const
    {
        auto originVal = getOriginVal<T>(firstTag, secondTag);
        if (originVal)
        {
            return originVal.value();
        }
        return defaultValue;
    }

    template <typename T>
    std::optional<T> getStateVal(const std::string& firstTag, const std::string& secondTag, state stateCode) const
    {
        auto stateConfig = getStateConfig(stateCode);
        if (stateConfig)
        {
            return getVal<T>(*stateConfig, firstTag, secondTag);
        }
        else
        {
            return std::nullopt;
        }
    }

    template <typename T>
    T getStateValOr(const std::string& firstTag, const std::string& secondTag, state stateCode, T defaultValue) const
    {
        auto stateVal = getStateVal<T>(firstTag, secondTag, stateCode);
        if(stateVal)
        {
            return stateVal.value();
        }
        return defaultValue;
    }

    template <typename T>
    std::optional<T> tryGetStateVal(const std::string& firstTag, const std::string& secondTag, state stateCode) const
    {
        auto stateConfig = getStateConfig(stateCode);
        if (stateConfig)
        {
            auto stateVal = getVal<T>(*getStateConfig(stateCode), firstTag, secondTag);
            if (stateVal)
            {
                return stateVal;
            }
        }
        return getOriginVal<T>(firstTag, secondTag);
    }

    template <typename T>
    T tryGetStateValOr(const std::string& firstTag, const std::string& secondTag, state stateCode, T defaultValue) const
    {
        auto stateVal = tryGetStateVal<T>(firstTag, secondTag, stateCode);
        if (stateVal)
        {
            return stateVal.value();
        }
        return defaultValue;
    }

    const 

    const rapidjson::Document& getConfig() const;
    operator bool() const;
private:
    optional_id _id;
    rapidjson::Document* _config = nullptr;


    std::unordered_map<state, std::string> stateCodeToName;
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
