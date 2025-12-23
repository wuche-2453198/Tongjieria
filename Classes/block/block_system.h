#include <memory>
#include "entt/entt.hpp"
#include "block_event.h"
#include "cocos2d.h"
#pragma once

class ISystem 
{
public:
    ISystem(entt::registry& registry, entt::dispatcher& dispatcher);
    virtual ~ISystem();
    void update(float delta);
protected:
    entt::registry& _registry;
    entt::dispatcher& _dispatcher;
};

class Vec2i;
class BlockLayer;
class ChunkLoadSystem;
class BlockLoadSystem;
class ChunkUnloadSystem;
class BlockPhysicsSystem;
class ChunkRenderCommandSystem;
class BlockInteractSystem;
class DebugSystem;

class BlockSystemManager {
public:
    BlockSystemManager(entt::registry& registry, entt::dispatcher& dispatcher);
    ~BlockSystemManager();
    void update(float delta);
private:
    entt::registry& _registry;
    entt::dispatcher& _dispatcher;

    std::unique_ptr<ChunkLoadSystem> _chunkLoadSystem = nullptr;
    std::unique_ptr<BlockLoadSystem> _blockLoadSystem = nullptr;
    std::unique_ptr<ChunkUnloadSystem> _chunkUnloadSystem = nullptr;
    std::unique_ptr<BlockPhysicsSystem> _blockPhysicsSystem = nullptr;
    std::unique_ptr<ChunkRenderCommandSystem> _chunkRenderCommandSystem = nullptr;
    std::unique_ptr<BlockInteractSystem> _blockInteractSystem = nullptr;
    std::unique_ptr<DebugSystem> _debugSystem = nullptr;
};

class BlockWorld;
class LoadingTicket;

/**
* @brief 区块加载系统。读取任何有**位置**且持有**加载票**的实体，根据它们的加载票加载对应区块。
* 
* 区块的加载，读写权限和根据**优先级**决定。
* - 区块具有对应的**优先级**，计算公式为：优先级 = 加载票的半径 - 区块离加载中心有多少**层**。
* - 若区块附近有多个加载票，其优先级会取最高的那个。
* - 优先级 = 0的区块可以渲染，但是无法读写。
* - 优先级 < 0的区块会进入卸载流。
* - 优先级 > 0的区块会进入加载流。
* 
* 因此一张票的理论最大加载范围是: （2 * 加载票半径 + 1）^ 2。其中最外层为软加载。
* 
* 加载系统不会初始化区块的任何数据，加载系统只加载区块头。区块的内容由其他系统异步加载。
* 
* @see LoadingTicket
*/
class ChunkLoadSystem : public ISystem 
{
public:
    static inline int UPPER_LIMIT = 20;
    static inline int LOWWER_LIMIT = -10;
    ChunkLoadSystem(entt::registry& registry, entt::dispatcher& dispatcher);
    ~ChunkLoadSystem();
    void update(float delta);
private:

    void AddNewChunk();
    void removeLowPriorityChunk();

    /**
    * @brief 在对应的区块坐标上生成一个区块头。
    */
    void addChunk(const Vec2i& chunkPos);

    /**
    * @brief 计算目前所有区块的优先级。
    */
    void updatePriority();

    /**
    * @brief 辅助检查函数，该位置是否合法。
    */
    bool isValiedChunkPos(const Vec2i& chunkPos);

    /**
    * @brief 工具函数，获取chunkPos在center的第几层外。
    */
    int inLayer(const Vec2i& center, const Vec2i& chunkPos);

    int priority(const Vec2i& center, const Vec2i& chunkPos);

    BlockLayer& _blockLayer;
};

class ChunkBlocks;

/**
* @brief 生成区块的方块。调度加载系统和生成系统。
* 
* 使用噪声库生成的噪声（主要是柏林噪声）进行逐方块的生成。
* 性能消耗中等，不建议在每帧调用。
*/
class BlockLoadSystem : public ISystem 
{
public:
    BlockLoadSystem(entt::registry& registry, entt::dispatcher& dispatcher);
    ~BlockLoadSystem();
    void update(float delta);
private:
    ChunkBlocks loadChunk();
    ChunkBlocks generateChunk(); 
    /**
    * @brief 根据世界位置生成方块。
    */
    entt::id_type WorldGenAt(const Vec2i& pos);
};

/**
* @brief 根据优先级卸载区块。
* 
* 优先级 < 0 的方块会进入卸载流。
* - 如果区块被修改过，推入保存流
* - 如果区块没有被修改过，直接卸载。
* 特别地，如果一个区块自生成以来没有被修改过，那么它将不会进入保存流。
* 这意味着，这个区块在每次被加载的时候都会被世界系统从噪声中重新生成。
* 但是性能仍然是可接受的，因为系统IO的性能消耗更高。
*/
class ChunkUnloadSystem : public ISystem
{
public:
    ChunkUnloadSystem(entt::registry& registry, entt::dispatcher& dispatcher);
    ~ChunkUnloadSystem();
    void update(float delta);
private:
};

class SaveChunkSystem : public ISystem
{

};

class BlockBehaviorRegistry;

/**
* @brief 接受区块互动的事件，进行分发和调度。
* 
* 方块被互动的时候，处理对应事件。
* 所有互动事件的处理过程如下：
* - 系统会检查这个方块是否已经成为**方块实体**，如果没有
* - 系统会读取这个方块的配置。生成对应的behavior组件。
* - 系统根据事件
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

class BlockPhysicsLayer;
class AssetManager;
class Position;
class PhysicsTicket;

class BlockPhysicsSystem : public ISystem
{
public:
    BlockPhysicsSystem(entt::registry& registry, entt::dispatcher& dispatcher);
    ~BlockPhysicsSystem();
    void update(float delta);
private:
    std::vector<Vec2i> getAllAddIn();
    std::vector<Vec2i> getAllRemoveOut();
    void addAll(std::vector<Vec2i> allAdded);
    void removeAll(std::vector<Vec2i> allRemoved);
    void updateDirtyBlock();
    bool isInside(const Vec2i& blockPos, const Vec2i& blockUpperLeft, const Vec2i& blockLowerRight);
    bool hasCollision(const Vec2i& blockPos);
    Vec2i getUpperLeft(const Position& worldPos, const PhysicsTicket& ticket);
    Vec2i getLowerRight(const Position& worldPos, const PhysicsTicket& ticket);
    cocos2d::PhysicsShapeBox* createBoxAtBlockPos(const Vec2i& blockPos);
    cocos2d::Node* _physicsNode = nullptr;
    cocos2d::PhysicsBody* _body = nullptr;

    AssetManager& _assetManager;
    BlockLayer& _blockLayer;
    BlockPhysicsLayer& _physicsLayer;
};

class BlockCommand;
class BlockState;

/**
* @brief 生成区块的渲染指令。
*/
class ChunkRenderCommandSystem : public ISystem 
{
public:
    ChunkRenderCommandSystem(entt::registry& registry, entt::dispatcher& dispatcher);
    ~ChunkRenderCommandSystem();
    void update(float delta);
private:
    BlockCommand* generateCommand(const ChunkBlocks& blocks, const Vec2i& localPos);
    void updateChunkCommand();
    void updateDirtyBlock();
};

/**
* @brief 将所有自定义渲染指令提交给Renderer。
*/
class CommandSystem : public ISystem , public cocos2d::Node 
{
public:
    CommandSystem(entt::registry& registry, entt::dispatcher& dispatcher);
    ~CommandSystem();
    void draw(cocos2d::Renderer* renderer, 
        const cocos2d::Mat4& transform, uint32_t flags);

    void visit(cocos2d::Renderer* renderer, 
        const cocos2d::Mat4& parentTransform, uint32_t parentFlags);
private:
};

class MouseEvent;
class DebugSystem : public ISystem
{
public:
    DebugSystem(entt::registry& registry, entt::dispatcher& dispatcher);
    ~DebugSystem();

    void onMouseEvent(const MouseEvent& event);
    void update(float delta);

    void addADrawNode();
    void addAPhysicsSprites();
    
    static inline std::vector<entt::entity> testEntites;
    static inline std::vector<cocos2d::DrawNode*> drawNodes;
    static inline std::vector<cocos2d::Label*> labels;
    static inline std::vector<entt::entity> physicsEntity;
    static inline std::vector<cocos2d::Sprite*> physicsSprites;
};