#include "tools.h"
#include "memory"

namespace tools {		

bool is_json(const std::string& file_name)
{
	int pos = file_name.find_last_of('.');
	return  file_name.substr(pos + 1) == "json";
}


std::vector<std::string> get_all_json(const std::string& folder_path)
{
	auto file_utils = cocos2d::FileUtils::getInstance();
	auto file_list = file_utils->listFiles(folder_path);
	std::vector<std::string> json_files;
	for (const auto& file_name : file_list) {
		if (is_json(file_name)) json_files.push_back(file_name);
	}
	return json_files;
}

std::optional<bool> get_bool(const rapidjson::Value& json, const std::string& name)
{
	return json.HasMember(name.c_str()) && json[name.c_str()].IsBool() ?
        std::make_optional(json[name.c_str()].GetBool()) :
        std::nullopt;
}

bool get_bool_or(const rapidjson::Value& json, const std::string& name, bool default_val)
{
	return json.HasMember(name.c_str()) && json[name.c_str()].IsBool() ?
        json[name.c_str()].GetBool() :
        default_val;
}

std::optional<int> get_int(const rapidjson::Value& json, const std::string& name)
{
	return json.HasMember(name.c_str()) && json[name.c_str()].IsInt() ?
		std::make_optional(json[name.c_str()].GetInt()) :
		std::nullopt;
}

int get_int_or(const rapidjson::Value& json, const std::string& name, int default_val)
{
	return json.HasMember(name.c_str()) && json[name.c_str()].IsInt() ?
		json[name.c_str()].GetInt() :
		default_val;
}

std::optional<std::string> get_str(const rapidjson::Value& json, const std::string& name)
{
	return json.HasMember(name.c_str()) && json[name.c_str()].IsString() ?
		std::make_optional(json[name.c_str()].GetString()) :
		std::nullopt;
}

std::string get_str_or(const rapidjson::Value& json, const std::string& name, const std::string& default_val)
{
	return json.HasMember(name.c_str()) && json[name.c_str()].IsString() ?
		json[name.c_str()].GetString() :
		default_val;
}

std::optional<float> get_float(const rapidjson::Value& json, const std::string& name)
{
	return json.HasMember(name.c_str()) && json[name.c_str()].IsFloat() ?
		std::make_optional(json[name.c_str()].GetFloat()) :
		std::nullopt;
}

float get_float_or(const rapidjson::Value& json, const std::string& name, float default_val)
{
	return json.HasMember(name.c_str()) && json[name.c_str()].IsFloat() ?
		json[name.c_str()].GetFloat() :
		default_val;
}

cocos2d::Vec3 MouseDebugTool::s_screenPos = cocos2d::Vec3::ZERO;
cocos2d::Vec2 MouseDebugTool::s_worldPos = cocos2d::Vec2::ZERO;
bool MouseDebugTool::s_initialized = false;

void MouseDebugTool::init(cocos2d::Node* targetNode)
{
	if (s_initialized || !targetNode) return;

	auto listener = cocos2d::EventListenerMouse::create();

	listener->onMouseMove = [](cocos2d::EventMouse* event) {
		onMouseEvent(event);
		};

	listener->onMouseDown = [](cocos2d::EventMouse* event) {
		onMouseEvent(event);
		};

	listener->onMouseUp = [](cocos2d::EventMouse* event) {
		onMouseEvent(event);
		};

	auto dispatcher = cocos2d::Director::getInstance()->getEventDispatcher();
	dispatcher->addEventListenerWithSceneGraphPriority(listener, targetNode);

	s_initialized = true;
}

cocos2d::Vec2 MouseDebugTool::getWorldPosition()
{
	return s_worldPos;
}

cocos2d::Vec2 MouseDebugTool::getScreenPosition()
{
	return { s_screenPos.x, s_screenPos.y };
}

void MouseDebugTool::update(float delta)
{

}

void MouseDebugTool::onMouseEvent(cocos2d::EventMouse* event)
{
	auto& size = cocos2d::Director::getInstance()->getWinSize();
	s_screenPos = cocos2d::Vec3(event->getCursorX(), event->getCursorY(), 0.99345f);

	auto camera = cocos2d::Camera::getVisitingCamera();
	if (!camera) camera = cocos2d::Camera::getDefaultCamera();

	if (camera)
	{
		
		cocos2d::Vec3 worldPos3D;
		camera->unprojectGL(
			size,
			&s_screenPos,
			&worldPos3D
		);
		s_worldPos = cocos2d::Vec2(worldPos3D.x, worldPos3D.y);
	}
	else
	{
		s_worldPos = { s_screenPos.x, s_screenPos.y };
	}
}

}// ÃüÃû¿Õ¼ä½áÊø