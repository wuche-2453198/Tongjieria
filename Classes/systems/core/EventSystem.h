#ifndef __ECS_SYSTEM_EVENTSYSTEM_H__
#define __ECS_SYSTEM_EVENTSYSTEM_H__

#include <entt/entt.hpp>
#include <vector>
#include <functional>
#include <memory>
#include <unordered_map>
#include <queue>
#include <typeindex>
#include "cocos2d.h"
#include "ObjectPool.h"

namespace ecs {

/**
 * @brief 事件优先级枚举
 * 
 * 用于控制事件处理器的执行顺序。
 * 高优先级事件先于低优先级事件处理。
 * 
 * Requirements: 9.4
 */
enum class EventPriority {
    HIGH = 0,    // 高优先级，最先处理
    NORMAL = 1,  // 普通优先级
    LOW = 2      // 低优先级，最后处理
};

/**
 * @brief 事件包装器基类
 * 
 * 用于类型擦除，支持延迟事件队列
 */
struct IEventWrapper {
    virtual ~IEventWrapper() = default;
    virtual void dispatch(entt::dispatcher& dispatcher) = 0;
    virtual void returnToPool() = 0;
};

/**
 * @brief 事件包装器模板类
 * 
 * 包装具体事件类型，支持延迟处理和池化
 */
template<typename Event>
struct EventWrapper : public IEventWrapper {
    Event event;
    std::function<void(Event*)> poolReturnFunc;
    
    EventWrapper() = default;
    explicit EventWrapper(const Event& e) : event(e) {}
    explicit EventWrapper(Event&& e) : event(std::move(e)) {}
    
    void dispatch(entt::dispatcher& dispatcher) override {
        dispatcher.trigger(event);
    }
    
    void returnToPool() override {
        if (poolReturnFunc) {
            poolReturnFunc(&event);
        }
    }
};

/**
 * @brief 优先级事件包装器
 * 
 * 用于优先级队列排序
 */
struct PrioritizedEvent {
    EventPriority priority;
    std::unique_ptr<IEventWrapper> wrapper;
    int64_t sequenceNumber;  // 用于保持同优先级事件的FIFO顺序
    
    bool operator<(const PrioritizedEvent& other) const {
        // 优先级队列是最大堆，所以我们反转比较
        // 优先级数值小的应该先处理
        if (priority != other.priority) {
            return static_cast<int>(priority) > static_cast<int>(other.priority);
        }
        // 同优先级按序列号排序（先入先出）
        return sequenceNumber > other.sequenceNumber;
    }
};


/**
 * @brief 事件系统 - 基于 EnTT dispatcher 的事件系统
 * 
 * 提供以下功能：
 * - 基于 EnTT dispatcher 的事件发布/订阅
 * - 事件优先级支持
 * - 延迟事件处理（帧边界处理）
 * - 频繁事件的对象池化
 * 
 * Requirements: 9.1, 9.3, 9.4, 9.6
 */
class EventSystem {
public:
    /**
     * @brief 获取单例实例
     */
    static EventSystem& getInstance() {
        static EventSystem instance;
        return instance;
    }

    // 禁止拷贝和移动
    EventSystem(const EventSystem&) = delete;
    EventSystem& operator=(const EventSystem&) = delete;
    EventSystem(EventSystem&&) = delete;
    EventSystem& operator=(EventSystem&&) = delete;

    /**
     * @brief 订阅事件
     * 
     * 注册一个事件处理器。处理器函数签名为 void(const Event&)
     * 
     * @tparam Event 事件类型
     * @param handler 事件处理函数
     * @param priority 处理器优先级（目前用于延迟事件排序）
     * 
     * Requirements: 9.1
     */
    template<typename Event>
    void subscribe(std::function<void(const Event&)> handler, 
                   EventPriority priority = EventPriority::NORMAL) {
        // 使用 EnTT sink 注册处理器
        // 注意：EnTT dispatcher 本身不支持优先级，优先级主要用于延迟事件
        _dispatcher.sink<Event>().template connect<&EventSystem::handleEvent<Event>>(*this);
        
        // 存储处理器和优先级
        auto& handlers = _handlers[std::type_index(typeid(Event))];
        handlers.push_back({
            [handler](const void* event) {
                handler(*static_cast<const Event*>(event));
            },
            priority
        });
        
        // 按优先级排序
        std::sort(handlers.begin(), handlers.end(),
            [](const HandlerInfo& a, const HandlerInfo& b) {
                return static_cast<int>(a.priority) < static_cast<int>(b.priority);
            });
    }

    /**
     * @brief 订阅事件（成员函数版本）
     * 
     * @tparam Event 事件类型
     * @tparam Instance 实例类型
     * @param instance 实例指针
     * @param handler 成员函数指针
     */
    template<typename Event, typename Instance>
    void subscribe(Instance* instance, void(Instance::*handler)(const Event&)) {
        _dispatcher.sink<Event>().template connect<handler>(*instance);
    }

    /**
     * @brief 取消订阅事件
     * 
     * @tparam Event 事件类型
     * @tparam Instance 实例类型
     * @param instance 实例指针
     */
    template<typename Event, typename Instance>
    void unsubscribe(Instance* instance) {
        _dispatcher.sink<Event>().disconnect(instance);
    }

    /**
     * @brief 立即发送事件
     * 
     * 事件会立即被所有订阅者处理
     * 
     * @tparam Event 事件类型
     * @param event 事件实例
     * 
     * Requirements: 9.1
     */
    template<typename Event>
    void emit(const Event& event) {
        _dispatcher.trigger(event);
    }

    /**
     * @brief 立即发送事件（右值版本）
     */
    template<typename Event>
    void emit(Event&& event) {
        _dispatcher.trigger(std::forward<Event>(event));
    }

    /**
     * @brief 延迟发送事件
     * 
     * 事件会被加入队列，在帧边界调用 processDeferred() 时处理
     * 
     * @tparam Event 事件类型
     * @param event 事件实例
     * @param priority 事件优先级
     * 
     * Requirements: 9.6
     */
    template<typename Event>
    void emitDeferred(const Event& event, EventPriority priority = EventPriority::NORMAL) {
        auto wrapper = std::make_unique<EventWrapper<Event>>(event);
        _deferredEvents.push({
            priority,
            std::move(wrapper),
            _sequenceCounter++
        });
    }

    /**
     * @brief 延迟发送事件（右值版本）
     */
    template<typename Event>
    void emitDeferred(Event&& event, EventPriority priority = EventPriority::NORMAL) {
        auto wrapper = std::make_unique<EventWrapper<Event>>(std::forward<Event>(event));
        _deferredEvents.push({
            priority,
            std::move(wrapper),
            _sequenceCounter++
        });
    }

    /**
     * @brief 处理所有延迟事件
     * 
     * 应在帧边界调用此方法处理所有排队的延迟事件。
     * 事件按优先级顺序处理。
     * 
     * Requirements: 9.6
     */
    void processDeferred() {
        while (!_deferredEvents.empty()) {
            auto event = std::move(const_cast<PrioritizedEvent&>(_deferredEvents.top()));
            _deferredEvents.pop();
            
            if (event.wrapper) {
                event.wrapper->dispatch(_dispatcher);
                event.wrapper->returnToPool();
            }
        }
        
        // 重置序列计数器（防止溢出）
        if (_deferredEvents.empty()) {
            _sequenceCounter = 0;
        }
    }

    /**
     * @brief 获取延迟事件队列大小
     */
    size_t getDeferredCount() const {
        return _deferredEvents.size();
    }

    /**
     * @brief 清空所有延迟事件
     */
    void clearDeferred() {
        while (!_deferredEvents.empty()) {
            _deferredEvents.pop();
        }
        _sequenceCounter = 0;
    }


    // ==================== 事件池化功能 ====================
    
    /**
     * @brief 注册频繁事件类型的对象池
     * 
     * 为频繁发生的事件类型创建对象池，避免频繁的内存分配
     * 
     * @tparam Event 事件类型
     * @param initialSize 初始池大小
     * 
     * Requirements: 9.3
     */
    template<typename Event>
    void registerEventPool(size_t initialSize = 32) {
        auto typeIndex = std::type_index(typeid(Event));
        
        if (_eventPools.find(typeIndex) != _eventPools.end()) {
            CCLOG("[EventSystem] Event pool for type already registered");
            return;
        }
        
        auto poolWrapper = std::make_unique<PoolWrapper<Event>>(
            []() { return std::make_unique<Event>(); },
            [](Event* e) { *e = Event{}; }  // 重置为默认状态
        );
        poolWrapper->pool.preallocate(initialSize);
        
        _eventPools[typeIndex] = std::move(poolWrapper);
        
        CCLOG("[EventSystem] Registered event pool for type, initial size: %zu", initialSize);
    }

    /**
     * @brief 从池中获取事件对象
     * 
     * @tparam Event 事件类型
     * @return 事件对象指针，如果没有注册池则返回 nullptr
     * 
     * Requirements: 9.3
     */
    template<typename Event>
    Event* acquireEvent() {
        auto typeIndex = std::type_index(typeid(Event));
        auto it = _eventPools.find(typeIndex);
        
        if (it == _eventPools.end()) {
            CCLOG("[EventSystem] Warning: No pool registered for event type, creating new object");
            return new Event();
        }
        
        auto* poolWrapper = static_cast<PoolWrapper<Event>*>(it->second.get());
        return poolWrapper->pool.acquire();
    }

    /**
     * @brief 归还事件对象到池中
     * 
     * @tparam Event 事件类型
     * @param event 事件对象指针
     * 
     * Requirements: 9.3
     */
    template<typename Event>
    void releaseEvent(Event* event) {
        if (!event) return;
        
        auto typeIndex = std::type_index(typeid(Event));
        auto it = _eventPools.find(typeIndex);
        
        if (it == _eventPools.end()) {
            CCLOG("[EventSystem] Warning: No pool registered for event type, deleting object");
            delete event;
            return;
        }
        
        auto* poolWrapper = static_cast<PoolWrapper<Event>*>(it->second.get());
        poolWrapper->pool.release(event);
    }

    /**
     * @brief 使用池化事件发送（立即）
     * 
     * 从池中获取事件对象，设置数据后发送，然后自动归还
     * 
     * @tparam Event 事件类型
     * @tparam SetupFunc 设置函数类型
     * @param setup 设置事件数据的函数
     * 
     * Requirements: 9.3
     */
    template<typename Event, typename SetupFunc>
    void emitPooled(SetupFunc setup) {
        Event* event = acquireEvent<Event>();
        setup(*event);
        emit(*event);
        releaseEvent(event);
    }

    /**
     * @brief 使用池化事件发送（延迟）
     * 
     * @tparam Event 事件类型
     * @tparam SetupFunc 设置函数类型
     * @param setup 设置事件数据的函数
     * @param priority 事件优先级
     * 
     * Requirements: 9.3, 9.6
     */
    template<typename Event, typename SetupFunc>
    void emitPooledDeferred(SetupFunc setup, EventPriority priority = EventPriority::NORMAL) {
        Event* event = acquireEvent<Event>();
        setup(*event);
        
        auto wrapper = std::make_unique<EventWrapper<Event>>(*event);
        wrapper->poolReturnFunc = [this](Event* e) {
            // 注意：这里 e 是 wrapper 内部的副本，不是池中的对象
            // 池中的对象已经在 setup 后被复制到 wrapper 中
        };
        
        // 归还原始池对象
        releaseEvent(event);
        
        _deferredEvents.push({
            priority,
            std::move(wrapper),
            _sequenceCounter++
        });
    }

    /**
     * @brief 获取事件池统计信息
     * 
     * @tparam Event 事件类型
     * @return 池大小，如果没有注册池则返回 0
     */
    template<typename Event>
    size_t getEventPoolSize() const {
        auto typeIndex = std::type_index(typeid(Event));
        auto it = _eventPools.find(typeIndex);
        
        if (it == _eventPools.end()) {
            return 0;
        }
        
        auto* poolWrapper = static_cast<const PoolWrapper<Event>*>(it->second.get());
        return poolWrapper->pool.getPoolSize();
    }

    /**
     * @brief 获取底层 EnTT dispatcher
     * 
     * 用于高级用法或直接访问 EnTT 功能
     */
    entt::dispatcher& getDispatcher() {
        return _dispatcher;
    }

    /**
     * @brief 清理所有资源
     */
    void cleanup() {
        clearDeferred();
        _handlers.clear();
        _eventPools.clear();
    }

private:
    EventSystem() = default;
    ~EventSystem() = default;

    /**
     * @brief 内部事件处理器（用于优先级支持）
     */
    template<typename Event>
    void handleEvent(const Event& event) {
        auto typeIndex = std::type_index(typeid(Event));
        auto it = _handlers.find(typeIndex);
        
        if (it != _handlers.end()) {
            for (const auto& handler : it->second) {
                handler.func(&event);
            }
        }
    }

    // 处理器信息
    struct HandlerInfo {
        std::function<void(const void*)> func;
        EventPriority priority;
    };

    // EnTT dispatcher
    entt::dispatcher _dispatcher;
    
    // 处理器映射（按类型索引）
    std::unordered_map<std::type_index, std::vector<HandlerInfo>> _handlers;
    
    // 延迟事件队列（优先级队列）
    std::priority_queue<PrioritizedEvent> _deferredEvents;
    
    // 序列计数器（用于保持同优先级事件的FIFO顺序）
    int64_t _sequenceCounter = 0;
    
    // 事件对象池（类型擦除）
    struct IPoolBase {
        virtual ~IPoolBase() = default;
    };
    
    template<typename T>
    struct PoolWrapper : public IPoolBase {
        ObjectPool<T> pool;
        
        PoolWrapper(typename ObjectPool<T>::CreateFunc createFunc,
                    typename ObjectPool<T>::ResetFunc resetFunc)
            : pool(createFunc, resetFunc) {}
    };
    
    std::unordered_map<std::type_index, std::unique_ptr<IPoolBase>> _eventPools;
};

} // namespace ecs

#endif // __ECS_SYSTEM_EVENTSYSTEM_H__
