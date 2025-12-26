#include "components/block/block_component.h"
#include "components/block/chunk_render.h"
#include "core/assets_manager.h"
#include "block_mining_system.h"

BlockMiningSystem::BlockMiningSystem(entt::registry& registry, entt::dispatcher& dispatcher) 
    : ISystem(registry, dispatcher), _assetManager(_registry.ctx().get<AssetManager>()) {}
BlockMiningSystem::~BlockMiningSystem() = default;

void BlockMiningSystem::update(float delta)
{
    mining();
    render();
}

void BlockMiningSystem::mining()
{
    auto view = _registry.view<ActiveBlock, MiningTag>();
    view.each([&](entt::entity entity, ActiveBlock& block, MiningTag& tag)
        {
            auto progress = _registry.try_get<MiningProgress>(entity);
            if (!progress)
            {
                progress = &(block.saveEmplace<MiningProgress>(_registry, entity));
            }
            // ∆∆ªµ∑ΩøÈ
            progress->progress += tag.factor;

            // Õ⁄æÚÕÍ≥…£¨≥¢ ‘∆∆ªµ
            if (progress->progress > 1.0f)
            {
                auto& blockWorld = _registry.ctx().get<BlockWorld>();
                if (blockWorld.tryDestroy(block.blockPos, tag.minier))
                {
                    block.saveRemove<MiningProgress>(_registry, entity);
                }
            }
            block.saveRemove<MiningTag>(_registry, entity);
        });
}

void BlockMiningSystem::render()
{   
    // ‘ˆº”‰÷»æ÷∏¡Ó
    auto needToAdd = _registry.view<ActiveBlock, MiningProgress>(entt::exclude<CustomcommandPack>);
    needToAdd.each([&](entt::entity entity, ActiveBlock& block, MiningProgress& progress)
        {
            auto texture = _registry.ctx().get<AssetManager>().getTexture("blocks\\textures\\air.png");
            auto command = new BlockBatchCommand(10, block.blockPos, texture);
            auto& pack = block.saveEmplace<CustomcommandPack>(_registry, entity);
            pack.commands.push_back(command);
        });

    // À¢–¬‰÷»æ÷∏¡Ó
    auto needToUpdate = _registry.view<ActiveBlock, MiningProgress, CustomcommandPack>();
    needToUpdate.each([&](entt::entity entity, ActiveBlock& block, MiningProgress& progress, CustomcommandPack& pack)
        {
            auto command = pack.commands[0];
            auto blockCommand = dynamic_cast<BlockBatchCommand*>(command);
            blockCommand->setTexture(getProgressTexture(progress.progress));
        });

    // “∆≥˝Œﬁ–ßµƒ‰÷»æ÷∏¡Ó
    auto needToRemove = _registry.view<ActiveBlock, CustomcommandPack>(entt::exclude<MiningProgress>);
    needToRemove.each([&](entt::entity entity, ActiveBlock& block, CustomcommandPack& pack)
        {
            block.saveRemove<CustomcommandPack>(_registry, entity);
        });
}

cocos2d::Texture2D* BlockMiningSystem::getProgressTexture(float progress)
{
    if (progress <= 0)
    {
        return _assetManager.getTexture("blocks\\textures\\air.png");
    }
    if (progress <= 0.33f)
    {
        return _assetManager.getTexture("blocks\\textures\\mining1.png");
    }
    if (progress <= 0.66f)
    {
        return _assetManager.getTexture("blocks\\textures\\mining2.png");
    }
    return _assetManager.getTexture("blocks\\textures\\mining3.png");
}

