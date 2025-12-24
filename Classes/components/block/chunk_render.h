#pragma once
#include "cocos2d.h"
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

class RenderComponent 
{
public:
    RenderComponent();
    virtual ~RenderComponent();
    virtual void draw(cocos2d::Renderer* renderer, 
        const cocos2d::Mat4& transform, uint32_t flags) = 0;
    virtual void visit(cocos2d::Renderer* renderer, 
        const cocos2d::Mat4& parentTransform, uint32_t parentFlags) = 0;

    bool isActive() const { return _isActive; }
    void setActive(bool active) { _isActive = active; }
    bool isUseTransform() const { return _isUseTransform; }
    void setUseTransform(bool useTransform) { _isUseTransform = useTransform; }

protected:
    bool _isActive = true;
    bool _isUseTransform = true;
};

class BlockBatchCommand : public cocos2d::TrianglesCommand, public RenderComponent
{
public:
    BlockBatchCommand(const Vec2i& blockPos);
    ~BlockBatchCommand();
    virtual void draw(cocos2d::Renderer* renderer,
        const cocos2d::Mat4& transform, uint32_t flags) override;
    virtual void visit(cocos2d::Renderer* renderer,
        const cocos2d::Mat4& parentTransform, uint32_t parentFlags) override;
    void updateShaders();
    void setVertexLayout();
    void updateUniforms(const cocos2d::Mat4& transform);
    std::vector<V3F_C4B_T2F> genVertAtBlockPos(Vec2i block_pos);
private:
    std::vector<V3F_C4B_T2F> _vertices;
    std::vector<unsigned short> _indices;
};

class ChunkBlockBatch 
{
public:
    ChunkBlockBatch() = default;
    virtual ~ChunkBlockBatch() = default;
private:
};

/*
* @brief 区块的渲染指令。
*/
class ChunkCommand : public cocos2d::CustomCommand {
public:
    /*
    * @brief 创建一个区块渲染指令。
    * 由于区块的顶点索引一致，渲染指令初始化的时候可以指定其索引缓冲区进行享元。
    * @param shared_index_buffer 享元索引缓冲区
    */
    ChunkCommand(Buffer* sharedIndexBuffer);
    ~ChunkCommand();
    
    void init(float globalZOrder);
    void updateUniforms(const cocos2d::Mat4& transform);
private:
    void setVertexLayout();
    ProgramState* _programState = nullptr;
    cocos2d::backend::Buffer* _originVBO = nullptr;
};

class SingleBlockCommandData
{

};

class BlockBatchCommandComponent
{
public:
    BlockBatchCommandComponent() = default;
    virtual ~BlockBatchCommandComponent() = default;

};

class BlockCommand: public cocos2d::CustomCommand, public RenderComponent {
public:
    BlockCommand(const Vec2i& pos, cocos2d::Texture2D* texture, Buffer* shared_index_buffer);
    ~BlockCommand() override;
    void init(float globalZOrder);
    void setTexture(cocos2d::Texture2D* texture);
    void updateShaders(cocos2d::Texture2D* texture);
    void setVertexLayout();
    void generateVertex(const Vec2i& local_pos);
    void generateIndex();
    void updateUniforms(const cocos2d::Mat4& transform);
    virtual void draw(cocos2d::Renderer* renderer,
        const cocos2d::Mat4& transform, uint32_t flags) override;
    virtual void visit(cocos2d::Renderer* renderer,
        const cocos2d::Mat4& parentTransform, uint32_t parentFlags) override;
private:
    ProgramState* _programState = nullptr;
};