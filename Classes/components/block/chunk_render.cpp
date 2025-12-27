#include "cocos2d.h"
#include "cocos/renderer/backend/Device.h"
#include "cocos/renderer/backend/Buffer.h"
#include "chunk_render.h"
#include "systems/block_layer/block_layer.h"
#include "core/consts.h"

static constexpr int VERTEX_CAPELICITY = CHUNK_SIZE * CHUNK_SIZE * 4;///< 顶点缓冲区的顶点数量
static constexpr int INDEX_CAPELICITY = CHUNK_SIZE * CHUNK_SIZE * 6; ///< 索引缓冲区的索引数量

using Vec3 = cocos2d::Vec3;
using Vec2 = cocos2d::Vec2;
using C4B = cocos2d::Color4B;
using Tex2F = cocos2d::Tex2F;

BlockBatchCommand::BlockBatchCommand(int globalOrder, const Vec2i& blockPos, cocos2d::Texture2D* texture)
    : BlockBatchCommand(globalOrder, std::vector<Vec2i>{blockPos}, texture)
{}

BlockBatchCommand::BlockBatchCommand(int globalOrder, const std::vector<Vec2i>& blockPos, cocos2d::Texture2D* texture)
{
    auto* triangles = new Triangles();
    _vertices = genVert(blockPos);
    _indices = genIndex(blockPos.size());

    triangles->indexCount = _indices.size();
    triangles->indices = _indices.data();
    triangles->vertCount = _vertices.size();
    triangles->verts = _vertices.data();

    updateShaders();
    setTexture(texture);
    setVertexLayout();
    init(globalOrder, texture, cocos2d::BlendFunc::ALPHA_PREMULTIPLIED, *triangles, cocos2d::Mat4(), 0);
}

BlockBatchCommand::~BlockBatchCommand() 
{
    CC_SAFE_RELEASE(_pipelineDescriptor.programState);
}

void BlockBatchCommand::setTexture(cocos2d::Texture2D* texture)
{   
    auto programState = getPipelineDescriptor().programState;
    auto textureLocation = programState->getUniformLocation("u_texture");
    programState->setTexture(textureLocation, 0, texture->getBackendTexture());
}

void BlockBatchCommand::setNewTexture(cocos2d::Texture2D* texture)
{
    if (_texture == texture->getBackendTexture())
    {
        return;
    }
    updateShaders();
    setTexture(texture);
    init(_globalOrder, texture, cocos2d::BlendFunc::ALPHA_PREMULTIPLIED, _triangles, cocos2d::Mat4(), 0);
}

void BlockBatchCommand::updateShaders()
{
    CC_SAFE_RELEASE(_pipelineDescriptor.programState);
    auto* program = Program::getBuiltinProgram(ProgramType::POSITION_TEXTURE_COLOR);
    auto programState = new (std::nothrow) ProgramState(program);
    getPipelineDescriptor().programState = programState;
}

void BlockBatchCommand::setVertexLayout() 
{
    auto programState = getPipelineDescriptor().programState;
    auto layout = programState->getVertexLayout();

    layout->setAttribute(
        cocos2d::backend::ATTRIBUTE_NAME_POSITION,
        programState->getAttributeLocation(Attribute::POSITION),
        VertexFormat::FLOAT3,
        0,
        false
    );

    layout->setAttribute(
        cocos2d::backend::ATTRIBUTE_NAME_TEXCOORD,
        programState->getAttributeLocation(Attribute::TEXCOORD),
        VertexFormat::FLOAT2,
        offsetof(V3F_C4B_T2F, texCoords),
        false
    );

    layout->setAttribute(
        cocos2d::backend::ATTRIBUTE_NAME_COLOR,
        programState->getAttributeLocation(Attribute::COLOR),
        VertexFormat::UBYTE4,
        offsetof(V3F_C4B_T2F, colors),
        true
    );

    layout->setLayout(sizeof(V3F_C4B_T2F));
}

void BlockBatchCommand::updateUniforms(const cocos2d::Mat4& transform)
{
    auto& pipelineDescriptor = getPipelineDescriptor();
    const auto& matrixP = cocos2d::Director::getInstance()
        ->getMatrix(cocos2d::MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);
    cocos2d::Mat4 matrixMVP = matrixP * transform;
    auto programState = getPipelineDescriptor().programState;
    auto mvpLocation = programState->getUniformLocation("u_MVPMatrix");
    programState->setUniform(mvpLocation, matrixMVP.m, sizeof(matrixMVP.m));
}

std::vector<V3F_C4B_T2F> BlockBatchCommand::genVert(const std::vector<Vec2i>& blockPos)
{
    std::vector<V3F_C4B_T2F> verts(blockPos.size() * 4);

    int index = 0;
    for (int i = 0; i < blockPos.size(); i ++)
    {
        Vec2i pos = blockPos[i];
        int vertIndex = i * 4;
        verts[vertIndex]     = V3F_C4B_T2F(Vec3(pos.x,     pos.y,     0) * BLOCK_SIZE, C4B::WHITE, Tex2F(0, 0));
        verts[vertIndex + 1] = V3F_C4B_T2F(Vec3(pos.x,     pos.y + 1, 0) * BLOCK_SIZE, C4B::WHITE, Tex2F(0, 1));
        verts[vertIndex + 2] = V3F_C4B_T2F(Vec3(pos.x + 1, pos.y + 1, 0) * BLOCK_SIZE, C4B::WHITE, Tex2F(1, 1));
        verts[vertIndex + 3] = V3F_C4B_T2F(Vec3(pos.x + 1, pos.y,     0) * BLOCK_SIZE, C4B::WHITE, Tex2F(1, 0));
    }
    return verts;
}

std::vector<unsigned short> BlockBatchCommand::genIndex(int blockNum)
{
    std::vector<unsigned short> indices(blockNum * 6);
    for (int i = 0; i < blockNum; i++)
    {
        indices[i * 6] = i * 4;
        indices[i * 6 + 1] = i * 4 + 1;
        indices[i * 6 + 2] = i * 4 + 2;
        indices[i * 6 + 3] = i * 4;
        indices[i * 6 + 4] = i * 4 + 2;
        indices[i * 6 + 5] = i * 4 + 3;
    }
    return indices;
}

void BlockBatchCommand::draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags)
{
    RenderCommand::init(_globalOrder, transform, flags);
    updateUniforms(transform);
    renderer->addCommand(this);
}

void BlockBatchCommand::visit(cocos2d::Renderer* renderer, const cocos2d::Mat4& parentTransform, uint32_t parentFlags) {}

RenderComponent::RenderComponent() = default;

RenderComponent::~RenderComponent() = default;

void ChunkRenderBatchID::addBatchID(entt::id_type id, int commandIndex)
{
    assert(!hasBatchID(id), "ID 重复添加！");
    _blockBatchMap[id] = commandIndex;
}

void ChunkRenderBatchID::removeBatchID(entt::id_type id)
{
    assert(hasBatchID(id), "ID 重复抹除或不存在！");
    _blockBatchMap.erase(id);
}

int ChunkRenderBatchID::getBatchIndex(entt::id_type id) const
{
    return _blockBatchMap.at(id);
}

bool ChunkRenderBatchID::hasBatchID(entt::id_type id) const
{
    return _blockBatchMap.find(id) != _blockBatchMap.end();
}

void ChunkRenderBatchID::clear()
{
    _blockBatchMap.clear();
}
