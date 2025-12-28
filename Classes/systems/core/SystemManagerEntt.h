#ifndef __ECS_SYSTEM_SYSTEMMANAGERENTT_H__
#define __ECS_SYSTEM_SYSTEMMANAGERENTT_H__

#include "ISystemEntt.h"
#include "EntityDestructionManager.h"
#include <entt/entt.hpp>
#include <chrono>
#include <cstdint>
#include <vector>
#include <memory>
#include <algorithm>
#include <string>
#include <unordered_map>
#include "cocos2d.h"

namespace ecs {

/**
 * @brief System管理器
 * 
 * 用法：
 * SystemManagerEntt manager;
 * manager.setRegistry(&registry);
 * manager.addSystem<HealthSystemEntt>();
 * manager.update(delta);
 * 
 * 调试功能：
 * manager.setDebugLogging(true);  // 启用执行顺序日志
 * manager.logSystemOrder();       // 打印当前系统执行顺序
 */
class SystemManagerEntt {
private:
    entt::registry* _registry = nullptr;
    std::vector<std::unique_ptr<ISystemEntt>> _systems;
    bool _debugLogging = false;
    int _frameCounter = 0;
    int _logInterval = 60;  // 每60帧打印一次执行日志（约1秒）

    struct PerfRecord {
        std::string name;
        double totalMs = 0.0;
        double maxMs = 0.0;
        std::uint64_t calls = 0;
    };

    bool _perfEnabled = false;
    float _perfReportIntervalSeconds = 1.0f;
    size_t _perfTopN = 8;
    float _perfTimerSeconds = 0.0f;
    std::unordered_map<const ISystemEntt*, PerfRecord> _perfRecords;
    PerfRecord _destructionPerf;

public:
    /**
     * @brief 设置Registry
     */
    void setRegistry(entt::registry* registry) {
        _registry = registry;
        
        // 更新所有已注册System的Registry
        for (auto& system : _systems) {
            system->setRegistry(_registry);
        }
    }

    /**
     * @brief 添加System
     */
    template<typename T, typename... Args>
    T* addSystem(Args&&... args) {
        static_assert(std::is_base_of<ISystemEntt, T>::value, 
                      "T must derive from ISystemEntt");
        
        auto system = std::make_unique<T>(std::forward<Args>(args)...);
        system->setRegistry(_registry);
        
        T* ptr = system.get();
        _systems.push_back(std::move(system));
        
        // 按优先级排序
        sortSystems();
        
        if (_debugLogging) {
            CCLOG("SystemManager: Added system '%s' with priority %d", 
                  ptr->getName(), ptr->getPriority());
        }
        
        return ptr;
    }

    void setPerformanceProfiling(bool enabled, float reportIntervalSeconds = 1.0f, size_t topN = 8) {
        _perfEnabled = enabled;
        _perfReportIntervalSeconds = reportIntervalSeconds > 0.0f ? reportIntervalSeconds : 1.0f;
        _perfTopN = topN > 0 ? topN : 8;
        _perfTimerSeconds = 0.0f;
        _perfRecords.clear();
        _destructionPerf = PerfRecord{};
        _destructionPerf.name = "EntityDestructionManager";
    }

    bool isPerformanceProfilingEnabled() const {
        return _perfEnabled;
    }

    /**
     * @brief 更新所有System
     */
    void update(float delta) {
        _frameCounter++;
        
        // 定期打印执行顺序日志（仅在调试模式下）
        if (_debugLogging && (_frameCounter % _logInterval == 0)) {
            CCLOG("=== SystemManager: Frame %d - Executing %zu systems ===", 
                  _frameCounter, _systems.size());
        }
        
        for (size_t i = 0; i < _systems.size(); ++i) {
            auto& system = _systems[i];
            
            if (_debugLogging && (_frameCounter % _logInterval == 0)) {
                CCLOG("  [%zu] %s (priority: %d)", 
                      i, system->getName(), system->getPriority());
            }
            
            if (_perfEnabled) {
                using Clock = std::chrono::steady_clock;
                auto start = Clock::now();
                system->update(delta);
                auto end = Clock::now();
                double ms = std::chrono::duration<double, std::milli>(end - start).count();

                auto* key = system.get();
                auto& rec = _perfRecords[key];
                if (rec.name.empty()) {
                    rec.name = system->getName();
                }
                rec.totalMs += ms;
                rec.calls += 1;
                if (ms > rec.maxMs) {
                    rec.maxMs = ms;
                }
            } else {
                system->update(delta);
            }
        }
        
        // 处理延迟销毁队列 - 在所有系统更新完成后统一销毁实体
        // Requirements: 1.2 - WHEN all systems have completed their update cycle, 
        // THE EntityDestructionManager SHALL process the destruction queue
        if (_registry) {
            if (_perfEnabled) {
                using Clock = std::chrono::steady_clock;
                auto start = Clock::now();
                EntityDestructionManager::getInstance().processQueue(*_registry);
                auto end = Clock::now();
                double ms = std::chrono::duration<double, std::milli>(end - start).count();
                _destructionPerf.totalMs += ms;
                _destructionPerf.calls += 1;
                if (ms > _destructionPerf.maxMs) {
                    _destructionPerf.maxMs = ms;
                }
            } else {
                EntityDestructionManager::getInstance().processQueue(*_registry);
            }
        }

        if (_perfEnabled) {
            _perfTimerSeconds += delta;
            if (_perfTimerSeconds >= _perfReportIntervalSeconds) {
                struct Row {
                    const PerfRecord* rec;
                };
                std::vector<Row> rows;
                rows.reserve(_perfRecords.size() + 1);
                for (const auto& kv : _perfRecords) {
                    rows.push_back(Row{ &kv.second });
                }
                rows.push_back(Row{ &_destructionPerf });

                std::sort(rows.begin(), rows.end(), [](const Row& a, const Row& b) {
                    return a.rec->totalMs > b.rec->totalMs;
                });

                cocos2d::log("=== System Perf Top %zu (interval=%.2fs) ===", _perfTopN, _perfTimerSeconds);
                size_t printed = 0;
                for (const auto& row : rows) {
                    if (printed >= _perfTopN) {
                        break;
                    }
                    if (row.rec->calls == 0) {
                        continue;
                    }
                    double avgMs = row.rec->totalMs / static_cast<double>(row.rec->calls);
                    cocos2d::log("  %s: total=%.3fms calls=%llu avg=%.3fms max=%.3fms",
                        row.rec->name.c_str(),
                        row.rec->totalMs,
                        static_cast<unsigned long long>(row.rec->calls),
                        avgMs,
                        row.rec->maxMs);
                    printed++;
                }

                for (auto& kv : _perfRecords) {
                    kv.second.totalMs = 0.0;
                    kv.second.maxMs = 0.0;
                    kv.second.calls = 0;
                }
                _destructionPerf.totalMs = 0.0;
                _destructionPerf.maxMs = 0.0;
                _destructionPerf.calls = 0;
                _perfTimerSeconds = 0.0f;
            }
        }
    }

    /**
     * @brief 获取System数量
     */
    size_t getSystemCount() const {
        return _systems.size();
    }

    /**
     * @brief 清空所有System
     */
    void clear() {
        _systems.clear();
        
        // 清空延迟销毁队列（场景切换时）
        EntityDestructionManager::getInstance().clear();
    }
    
    /**
     * @brief 启用/禁用调试日志
     * @param enabled 是否启用
     */
    void setDebugLogging(bool enabled) {
        _debugLogging = enabled;
        if (enabled) {
            CCLOG("SystemManager: Debug logging ENABLED");
            logSystemOrder();
        } else {
            CCLOG("SystemManager: Debug logging DISABLED");
        }
    }
    
    /**
     * @brief 设置日志打印间隔（帧数）
     * @param interval 间隔帧数，默认60帧
     */
    void setLogInterval(int interval) {
        _logInterval = interval > 0 ? interval : 60;
    }
    
    /**
     * @brief 打印当前系统执行顺序
     */
    void logSystemOrder() const {
        CCLOG("=== SystemManager: Current System Execution Order ===");
        CCLOG("Total systems: %zu", _systems.size());
        
        int lastPriority = -1;
        bool orderValid = true;
        
        for (size_t i = 0; i < _systems.size(); ++i) {
            const auto& system = _systems[i];
            int priority = system->getPriority();
            
            // 验证优先级顺序
            if (priority < lastPriority) {
                CCLOG("  [%zu] %s (priority: %d) *** ORDER ERROR ***", 
                      i, system->getName(), priority);
                orderValid = false;
            } else {
                CCLOG("  [%zu] %s (priority: %d)", 
                      i, system->getName(), priority);
            }
            
            lastPriority = priority;
        }
        
        if (orderValid) {
            CCLOG("=== System execution order is VALID ===");
        } else {
            CCLOG("=== WARNING: System execution order has ERRORS ===");
        }
    }
    
    /**
     * @brief 验证系统执行顺序是否正确
     * @return true 如果所有系统按优先级升序排列
     */
    bool verifySystemOrder() const {
        int lastPriority = -1;
        
        for (const auto& system : _systems) {
            int priority = system->getPriority();
            if (priority < lastPriority) {
                return false;
            }
            lastPriority = priority;
        }
        
        return true;
    }
    
    /**
     * @brief 获取系统优先级列表（用于测试）
     * @return 按执行顺序排列的优先级列表
     */
    std::vector<int> getSystemPriorities() const {
        std::vector<int> priorities;
        priorities.reserve(_systems.size());
        
        for (const auto& system : _systems) {
            priorities.push_back(system->getPriority());
        }
        
        return priorities;
    }

private:
    void sortSystems() {
        std::sort(_systems.begin(), _systems.end(),
            [](const auto& a, const auto& b) {
                return a->getPriority() < b->getPriority();
            });
    }
};

} // namespace ecs

#endif // __ECS_SYSTEM_SYSTEMMANAGERENTT_H__
