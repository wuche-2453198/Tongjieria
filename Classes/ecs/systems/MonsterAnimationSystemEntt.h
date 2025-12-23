#ifndef __ECS_SYSTEM_MONSTERANIMATIONSYSTEMENTT_H__
#define __ECS_SYSTEM_MONSTERANIMATIONSYSTEMENTT_H__

#include "ISystemEntt.h"
#include "SystemPriority.h"
#include "../AllComponents.h"

namespace ecs {

/**
 * @brief 怪物精灵动画系统 - 已被AnimationSystem替代（新架构）
 * 
 * 注意：此系统已废弃，动画现在由AnimationSystem统一处理
 * 保留此文件仅为兼容性，实际不应再注册此系统
 */
class MonsterAnimationSystemEntt : public ISystemEntt {
public:
    const char* getName() const override { return "MonsterAnimationSystem"; }
    int getPriority() const override { return SystemPriority::ANIMATION; }

    void update(float delta) override {
        // 新架构：动画由AnimationSystem统一处理
        // 此系统已废弃，不执行任何操作
    }
};

} // namespace ecs

#endif // __ECS_SYSTEM_MONSTERANIMATIONSYSTEMENTT_H__
