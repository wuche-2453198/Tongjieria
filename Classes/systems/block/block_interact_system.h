#include <memory>
#include "entt/entt.hpp"
#include "components/block/block_event.h"
#include "block_system_manager.h"
#include "cocos2d.h"
#pragma once

class BlockBehaviorRegistry;

/**
* @brief 接受区块互动的事件，进行分发和调度。
*
* 方块被互动的时候，处理对应事件。
* 所有互动事件的处理过程如下：
* - 系统会检查这个方块是否已经成为**方块实体**，如果没有
* - 系统会读取这个方块的配置。生成对应的behavior组件。
*/
class BlockInteractSystem : public ISystem
{
public:
    BlockInteractSystem(entt::registry& registry, entt::dispatcher& dispatcher);
    ~BlockInteractSystem();
private:
    void onBlockPlaced(const BlockPlacedEvent& event);
    void onBlockDestroyed(const BlockDestroyEvent& event);
    void onBlockMined(const BlockMinedEvent& event);
    void onBlockNeighborChanged();
    void onBlockInteracted(const BlockInteractEvent& event);
    void onRandomTick();
    void addDirtyTag(entt::entity chunk, const Vec2i& localPos);
    std::unique_ptr<BlockBehaviorRegistry> _behaviorRegistry;
};

class BlockUpdateSystem : public ISystem
{
public:
    BlockUpdateSystem(entt::registry& registry, entt::dispatcher& dispatcher);
    ~BlockUpdateSystem();
};