#pragma once
#include <unordered_map>
#include <string>
#include <optional>
#include "cocos2d.h"
#include "json/rapidjson.h"
#include "json/document.h"

namespace tools {

template<typename Derived>
class Singleton {
public:
	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;
	Singleton(Singleton&&) = delete;
	Singleton& operator=(Singleton&&) = delete;
	virtual bool init() {
		return true;
	}
	static Derived* getInstance() {
		if (!_instance)
		{
			_instance = new Derived();
			bool isInit = _instance->init();
			CCASSERT(isInit, "Derived failed at init");
		}
		return _instance;
	}

protected:
	Singleton() = default;
	virtual ~Singleton() {
		delete _instance;
	};
	static inline Derived* _instance = nullptr;
};

bool is_json(const std::string& file_name);

std::vector<std::string> get_all_json(const std::string& folder_path);

template<typename T>
T get_or(const rapidjson::Value& config, const char* tag, T default_value) {
    // 检查tag是否存在
    if (!config.HasMember(tag)) {
        return default_value;
    }

    const auto& value = config[tag];

    // 根据类型判断
    if constexpr (std::is_same_v<T, bool>) 
    {
        if (value.IsBool()) 
        {
            return value.GetBool();
        }
    }
    else if constexpr (std::is_same_v<T, int>) 
    {
        if (value.IsInt()) 
        {
            return value.GetInt();
        }
    }
    else if constexpr (std::is_same_v<T, float>) 
    {
        if (value.IsFloat()) 
        {
            return value.GetFloat();
        }
    }
    else if constexpr (std::is_same_v<T, std::string>) 
    {
        if (value.IsString()) 
        {
            return std::string(value.GetString(), value.GetStringLength());
        }
    }
    else if constexpr (std::is_same_v<T, const char*>)
    {
        if (value.IsString())
        {
            return std::string(value.GetString());
        }
    }
    return default_value;
}

template<typename T>
std::optional<T> get(const rapidjson::Value& config, const std::string& tag) {
    // 检查tag是否存在
    if (!config.HasMember(tag.c_str())) 
    {
        return std::nullopt;
    }

    const auto& value = config[tag.c_str()];

    // 根据类型判断
    if constexpr (std::is_same_v<T, bool>)
    {
        if (value.IsBool())
        {
            return std::make_optional(value.GetBool());
        }
    }
    else if constexpr (std::is_same_v<T, int>)
    {
        if (value.IsInt())
        {
            return value.GetInt();
        }
    }
    else if constexpr (std::is_same_v<T, float>)
    {
        if (value.IsFloat())
        {
            return value.GetFloat();
        }
    }
    else if constexpr (std::is_same_v<T, std::string>)
    {
        if (value.IsString())
        {
            return std::string(value.GetString());
        }
    }
    return std::nullopt;
}

const rapidjson::Value* get_array(const rapidjson::Value& config, const std::string& tag);
const rapidjson::Value* get_obj(const rapidjson::Value& config, const std::string& tag);

class MouseDebugTool
{
public:
    /**
     * 必须在场景初始化时调用
     * @param targetNode 监听器绑定的节点（通常为当前Layer）
     */
    static void init(cocos2d::Node* targetNode);

    /**
     * 获取鼠标在世界空间中的位置
     */
    static cocos2d::Vec2 getWorldPosition();

    /**
     * 获取鼠标在屏幕空间中的位置
     */
    static cocos2d::Vec2 getScreenPosition();

	void update(float delta);
private:
    MouseDebugTool() = delete;

    static void onMouseEvent(cocos2d::EventMouse* event);

    static cocos2d::Vec3 s_screenPos;
    static cocos2d::Vec2 s_worldPos;
    static bool s_initialized;
};

}// end of name space