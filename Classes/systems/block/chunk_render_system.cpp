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

    updateDirtyBlock();
}

BlockCommand* ChunkRenderCommandSystem::generateCommand(const ChunkBlocks& blocks, const Vec2i& localPos)
{
    auto& assetManager = _registry.ctx().get<AssetManager>();
    auto blockID = blocks.getBlockAt(localPos);
    auto config = assetManager.getBlockConfig(blockID);
    std::string texture = tools::get_str_or(*config, "texture", "a_block.bmp");
    // 创建一个方块渲染指令
    return new BlockCommand(
        localPos,
        assetManager.getTexture(texture),
        nullptr
    );
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
            auto& commands = _registry.emplace<CustomcommandPack>(entity).commands;

            // 为区块中的每个方块创建渲染指令：
            // command[ x + y * CHUNK_SIZE ]
            for (int y = 0; y < CHUNK_SIZE; y++)
            {
                for (int x = 0; x < CHUNK_SIZE; x++)
                {
                    auto blockCommand = new BlockBatchCommand(chunkPos * CHUNK_SIZE + Vec2i(x, y));
                    blockCommand->setUseTransform(false);
                    commands.push_back(blockCommand);
                }
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
                auto blockCommand = generateCommand(blocks, localPos);
                delete pack.commands[cmdPos];
                pack.commands[cmdPos] = blockCommand;
                CCLOG("update a dirty block at %d %d", localPos.x, localPos.y);

                dirtyBlock.renderDirty = false;
            }
            if (tag.isAllClean())
            {
                _registry.remove<DirtyChunkTag>(entity);
            }
        });
}