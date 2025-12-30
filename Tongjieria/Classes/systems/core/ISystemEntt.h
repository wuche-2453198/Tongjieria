#ifndef __ECS_SYSTEM_ISYSTEMENTT_H__
#define __ECS_SYSTEM_ISYSTEMENTT_H__

#include <entt/entt.hpp>

namespace ecs {

/**
 * @brief System基类
 */
class ISystemEntt {
protected:
    entt::registry* _registry = nullptr;

public:
    virtual ~ISystemEntt() = default;

    /**
     * @brief 获取系统名称
     */
    virtual const char* getName() const = 0;

    /**
     * @brief 获取系统优先级（数值越小越先执行）
     */
    virtual int getPriority() const { return 0; }

    /**
     * @brief 每帧更新
     * @param delta 帧时间（秒）
     */
    virtual void update(float delta) = 0;

    /**
     * @brief 设置Registry
     */
    void setRegistry(entt::registry* registry) {
        _registry = registry;
    }

    /**
     * @brief 获取Registry
     */
    entt::registry* getRegistry() {
        return _registry;
    }
};

} // namespace ecs

#endif // __ECS_SYSTEM_ISYSTEMENTT_H__
