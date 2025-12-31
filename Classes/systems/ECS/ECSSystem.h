#ifndef __ECS_SYSTEM_H__
#define __ECS_SYSTEM_H__

#include "cocos2d.h"
#include <entt/entt.hpp>
#include <string>
#include <vector>
#include <map>

// 前向声明
struct SimpleSaveData;

class ECSSystem {
public:
    // ==================== 单例模式 ====================
    static ECSSystem* getInstance();
    static void destroyInstance();

    // ==================== 初始化与清理 ====================
    void init();
    void cleanup();

    // ==================== 实体管理 ====================
    entt::entity createPlayer(float x, float y);
    entt::entity createEntity();
    void destroyEntity(entt::entity entity);

    // 获取注册表（供其他系统直接操作）
    entt::registry& getRegistry() { return m_registry; }

    // 获取玩家实体
    entt::entity getPlayerEntity() const { return m_playerEntity; }

    // ==================== 组件管理 ====================
    template<typename Component, typename... Args>
    Component& addComponent(entt::entity entity, Args&&... args) {
        return m_registry.emplace<Component>(entity, std::forward<Args>(args)...);
    }

    template<typename Component>
    bool hasComponent(entt::entity entity) const {
        return m_registry.valid(entity) && m_registry.all_of<Component>(entity);
    }

    template<typename Component>
    Component& getComponent(entt::entity entity) {
        return m_registry.get<Component>(entity);
    }

    template<typename Component>
    Component* tryGetComponent(entt::entity entity) {
        return m_registry.try_get<Component>(entity);
    }

    template<typename Component>
    void removeComponent(entt::entity entity) {
        m_registry.remove<Component>(entity);
    }

    // ==================== 存档系统对接 ====================
    // 序列化整个游戏世界到存档数据
    SimpleSaveData serializeWorld();

    // 从存档数据反序列化游戏世界
    bool deserializeWorld(const SimpleSaveData& data);

    // 获取当前游戏状态（用于快速保存）
    SimpleSaveData getCurrentGameState();

    // 序列化单个实体（主要用于玩家）
    SimpleSaveData serializeEntity(entt::entity entity);

    // 从存档数据创建实体
    entt::entity createEntityFromSave(const SimpleSaveData& data);

    // ==================== 游戏逻辑更新 ====================
    void update(float deltaTime);

    // ==================== 实体查询 ====================
    std::vector<entt::entity> getEntitiesByTag(const std::string& tag);
    std::vector<entt::entity> getEntitiesNearPosition(float x, float y, float radius);

private:
    // ==================== 私有构造函数 ====================
    ECSSystem();
    ~ECSSystem();

    // 禁止拷贝
    ECSSystem(const ECSSystem&) = delete;
    ECSSystem& operator=(const ECSSystem&) = delete;

private:
    static ECSSystem* s_instance;

    entt::registry m_registry;          // ECS 注册表
    entt::entity m_playerEntity;        // 玩家实体

    // 实体ID管理
    std::map<entt::entity, int> m_entityToIdMap;    // 实体到ID的映射
    std::map<int, entt::entity> m_idToEntityMap;    // ID到实体的映射
    int m_nextEntityId = 1;                         // 下一个可用的实体ID
};

#endif // __ECS_SYSTEM_H__