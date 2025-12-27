#include "components/block/block_component.h"
#include "components/block/chunk_render.h"
#include "core/assets_manager.h"
#include "block_mining_system.h"

BlockMiningSystem::BlockMiningSystem(entt::registry& registry, entt::dispatcher& dispatcher) 
    : ISystem(registry, dispatcher), _assetManager(_registry.ctx().get<AssetManager>()) {}
BlockMiningSystem::~BlockMiningSystem() = default;

void BlockMiningSystem::update(float delta)
{
    mining(delta);
    render();
}

void BlockMiningSystem::mining(float delta)
{
    auto view = _registry.view<BlockEntityHead, MiningTag>();
    view.each([&](entt::entity entity, BlockEntityHead& blockEntity, MiningTag& tag)
        {
            auto progress = _registry.try_get<MiningProgress>(entity);
            if (!progress)
            {
                progress = &(blockEntity.saveEmplace<MiningProgress>(_registry, entity));
            }
            // ∆∆ªµ∑ΩøÈ
            progress->progress += tag.factor * delta;

            // Õ⁄æÚÕÍ≥…£¨≥¢ ‘∆∆ªµ
            if (progress->progress > 1.0f)
            {
                auto& blockWorld = _registry.ctx().get<BlockWorld>();
                if (blockWorld.tryDestroy(blockEntity.layer , blockEntity.blockPos, tag.miner))
                {
                    blockEntity.saveRemove<MiningProgress>(_registry, entity);
                }
            }
            blockEntity.saveRemove<MiningTag>(_registry, entity);
        });
}

void BlockMiningSystem::render()
{   
    // ‘ˆº”‰÷»æ÷∏¡Ó
    auto needToAdd = _registry.view<BlockEntityHead, MiningProgress>(entt::exclude<CustomcommandPack>);
    needToAdd.each([&](entt::entity entity, BlockEntityHead& block, MiningProgress& progress)
        {
            auto texture = _registry.ctx().get<AssetManager>().getTexture("blocks\\textures\\mining2.png");
            auto command = new BlockBatchCommand(10, block.blockPos, texture);
            command->setUseTransform(false);
            auto& pack = block.saveEmplace<CustomcommandPack>(_registry, entity);
            pack.commands.push_back(command);
        });

    // “∆≥˝Œﬁ–ßµƒ‰÷»æ÷∏¡Ó
    auto needToRemove = _registry.view<BlockEntityHead, CustomcommandPack>(entt::exclude<MiningProgress>);
    needToRemove.each([&](entt::entity entity, BlockEntityHead& block, CustomcommandPack& pack)
        {
            block.saveRemove<CustomcommandPack>(_registry, entity);
        });
}
