#include "ECSSystem.h"
#include "systems/save/SaveData.h"

// 初始化静态成员变量
ECSSystem* ECSSystem::s_instance = nullptr;

// ==================== 单例模式 ====================
ECSSystem* ECSSystem::getInstance() {
    if (s_instance == nullptr) {
        s_instance = new ECSSystem();
    }
    return s_instance;
}

void ECSSystem::destroyInstance() {
    if (s_instance != nullptr) {
        delete s_instance;
        s_instance = nullptr;
    }
}

// ==================== 构造函数与析构函数 ====================
ECSSystem::ECSSystem()
    : m_playerEntity(entt::null)
    , m_nextEntityId(1) {
}

ECSSystem::~ECSSystem() {
    cleanup();
}

// ==================== 初始化与清理 ====================
void ECSSystem::init() {
    // 清空注册表
    m_registry.clear();

    // 重置ID映射
    m_entityToIdMap.clear();
    m_idToEntityMap.clear();
    m_nextEntityId = 1;

    // 重置玩家实体
    m_playerEntity = entt::null;

    // TODO: 初始化其他必要的系统
}

void ECSSystem::cleanup() {
    // 销毁所有实体
    auto view = m_registry.view<entt::entity>();
    for (auto entity : view) {
        destroyEntity(entity);
    }

    // 清空注册表
    m_registry.clear();

    // 清空ID映射
    m_entityToIdMap.clear();
    m_idToEntityMap.clear();
    m_nextEntityId = 1;

    m_playerEntity = entt::null;
}

// ==================== 实体管理 ====================
entt::entity ECSSystem::createPlayer(float x, float y) {
    // 创建实体
    m_playerEntity = createEntity();

    // TODO: 为玩家实体添加必要的组件
    // 例如: addComponent<Transform>(m_playerEntity, x, y);
    //       addComponent<PlayerTag>(m_playerEntity);

    return m_playerEntity;
}

entt::entity ECSSystem::createEntity() {
    // 创建新实体
    entt::entity entity = m_registry.create();

    // 分配ID
    int entityId = m_nextEntityId++;
    m_entityToIdMap[entity] = entityId;
    m_idToEntityMap[entityId] = entity;

    return entity;
}

void ECSSystem::destroyEntity(entt::entity entity) {
    if (!m_registry.valid(entity)) {
        return;
    }

    // 如果是玩家实体，重置引用
    if (entity == m_playerEntity) {
        m_playerEntity = entt::null;
    }

    // 从ID映射中移除
    auto it = m_entityToIdMap.find(entity);
    if (it != m_entityToIdMap.end()) {
        int entityId = it->second;
        m_idToEntityMap.erase(entityId);
        m_entityToIdMap.erase(it);
    }

    // 销毁实体
    m_registry.destroy(entity);
}

// ==================== 存档系统对接 ====================
SimpleSaveData ECSSystem::serializeWorld() {
    SimpleSaveData saveData;

    // TODO: 实现序列化整个游戏世界的逻辑
    // 1. 遍历所有实体
    // 2. 序列化每个实体及其组件
    // 3. 保存到 SimpleSaveData

    return saveData;
}

bool ECSSystem::deserializeWorld(const SimpleSaveData& data) {
    // 先清理当前世界
    cleanup();

    // TODO: 实现反序列化整个游戏世界的逻辑
    // 1. 从 data 读取实体数据
    // 2. 创建实体并添加组件
    // 3. 恢复实体间的关系

    return true;
}

SimpleSaveData ECSSystem::getCurrentGameState() {
    // 直接使用 serializeWorld 的实现
    return serializeWorld();
}

SimpleSaveData ECSSystem::serializeEntity(entt::entity entity) {
    SimpleSaveData saveData;

    if (!m_registry.valid(entity)) {
        return saveData;
    }

    // TODO: 实现序列化单个实体的逻辑
    // 1. 获取实体的ID
    // 2. 序列化实体上的所有组件
    // 3. 保存到 SimpleSaveData

    return saveData;
}

entt::entity ECSSystem::createEntityFromSave(const SimpleSaveData& data) {
    // TODO: 实现从存档数据创建实体的逻辑
    // 1. 从 data 读取实体ID和组件数据
    // 2. 创建实体
    // 3. 添加并初始化组件

    return createEntity();  // 暂时返回新创建的实体
}

// ==================== 游戏逻辑更新 ====================
void ECSSystem::update(float deltaTime) {
    // TODO: 实现游戏逻辑更新
    // 例如：
    // 1. 更新物理系统
    // 2. 更新动画系统
    // 3. 处理输入
    // 4. 更新AI等

    // 注意：这里只是示例，实际实现需要根据具体的组件和系统来编写
}

// ==================== 实体查询 ====================
std::vector<entt::entity> ECSSystem::getEntitiesByTag(const std::string& tag) {
    std::vector<entt::entity> result;

    // TODO: 实现按标签查询实体的逻辑
    // 假设有 TagComponent 组件
    // auto view = m_registry.view<TagComponent>();
    // for (auto entity : view) {
    //     auto& tagComp = m_registry.get<TagComponent>(entity);
    //     if (tagComp.tag == tag) {
    //         result.push_back(entity);
    //     }
    // }

    return result;
}

std::vector<entt::entity> ECSSystem::getEntitiesNearPosition(float x, float y, float radius) {
    std::vector<entt::entity> result;

    // TODO: 实现按位置查询实体的逻辑
    // 假设有 TransformComponent 组件
    // auto view = m_registry.view<TransformComponent>();
    // for (auto entity : view) {
    //     auto& transform = m_registry.get<TransformComponent>(entity);
    //     float dx = transform.x - x;
    //     float dy = transform.y - y;
    //     float distanceSq = dx * dx + dy * dy;
    //     
    //     if (distanceSq <= radius * radius) {
    //         result.push_back(entity);
    //     }
    // }

    return result;
}