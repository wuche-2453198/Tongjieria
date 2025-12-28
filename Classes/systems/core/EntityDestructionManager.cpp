#include "EntityDestructionManager.h"
#include "cocos2d.h"

namespace ecs {

void EntityDestructionManager::queueDestruction(entt::registry& registry, entt::entity entity) {
    // 验证实体有效性
    if (!registry.valid(entity)) {
        CCLOG("[EntityDestructionManager] Warning: Attempting to queue invalid entity for destruction");
        return;
    }
    
    // 使用 unordered_set 自动去重
    // insert 返回 pair<iterator, bool>，bool 表示是否成功插入（即是否是新元素）
    auto result = _destructionQueue.insert(entity);
    
    if (result.second) {
        CCLOG("[EntityDestructionManager] Entity %u queued for destruction, queue size: %zu",
              static_cast<uint32_t>(entity), _destructionQueue.size());
    }
    // 如果已存在，静默忽略（Requirements 1.3: 自动去重）
}

bool EntityDestructionManager::isPendingDestruction(entt::entity entity) const {
    return _destructionQueue.find(entity) != _destructionQueue.end();
}

void EntityDestructionManager::processQueue(entt::registry& registry) {
    if (_destructionQueue.empty()) {
        return;
    }
    
    size_t destroyedCount = 0;
    size_t skippedCount = 0;
    
    // 遍历队列，验证并销毁实体
    for (auto entity : _destructionQueue) {
        // Requirements 1.4: 验证实体有效性后再销毁
        if (registry.valid(entity)) {
            registry.destroy(entity);
            ++destroyedCount;
        } else {
            // 实体已被其他方式销毁，跳过
            ++skippedCount;
        }
    }
    
    CCLOG("[EntityDestructionManager] Processed destruction queue: %zu destroyed, %zu skipped",
          destroyedCount, skippedCount);
    
    // 清空队列
    _destructionQueue.clear();
}

void EntityDestructionManager::clear() {
    size_t previousSize = _destructionQueue.size();
    _destructionQueue.clear();
    
    if (previousSize > 0) {
        CCLOG("[EntityDestructionManager] Queue cleared, %zu pending entities discarded", previousSize);
    }
}

} // namespace ecs
