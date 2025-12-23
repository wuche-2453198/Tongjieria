#include <memory>
#include <optional>
#include "entt/entt.hpp"
#include "server/consts.h"
#include "utils/vec2i.h"
#pragma once

namespace cocos2d {
    class Vec2;
    class CustomCommand;
};

class Vec2i;

/**
* @brief 组件基类接口。
* 
* @tease 真的有用吗？
*/
class IComponent {};

/**
* @brief 浮点数位置组件。所有有位置的对象都应该带有这个组件。
* 
* @note 不兼容UI，UI应该有专用的位置组件。
*/
class Position : public IComponent {
public:
    Position();
    Position(float x, float y);
    Position(const Vec2i& vec);
    Position(const cocos2d::Vec2& vec);

    operator cocos2d::Vec2() const;
    operator Vec2i() const;

    const cocos2d::Vec2& getPostion() const;
    void setPosition(const cocos2d::Vec2& pos);
private:
    cocos2d::Vec2 pos; ///< 位置
};


using BlockArray = std::array<std::array<entt::id_type, CHUNK_SIZE>, CHUNK_SIZE>;

/**
* @brief 加载票。任何持有票且有位置的实体会被区块加载系统读取，加载一定半径的区块。
* 让玩家以外的实体持有永久加载票是危险的，需要谨慎使用。
*
* @see ChunkLoadingSystem
*/
struct LoadingTicket {
    LoadingTicket();
    LoadingTicket(entt::entity entity_id, unsigned int radius, bool is_permanent)
        :entity_id(entity_id), radius(radius), is_permanent(is_permanent) {};
    LoadingTicket(entt::entity entity_id, unsigned int radius, float doration)
        :entity_id(entity_id), radius(radius), doration(doration) {};

    entt::entity entity_id;
    unsigned int radius = 0;
    bool is_permanent = false;
    float doration = -1;
};

/**
* @brief 区块头，包含这个区块的基本信息。
* 
* @see ChunkLoadSystem
* 
* @tease 原来区块是一个整体来着（本来区块类包含了区块的所有信息，包括方块和渲染），
* 还有一个温暖的家（blockLayer），后来被万恶的开发者分尸后扔到大路上了（registry），
* 这个是他的头。
*/
class ChunkHead : public IComponent {
public:
    ChunkHead();
    ChunkHead(int priority);
    ~ChunkHead();

    int getPriority() const;
    void setPriority(int priority);

    static inline int UNLOADING_PRIORITY = 0; ///< 卸载优先级阈值
private:
    int _priority = 0; ///< 区块优先级
};

/**
* @brief 区块网格，存储这个区块下的所有方块。
* 
* @note 注意！所有对区块的操作函数都不检查输入的位置。
* 如果想安全地操作方块，使用：BlockLayer
* 
* @see BlockLoadSystem
* 
* @tease 这个是他的身体。
*/
class ChunkBlocks : public IComponent {
public:
    ChunkBlocks();
    ~ChunkBlocks();

    entt::id_type getBlockAt(const Vec2i& pos) const;
    void setBlockAt(const Vec2i& pos, entt::id_type id);

    /**
    * @brief 局部位置是否合法。
    */
    bool isPosValied(const Vec2i& pos) const;
    const BlockArray& const getBlockView() const;
private:
    BlockArray _blocks; ///< 区块内的方块数组
    std::vector<entt::entity> _blockEntities; ///< 这个区块加载的方块实体列表
};

struct PhysicsTicket
{
    PhysicsTicket() = default;
    PhysicsTicket(const cocos2d::Vec2& size, const cocos2d::Vec2& offset)
        : size(size), offset(offset) {}

    cocos2d::Vec2 size;        ///< 粗物理体大小
    cocos2d::Vec2 offset;      ///< 粗物理体偏移
};

class World;

class WorldScene
{
public:
    WorldScene(World* world);
    ~WorldScene();
    World& operator*() const;
    World* operator->() const;
    operator bool() const;
private:
    World* _world = nullptr;
};

class RenderComponent;

/**
* @brief 渲染包。一个自定义渲染对象会在其中提交自己所有的命令。
* 这个包在提交到registry会被渲染系统读取并转发给cocos渲染管线。
* 
* @see CommandSystem
* 
* @tease 你怎么在registry中被删除两次了呀，害得我19号晚上两个小时消失了。
*/
class CustomcommandPack : public IComponent {
public:
    CustomcommandPack();
    ~CustomcommandPack();
    CustomcommandPack(CustomcommandPack&& other) noexcept;

    std::vector<RenderComponent*> commands; ///< 渲染命令列表
};

struct DirtyBlock
{
    DirtyBlock(const Vec2i& pos)
        : localPos(pos) {};
    bool isClean();
    Vec2i localPos;
    bool collisionDirty = true;
    bool renderDirty = true;
};

struct DirtyChunkTag
{
    DirtyChunkTag();
    void addDirtyBlock(const Vec2i& pos);
    bool isAllClean();
    std::vector<DirtyBlock> dirtyBlocks;
};