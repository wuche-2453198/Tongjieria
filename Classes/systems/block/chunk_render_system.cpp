#include "core/assets_manager.h"
#include "components/block/block_component.h"
#include "components/block/chunk_render.h"
#include "systems/block_layer/block_layer.h"
#include "chunk_render_system.h"
#include "utils/tools.h"

ChunkRenderSystem::ChunkRenderSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : ISystem(registry, dispatcher), _assetManager(_registry.ctx().get<AssetManager>()) {};
ChunkRenderSystem::~ChunkRenderSystem() = default;

void ChunkRenderSystem::update(float delta)
{
    createCommand();
    cutCommand();
    updateDirtyBlock();
}

void ChunkRenderSystem::createCommand()
{
    // 获取已经生成但没有渲染指令的区块
    auto view = _registry.view<Position, ChunkBlocks>(entt::exclude<CustomcommandPack>);
    view.each([&](entt::entity entity, Position& pos, ChunkBlocks& blocks)
        {
            // 获取当前相机的区块位置
            Vec2i cameraChunkPos =
                BlockLayer::worldPosToChunkPos(cocos2d::Camera::getDefaultCamera()->getPosition());
            Vec2i chunkPos = BlockLayer::worldPosToChunkPos(pos);

            // 超出渲染距离
            if ((cameraChunkPos - chunkPos).dis() >= CHUNK_RENDER_START_DISTANCE) return;

            auto& batchs = _registry.emplace<CustomcommandPack>(entity);
            auto& batchIDs = _registry.emplace<ChunkRenderBatchID>(entity);

            constructPack(chunkPos, blocks, batchs, batchIDs);
        });
}

void ChunkRenderSystem::cutCommand()
{
    auto view = _registry.view<Position, ChunkBlocks, CustomcommandPack, ChunkRenderBatchID>();
    view.each([&](entt::entity entity, Position& pos, ChunkBlocks& blocks, CustomcommandPack& pack, ChunkRenderBatchID& id)
        {
            // 获取当前相机的区块位置
            Vec2i cameraChunkPos =
                BlockLayer::worldPosToChunkPos(cocos2d::Camera::getDefaultCamera()->getPosition());
            Vec2i chunkPos = BlockLayer::worldPosToChunkPos(pos);
            
            // 距离较近，还在渲染队列中
            if ((cameraChunkPos - chunkPos).dis() <= CHUNK_RENDER_CUT_DISTANCE) return;

            _registry.remove<CustomcommandPack>(entity);
            _registry.remove<ChunkRenderBatchID>(entity);
        });
}

void ChunkRenderSystem::updateDirtyBlock()
{
    auto view = _registry.view<Position, ChunkBlocks, CustomcommandPack, ChunkRenderBatchID, DirtyChunkTag>();
    view.each([&](entt::entity entity, Position& pos, ChunkBlocks& blocks, CustomcommandPack& pack, ChunkRenderBatchID& id, DirtyChunkTag tag)
        {
            // 清理区块命令和渲染缓存
            pack.releaseAllCommand();
            id.clear();

            Vec2i chunkPos = BlockLayer::worldPosToChunkPos(pos);

            constructPack(chunkPos, blocks, pack, id);
            
            for (auto& dirtyBlock : tag.dirtyBlocks)
            {
                dirtyBlock.renderDirty = false;
            }
            if (tag.isAllClean())
            {
                _registry.remove<DirtyChunkTag>(entity);
            }
        });
}

void ChunkRenderSystem::constructPack(const Vec2i chunkPos, const ChunkBlocks& blocks, CustomcommandPack& pack, ChunkRenderBatchID& id)
{
    std::unordered_map<entt::id_type, std::vector<Vec2i>> blockbatch;

    // 记录每个方块的位置
    for (int y = 0; y < CHUNK_SIZE; y++)
    {
        for (int x = 0; x < CHUNK_SIZE; x++)
        {
            Vec2i localPos = Vec2i(x, y);
            entt::id_type blockID = blocks.getBlockAt(localPos);
            if (_assetManager.getBlockConfig(blockID).isRenderble())
            {
                Vec2i blockPos = chunkPos * CHUNK_SIZE + localPos;
                blockbatch[blockID].push_back(blockPos);            // 记录该方块的位置
            }
        }
    }

    int top = 0;
    for (auto& [blockID, positions] : blockbatch)
    {
        // 读取方块配置
        auto texture = _assetManager.getBlockConfig(blockID).texture();

        auto batchCommand = new BlockBatchCommand(0, positions, texture);
        batchCommand->setUseTransform(false);   // 不使用transform，直接使用block的位置
        pack.commands.push_back(batchCommand);  // 添加到batchs中
        id.addBatchID(blockID, top);            // 记录batchID和top
        top++;                                  // top加1
    }
}
