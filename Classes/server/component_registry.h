#include "cocos2d.h"
#include "json/rapidjson.h"
#include "json/document.h"
#include "tools.h"
#include "entt/entt.hpp"
#pragma once

template<typename T>
concept RegistrableComponent = requires(T t, const rapidjson::Value & config) {
    { T::getID() } -> std::same_as<const std::string&>;
    { T::createDefault() } -> std::same_as<T>;
    { T::createFromConfig(config) } -> std::same_as<T>;
};

using CreateTo = std::function<void(entt::registry&, entt::entity)>;
using CreateToFromConfig = std::function<void(entt::registry&, entt::entity, const rapidjson::Value&)>;

template<RegistrableComponent T>
class ComponentAutoRegister;

/*
* @brief 组件注册工厂。
* 
* 组件通过 AutoRegister 在静态时期提交生成必要的元数据，并将它们跟一个id绑定在一起。
*/
class ComponentRegistry {
public:
    template<RegistrableComponent T>
    friend class ComponentAutoRegister;
    friend class tools::Singleton<ComponentRegistry>;

    /*
    * @brief 创建空组件到对应实体上
    */
    static void createTo(entt::registry& registry, const std::string& id);

    /*
    * @brief 创建空组件到对应实体上
    */
    static void createTo(entt::registry& registry, entt::id_type id);

    /*
    * @brief 创建已配置的组件到对应实体上
    */
    static void createToFromConfig(entt::registry& registry, const std::string& id, const rapidjson::Value&);

    /*
    * @brief 创建已配置的组件到对应实体上
    */
    static void createToFromConfig(entt::registry& registry, entt::id_type id, const rapidjson::Value&);

private:
    ComponentRegistry() = default;
    entt::resource_cache<CreateTo> _defaultMethods;
    entt::resource_cache<CreateToFromConfig> _configuredMethods;
};

/*
* @brief 自动组件注册。
* AutoRegister在其构造函数中注册组件工厂。
* 通过Regist概念来约束模板，传入类型名来获取元数据。
*/
template<RegistrableComponent ComponentType>
class ComponentAutoRegister {
public:
    ComponentAutoRegister(const ComponentAutoRegister&) = delete;
    ComponentAutoRegister& operator=(const ComponentAutoRegister&) = delete;

    /*
    * @brief 在构造函数中通过概念定义的接口来注册组件。
    */
    ComponentAutoRegister() {
        auto component = ComponentType::createDefault();
        CreateTo createTo =
            [component](
                entt::registry& registry,
                entt::entity entity)
            {
                registry.emplace<ComponentType>(entity, component);
            };
        ComponentRegistry::getInstance()->
            _defaultMethods.load(tools::str_to_id(ComponentType::getID()), createTo);
    }
};