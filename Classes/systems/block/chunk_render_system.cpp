#include "core/assets_manager.h"
#include "components/block/block_component.h"
#include "components/block/chunk_render.h"
#include "systems/block_layer/block_layer.h"
#include "chunk_render_system.h"
#include "utils/tools.h"

ChunkRenderCommandSystem::ChunkRenderCommandSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : ISystem(registry, dispatcher) {};
ChunkRenderCommandSystem::~ChunkRenderCommandSystem() = default;

void ChunkRenderCommandSystem::update(float delta)
{
    updateChunkCommand();

    //updateDirtyBlock();
}

void ChunkRenderCommandSystem::updateChunkCommand()
{
    // 获取已经生成但没有渲染指令的区块
    auto view = _registry.view<Position, ChunkBlocks>(entt::exclude<CustomcommandPack>);
    view.each([&](entt::entity entity, Position& pos, ChunkBlocks& blocks)
        {
            Vec2i cameraChunkPos =
                BlockLayer::worldPosToChunkPos(cocos2d::Camera::getDefaultCamera()->getPosition());
            Vec2i chunkPos =
                BlockLayer::worldPosToChunkPos(pos.getPostion());

            auto& batchs = _registry.emplace<CustomcommandPack>(entity).commands;
            auto& batchIDs = _registry.emplace<ChunkRenderBatchID>(entity);

            std::unordered_map<entt::id_type, std::vector<Vec2i>> blockbatch;

            for (int y = 0; y < CHUNK_SIZE; y++)
            {
                for (int x = 0; x < CHUNK_SIZE; x++)
                {
                    Vec2i localPos = Vec2i(x, y);
                    entt::id_type blockID = blocks.getBlockAt(localPos);
                    Vec2i blockPos = chunkPos * CHUNK_SIZE + localPos;
                    
                    blockbatch[blockID].push_back(blockPos);
                }
            }

            int top = 0;
            for (auto& [blockID, positions] : blockbatch)
            {
                // 读取方块配置
                auto& asset_manager = _registry.ctx().get<AssetManager>();
                auto config = asset_manager.getBlockConfig(blockID);
                
                // 读取方块纹理
                std::string texturePath = tools::get_str_or(*config, "texture", "a_block.bmp");
                auto texture = asset_manager.getTexture(texturePath);

                auto batchCommand = new BlockBatchCommand(positions, texture);
                batchCommand->setUseTransform(false);
                batchs.push_back(batchCommand);
                batchIDs.addBatchID(blockID, top);
                top++;
            }

            CCLOG("Create Chunk commands at %d %d", chunkPos.x, chunkPos.y);
        });
}

void ChunkRenderCommandSystem::updateUnseen()
{

}

void ChunkRenderCommandSystem::updateDirtyBlock()
{
    auto view = _registry.view<ChunkBlocks, CustomcommandPack, DirtyChunkTag>();
    view.each([&](entt::entity entity, ChunkBlocks& blocks, CustomcommandPack& pack, DirtyChunkTag tag)
        {
            for (auto& dirtyBlock : tag.dirtyBlocks)
            {
                Vec2i& localPos = dirtyBlock.localPos;
                int cmdPos = localPos.x + localPos.y * CHUNK_SIZE;
                //auto blockCommand = generateCommand(blocks, localPos);
                delete pack.commands[cmdPos];
                //pack.commands[cmdPos] = blockCommand;
                CCLOG("update a dirty block at %d %d", localPos.x, localPos.y);

                dirtyBlock.renderDirty = false;
            }
            if (tag.isAllClean())
            {
                _registry.remove<DirtyChunkTag>(entity);
            }
        });
}