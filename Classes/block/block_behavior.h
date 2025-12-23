#include <memory>
#include <optional>
#include "entt/entt.hpp"
#include "block_event.h"
#pragma once

class BlockBehavior;
class DirtBehavior;

class BlockBehaviorRegistry
{
public:
    BlockBehaviorRegistry();
    ~BlockBehaviorRegistry();
    const std::shared_ptr<BlockBehavior> const getBehavior(entt::id_type id) const;
private:
    std::unordered_map<entt::id_type, std::shared_ptr<BlockBehavior>> _behaviors;
};

/**
* @brief 方块行为接口。
* 
* 所有方块行为均需实现此接口。在特定事件发生时，行为会被调用。
* 行为的调用会在对应其他系统之前进行。
* - 行为不需要发送普通事件，系统会在适当的时候发送事件。
* - 行为需要做的事是根据事件执行方块逻辑。
* 
* 复杂逻辑方块的启动入口一般在这里。如箱子在被打开的时候加载其物品栏数据。
*/
class BlockBehavior
{
public:
    /**
    * @brief 当方块被放置时调用。
    */
    virtual void onBlockPlaced(const BlockPlacedEvent& event) = 0;

    /**
    * @brief 当方块被破坏时调用。
    */
    virtual void onBlockDestroyed(const BlockDestroyEvent& event) = 0;

    /**
    * @brief 当方块被挖掘时调用。
    */
    virtual void onBlockMined(const BlockMinedEvent& event) = 0;

    /**
    * @brief 当方块的邻居方块发生变化时调用。
    */
    virtual void onBlockNeighborChanged() = 0;

    /**
    * @brief 当方块被交互时调用。
    */
    virtual void onBlockInteracted(const BlockInteractEvent& event) = 0;

    /**
    * @brief 当方块收到随机刻时调用。
    */
    virtual void onRamdomTick() = 0;
};

class DirtBehavior : public BlockBehavior
{
public:
    virtual void onBlockPlaced(const BlockPlacedEvent& event) override;
    virtual void onBlockDestroyed(const BlockDestroyEvent& event) override;
    virtual void onBlockMined(const BlockMinedEvent& event) override;
    virtual void onBlockNeighborChanged() override;
    virtual void onBlockInteracted(const BlockInteractEvent& event) override;
    virtual void onRamdomTick() override;
};
