#ifndef __ECS_SYSTEM_OBJECTPOOL_H__
#define __ECS_SYSTEM_OBJECTPOOL_H__

#include <vector>
#include <memory>
#include <functional>
#include <cassert>
#include "cocos2d.h"

namespace ecs {

/**
 * @brief 通用对象池模板类
 * 
 * 用于预分配和复用对象，避免运行时频繁的内存分配和释放。
 * 支持池耗尽时的动态扩展。
 * 
 * Requirements: 4.1, 4.2
 * 
 * @tparam T 池化对象类型
 */
template<typename T>
class ObjectPool {
public:
    using ResetFunc = std::function<void(T*)>;
    using CreateFunc = std::function<std::unique_ptr<T>()>;

    /**
     * @brief 构造函数
     * @param createFunc 对象创建函数（可选，默认使用 new T()）
     * @param resetFunc 对象重置函数（可选，用于归还时重置对象状态）
     */
    ObjectPool(CreateFunc createFunc = nullptr, ResetFunc resetFunc = nullptr)
        : _createFunc(createFunc)
        , _resetFunc(resetFunc)
        , _activeCount(0)
        , _totalCreated(0)
    {
        if (!_createFunc) {
            _createFunc = []() { return std::make_unique<T>(); };
        }
    }

    ~ObjectPool() = default;

    // 禁止拷贝
    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;

    // 允许移动
    ObjectPool(ObjectPool&&) = default;
    ObjectPool& operator=(ObjectPool&&) = default;

    /**
     * @brief 预分配对象
     * @param count 预分配数量
     */
    void preallocate(size_t count) {
        _pool.reserve(_pool.size() + count);
        _available.reserve(_available.size() + count);

        for (size_t i = 0; i < count; ++i) {
            auto obj = _createFunc();
            T* rawPtr = obj.get();
            _pool.push_back(std::move(obj));
            _available.push_back(rawPtr);
            ++_totalCreated;
        }

        CCLOG("[ObjectPool] Preallocated %zu objects, total pool size: %zu", 
              count, _pool.size());
    }

    /**
     * @brief 从池中获取对象
     * 
     * 如果池中有可用对象，返回一个；否则动态创建新对象。
     * 
     * @return 对象指针（由池管理生命周期）
     */
    T* acquire() {
        if (_available.empty()) {
            // 池耗尽时动态扩展
            size_t expandSize = std::max(size_t(1), _pool.size() / 2);
            CCLOG("[ObjectPool] Pool exhausted, expanding by %zu objects...", expandSize);
            preallocate(expandSize);
        }

        assert(!_available.empty() && "Pool should have available objects after expansion");

        T* obj = _available.back();
        _available.pop_back();
        ++_activeCount;

        return obj;
    }

    /**
     * @brief 归还对象到池中
     * @param obj 要归还的对象指针
     */
    void release(T* obj) {
        if (!obj) {
            CCLOG("[ObjectPool] Warning: Attempting to release null object");
            return;
        }

        // 验证对象属于此池
        bool found = false;
        for (const auto& pooledObj : _pool) {
            if (pooledObj.get() == obj) {
                found = true;
                break;
            }
        }

        if (!found) {
            CCLOG("[ObjectPool] Warning: Object does not belong to this pool");
            return;
        }

        // 检查是否已经在可用列表中（防止重复归还）
        for (const auto& availObj : _available) {
            if (availObj == obj) {
                CCLOG("[ObjectPool] Warning: Object already released");
                return;
            }
        }

        // 重置对象状态
        if (_resetFunc) {
            _resetFunc(obj);
        }

        _available.push_back(obj);
        --_activeCount;
    }

    /**
     * @brief 获取当前活跃（正在使用）的对象数量
     */
    size_t getActiveCount() const {
        return _activeCount;
    }

    /**
     * @brief 获取池中可用对象数量
     */
    size_t getAvailableCount() const {
        return _available.size();
    }

    /**
     * @brief 获取池总大小（包括活跃和可用）
     */
    size_t getPoolSize() const {
        return _pool.size();
    }

    /**
     * @brief 获取总共创建的对象数量（包括动态扩展）
     */
    size_t getTotalCreated() const {
        return _totalCreated;
    }

    /**
     * @brief 清空池中所有对象
     */
    void clear() {
        _available.clear();
        _pool.clear();
        _activeCount = 0;
        _totalCreated = 0;
    }

    /**
     * @brief 收缩池大小，释放未使用的对象
     * @param keepCount 保留的最小可用对象数量
     */
    void shrink(size_t keepCount = 0) {
        while (_available.size() > keepCount && !_available.empty()) {
            T* obj = _available.back();
            _available.pop_back();

            // 从池中移除
            auto it = std::find_if(_pool.begin(), _pool.end(),
                [obj](const std::unique_ptr<T>& ptr) { return ptr.get() == obj; });
            
            if (it != _pool.end()) {
                _pool.erase(it);
            }
        }

        CCLOG("[ObjectPool] Shrunk pool, remaining size: %zu", _pool.size());
    }

private:
    std::vector<std::unique_ptr<T>> _pool;      // 所有池化对象
    std::vector<T*> _available;                  // 可用对象指针
    CreateFunc _createFunc;                      // 对象创建函数
    ResetFunc _resetFunc;                        // 对象重置函数
    size_t _activeCount;                         // 活跃对象数量
    size_t _totalCreated;                        // 总创建数量
};

} // namespace ecs

#endif // __ECS_SYSTEM_OBJECTPOOL_H__
