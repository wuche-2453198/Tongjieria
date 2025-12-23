#include <unordered_map>
#include <string>
#include <optional>
#include "cocos2d.h"
#include "json/rapidjson.h"
#include "json/document.h"
#pragma once

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
std::optional<bool> get_bool(const rapidjson::Value& json, const std::string& name);
bool get_bool_or(const rapidjson::Value& json, const std::string& name, bool default_val);
std::optional<int> get_int(const rapidjson::Value& json, const std::string& name);
int get_int_or(const rapidjson::Value& json, const std::string& name, int default_val);
std::optional<std::string> get_str(const rapidjson::Value& json, const std::string& name);
std::string get_str_or(const rapidjson::Value& json, const std::string& name, const std::string& default_val);
std::optional<float> get_float(const rapidjson::Value& json, const std::string& name);
float get_float_or(const rapidjson::Value& json, const std::string& name, float default_val);
std::optional<int> get_val(const rapidjson::Value& json, const std::string& name);
std::optional<int> get_obj(const rapidjson::Value& json, const std::string& name);
std::optional<int> get_list(const rapidjson::Value& json, const std::string& name);

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