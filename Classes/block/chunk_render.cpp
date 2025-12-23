#include "chunk_render.h"
#include "block_layer.h"
#include "cocos2d.h"

static constexpr int VERTEX_CAPELICITY = CHUNK_SIZE * CHUNK_SIZE * 4;///< 顶点缓冲区的顶点数量
static constexpr int INDEX_CAPELICITY = CHUNK_SIZE * CHUNK_SIZE * 6; ///< 索引缓冲区的索引数量

ChunkCommand::ChunkCommand(Buffer* shared_index_buffer = nullptr) {
    _indexBuffer = shared_index_buffer;

    updateShaders();
    setBlendFunc();
    setVertexLayout();
    generateVertex();
    generateIndex();
}

ChunkCommand::~ChunkCommand() {}

void ChunkCommand::init(float globalZOrder) {
    CustomCommand::init(globalZOrder);
}

void ChunkCommand::setBlendFunc() {
    
}

void ChunkCommand::updateShaders() {
    CC_SAFE_RELEASE(_programState);
    auto* program = Program::getBuiltinProgram(ProgramType::POSITION_TEXTURE_COLOR);
    _programState = new (std::nothrow) ProgramState(program);

    cocos2d::Texture2D* texture = cocos2d::Director::getInstance()->
        getTextureCache()->addImage("a_block.bmp");
    texture->setAliasTexParameters();
    auto textureLocation = _programState->getUniformLocation("u_texture");
    _programState->setTexture(textureLocation, 0, texture->getBackendTexture());

    getPipelineDescriptor().programState = _programState;
    setDrawType(DrawType::ELEMENT);
    setPrimitiveType(PrimitiveType::TRIANGLE);
}

void ChunkCommand::setVertexLayout() {
    auto layout = _programState->getVertexLayout();

    layout->setAttribute(
        cocos2d::backend::ATTRIBUTE_NAME_POSITION,
        _programState->getAttributeLocation(Attribute::POSITION),
        VertexFormat::FLOAT3,
        0,
        false
    );

    layout->setAttribute(
        cocos2d::backend::ATTRIBUTE_NAME_TEXCOORD,
        _programState->getAttributeLocation(Attribute::TEXCOORD),
        VertexFormat::FLOAT2,
        offsetof(V3F_C4B_T2F, texCoords),
        false
    );

    layout->setAttribute(
        cocos2d::backend::ATTRIBUTE_NAME_COLOR,
        _programState->getAttributeLocation(Attribute::COLOR),
        VertexFormat::UBYTE4,
        offsetof(V3F_C4B_T2F, colors),
        true
    );

    layout->setLayout(sizeof(cocos2d::V3F_C4B_T2F));
}

void ChunkCommand::generateBlockVertexAt(std::vector<V3F_C4B_T2F>& vertex, const Vec2i& local_pos) {
    cocos2d::Color4B WHITE = cocos2d::Color4B::WHITE;
    int index = (local_pos.x + local_pos.y * CHUNK_SIZE) * 4;
    vertex[index + 0] = {  local_pos               * BLOCK_SIZE, WHITE, {0, 0} };
    vertex[index + 1] = { (local_pos + Vec2i(0,1)) * BLOCK_SIZE, WHITE, {0, 1} };
    vertex[index + 2] = { (local_pos + Vec2i(1,1)) * BLOCK_SIZE, WHITE, {1, 1} };
    vertex[index + 3] = { (local_pos + Vec2i(1,0)) * BLOCK_SIZE, WHITE, {1, 0} };
}

void ChunkCommand::generateVertex() {
    if (_vertexBuffer) return;

    createVertexBuffer(sizeof(cocos2d::V3F_C4B_T2F), VERTEX_CAPELICITY, Usage::DYNAMIC);

    std::vector<V3F_C4B_T2F> vertex(VERTEX_CAPELICITY);
    for (int y = 0; y < CHUNK_SIZE; y++) {
        for (int x = 0; x < CHUNK_SIZE; x++) {
            generateBlockVertexAt(vertex, { x, y });
        }
    }
    updateVertexBuffer(vertex.data(), VERTEX_CAPELICITY * sizeof(V3F_C4B_T2F));
    setVertexDrawInfo(0, VERTEX_CAPELICITY);
}

void ChunkCommand::generateIndex() {
    if (_indexBuffer) return;
    createIndexBuffer(IndexFormat::U_SHORT, INDEX_CAPELICITY, Usage::DYNAMIC);
    std::vector<unsigned short> index(INDEX_CAPELICITY);
    for (int k = 0; k < CHUNK_SIZE * CHUNK_SIZE; k++)
    {
        int i = k * 6;
        int v = k * 4;
        index[i + 0] = v; index[i + 1] = v + 1; index[i + 2] = v + 2;
        index[i + 3] = v; index[i + 4] = v + 2; index[i + 5] = v + 3;
    }
    updateIndexBuffer(index.data(), INDEX_CAPELICITY * sizeof(unsigned short));
    setIndexDrawInfo(0, INDEX_CAPELICITY);
}

void ChunkCommand::updateUniforms(const cocos2d::Mat4& transform) {
    auto& pipelineDescriptor = getPipelineDescriptor();
    const auto& matrixP = cocos2d::Director::getInstance()
        ->getMatrix(cocos2d::MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);
    cocos2d::Mat4 matrixMVP = matrixP * transform;
    auto mvpLocation = _programState->getUniformLocation("u_MVPMatrix");
    _programState->setUniform(mvpLocation, matrixMVP.m, sizeof(matrixMVP.m));

    float alpha = 255.0f / 255.0f;
    auto alphaUniformLocation = _programState->getUniformLocation("u_alpha");
    _programState->setUniform(alphaUniformLocation, &alpha, sizeof(alpha));
}

RenderComponent::RenderComponent() = default;

RenderComponent::~RenderComponent() = default;

BlockCommand::BlockCommand(const Vec2i& pos, cocos2d::Texture2D* texture, Buffer* shared_index_buffer) {
    _indexBuffer = shared_index_buffer;

    updateShaders(texture);
    setVertexLayout();
    generateVertex(pos);
    generateIndex();
}

BlockCommand::~BlockCommand()
{
    CC_SAFE_RELEASE(_programState);
}

void BlockCommand::init(float globalZOrder) {
    CustomCommand::init(globalZOrder);
}

void BlockCommand::setTexture(cocos2d::Texture2D* texture) {
    texture->setAliasTexParameters();
    auto textureLocation = _programState->getUniformLocation("u_texture");
    _programState->setTexture(textureLocation, 0, texture->getBackendTexture());
}

void BlockCommand::updateShaders(cocos2d::Texture2D* texture) {
    CC_SAFE_RELEASE(_programState);
    auto* program = Program::getBuiltinProgram(ProgramType::POSITION_TEXTURE_COLOR);
    _programState = new (std::nothrow) ProgramState(program);

    setTexture(texture);

    getPipelineDescriptor().programState = _programState;
    setDrawType(DrawType::ELEMENT);
    setPrimitiveType(PrimitiveType::TRIANGLE);
}

void BlockCommand::setVertexLayout() {
    auto layout = _programState->getVertexLayout();

    layout->setAttribute(
        cocos2d::backend::ATTRIBUTE_NAME_POSITION,
        _programState->getAttributeLocation(Attribute::POSITION),
        VertexFormat::FLOAT3,
        0,
        false
    );

    layout->setAttribute(
        cocos2d::backend::ATTRIBUTE_NAME_TEXCOORD,
        _programState->getAttributeLocation(Attribute::TEXCOORD),
        VertexFormat::FLOAT2,
        offsetof(V3F_C4B_T2F, texCoords),
        false
    );

    layout->setAttribute(
        cocos2d::backend::ATTRIBUTE_NAME_COLOR,
        _programState->getAttributeLocation(Attribute::COLOR),
        VertexFormat::UBYTE4,
        offsetof(V3F_C4B_T2F, colors),
        true
    );

    layout->setLayout(sizeof(V3F_C4B_T2F));
}

void BlockCommand::generateVertex(const Vec2i& pos) {
    if (_vertexBuffer) return;

    createVertexBuffer(sizeof(V3F_C4B_T2F), 4, Usage::DYNAMIC);

    std::vector<V3F_C4B_T2F> vertex(4);

    cocos2d::Color4B WHITE = cocos2d::Color4B::WHITE;
    vertex[0] = { pos * BLOCK_SIZE, WHITE, {0, 0} };
    vertex[1] = { (pos + Vec2i(0,1)) * BLOCK_SIZE, WHITE, {0, 1} };
    vertex[2] = { (pos + Vec2i(1,1)) * BLOCK_SIZE, WHITE, {1, 1} };
    vertex[3] = { (pos + Vec2i(1,0)) * BLOCK_SIZE, WHITE, {1, 0} };

    updateVertexBuffer(vertex.data(), 4 * sizeof(V3F_C4B_T2F));
    setVertexDrawInfo(0, 4);
}

void BlockCommand::generateIndex() {
    if (_indexBuffer) return;
    createIndexBuffer(IndexFormat::U_SHORT, 6, Usage::DYNAMIC);
    std::vector<unsigned short> index = { 0,1,2,0,2,3 };
    updateIndexBuffer(index.data(), 6 * sizeof(unsigned short));
    setIndexDrawInfo(0, 6);
}

void BlockCommand::updateUniforms(const cocos2d::Mat4& transform) {
    auto& pipelineDescriptor = getPipelineDescriptor();
    const auto& matrixP = cocos2d::Director::getInstance()
        ->getMatrix(cocos2d::MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);
    cocos2d::Mat4 matrixMVP = matrixP * transform;
    auto mvpLocation = _programState->getUniformLocation("u_MVPMatrix");
    _programState->setUniform(mvpLocation, matrixMVP.m, sizeof(matrixMVP.m));

    float alpha = 255.0f / 255.0f;
    auto alphaUniformLocation = _programState->getUniformLocation("u_alpha");
    _programState->setUniform(alphaUniformLocation, &alpha, sizeof(alpha));
}

void BlockCommand::draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags) 
{
    init(0);
    updateUniforms(transform);
    renderer->addCommand(this);
}

void BlockCommand::visit(cocos2d::Renderer* renderer, const cocos2d::Mat4& parentTransform, uint32_t parentFlags) {}