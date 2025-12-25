#pragma once
#include "cocos2d.h"
#include "block_component.h"
#include "utils/vec2i.h"

class ChunkHead;
class AssetManager;
class BlockLayer;

using Program = cocos2d::backend::Program;
using ProgramType = cocos2d::backend::ProgramType;
using ProgramState = cocos2d::backend::ProgramState;
using DrawType = cocos2d::CustomCommand::DrawType;
using PrimitiveType = cocos2d::CustomCommand::PrimitiveType;
using Buffer = cocos2d::backend::Buffer;
using Usage = cocos2d::backend::BufferUsage;
using VertexFormat = cocos2d::backend::VertexFormat;
using IndexFormat = cocos2d::CustomCommand::IndexFormat;
using Attribute = cocos2d::backend::Attribute;
using V3F_C4B_T2F = cocos2d::V3F_C4B_T2F;

/**
* @class RenderComponent
* 
* @brief 渲染命令封装，以供渲染系统方便地提交给cocos
* 
* 本身并不是组件，它只是提供一个渲染接口。渲染组件见CustomCommandPack
* 
* @see CustomCommandPack
* 
* @tease 正如Opengl不Open， 渲染组件也不组件 UwU。
*/
class RenderComponent
{
public:
    RenderComponent();
    virtual ~RenderComponent();
    virtual void draw(cocos2d::Renderer* renderer, 
        const cocos2d::Mat4& transform, uint32_t flags) = 0;
    virtual void visit(cocos2d::Renderer* renderer, 
        const cocos2d::Mat4& parentTransform, uint32_t parentFlags) = 0;

    /**
    * @brief 是否激活
    * 
    * 未激活的渲染组件不会被提交。
    */
    bool isActive() const { return _isActive; }
    void setActive(bool active) { _isActive = active; }

    /**
    * @brief 是否使用变换
    * 
    * 是否使用对应实体的位置变换。如果不使用，默认传入的矩阵是世界矩阵，顶点会渲染在对应的世界位置上。
    */
    bool isUseTransform() const { return _isUseTransform; }
    void setUseTransform(bool useTransform) { _isUseTransform = useTransform; }

protected:
    bool _isActive = true;
    bool _isUseTransform = true;
};

/**
 * @class BlockBatchCommand
 * @brief 批量处理块的命令类
 *
 * BlockBatchCommand 类提供了批量处理多个块的功能。通过这个类，用户可以对一系列的块执行特定的操作。
 */
class BlockBatchCommand : public cocos2d::TrianglesCommand, public RenderComponent
{
public:
    /**
    * @brief 构造函数
    *
    * 初始化 BlockBatchCommand 对象。
    * 
    * @param blockPos 方块位置
    * @param texture 贴图
    */
    BlockBatchCommand(const Vec2i& blockPos, cocos2d::Texture2D* texture);

    /**
    * @brief 构造函数
    * 
    * 根据方块位置批量生成顶点数据
    * 
    * @param blockPos 方块位置列表
    * @param texture 贴图
    */
    BlockBatchCommand(const std::vector<Vec2i>& blockPosArray, cocos2d::Texture2D* texture);

    /**
    * @ToDO 检查是否析构完全
    */
    ~BlockBatchCommand();

    /**
    * @brief 绘制命令
    * 
    * 绘制命令，将方块渲染到屏幕上。
    */
    virtual void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags) override;
    virtual void visit(cocos2d::Renderer* renderer, const cocos2d::Mat4& parentTransform, uint32_t parentFlags) override;

    /**
    * @brief 更新顶点数据
    * 
    * 更新uniforms，主要是矩阵
    */
    void updateUniforms(const cocos2d::Mat4& transform);
private:

    /**
    * @brief 设置贴图
    */
    void setTexture(cocos2d::Texture2D* texture);
    void updateShaders();

    /**
    * @brief 
    * 
    * 
    */
    void setVertexLayout();

    /**
    * @brief 生成顶点
    * 
    * @param blockPos 方块位置列表
    */
    std::vector<V3F_C4B_T2F> genVert(const std::vector<Vec2i>& blockPos);

    /**
    * @brief 生成索引
    * 
    * @param blockNum 方块数量
    */
    std::vector<unsigned short> genIndex(int blockNum);

    std::vector<V3F_C4B_T2F> _vertices;     ///< 顶点数据
    std::vector<unsigned short> _indices;   ///< 索引数据
};

/**
* @brief 区块渲染批次ID->渲染批次索引映射表
* 
* 存储着渲染批次id和它们在对应的渲染包中的位置
* 区块渲染批次ID只作为区块渲染系统的专用缓存。
* 它不会对渲染命令的生命周期负责。
*/
class ChunkRenderBatchID : public IComponent
{
public:
    ChunkRenderBatchID() = default;
    ~ChunkRenderBatchID() = default;
    void addBatchID(entt::id_type id, int commandIndex);
    void removeBatchID(entt::id_type id);
    int getBatchIndex(entt::id_type id) const;
    bool hasBatchID(entt::id_type id) const;
    void clear();
private:
    std::unordered_map<entt::id_type, int> _blockBatchMap;
};