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
            auto miningProgress = _registry.try_get<MiningProgress>(entity);
            if (!miningProgress)
            {
                miningProgress = &(blockEntity.saveEmplace<MiningProgress>(_registry, entity));
            }
            
            // 方块硬度
            float hardness = _assetManager.getBlockConfig(blockEntity.id).getOriginValOr<float>("mining", "hardness", 0.00001f);

            float progress = miningProgress->progress;              // 进度
            float deltaProgress = (tag.factor / hardness) * delta;  // 挖掘的进度增量

            // 进度变化，需要渲染
            if (getProgressLevel(progress) != getProgressLevel(progress + deltaProgress))
            {
                _registry.emplace<MiningProgressRenderChangeTag>(entity);
            }
            // 破坏方块
            miningProgress->progress += deltaProgress;

            // 挖掘完成，尝试破坏
            if (miningProgress->progress > 1.0f)
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
    // 增加渲染指令
    auto needToAdd = _registry.view<BlockEntityHead, MiningProgress>(entt::exclude<CustomcommandPack>);
    needToAdd.each([&](entt::entity entity, BlockEntityHead& block, MiningProgress& progress)
        {
            auto& pack = block.saveEmplace<CustomcommandPack>(_registry, entity);

            auto texture = getProgressTexture(getProgressLevel(progress.progress));
            auto command = new BlockBatchCommand(getZOrder(block.layer) + 1, block.blockPos, texture);
            command->setUseTransform(false);

            pack.commands.push_back(command);
        });
    auto needToUpdate = _registry.view<BlockEntityHead, CustomcommandPack, MiningProgress, MiningProgressRenderChangeTag>();
    needToUpdate.each([&](entt::entity entity, BlockEntityHead& block, CustomcommandPack& pack, MiningProgress& miningProgress)
        {
            pack.releaseAllCommand();

            auto texture = getProgressTexture(getProgressLevel(miningProgress.progress));
            auto command = new BlockBatchCommand(getZOrder(block.layer) + 1, block.blockPos, texture);
            command->setUseTransform(false);

            pack.commands.push_back(command);

            _registry.remove<MiningProgressRenderChangeTag>(entity);
        });

    // 移除无效的渲染指令
    auto needToRemove = _registry.view<BlockEntityHead, CustomcommandPack>(entt::exclude<MiningProgress>);
    needToRemove.each([&](entt::entity entity, BlockEntityHead& block, CustomcommandPack& pack)
        {
            block.saveRemove<CustomcommandPack>(_registry, entity);
        });
}

int BlockMiningSystem::getProgressLevel(float progress)
{
    return floor(progress * 4);
}

cocos2d::Texture2D* BlockMiningSystem::getProgressTexture(int level)
{
    if (level == 0)
    {
        return _assetManager.getTexture("blocks\\textures\\air.png");
    }
    else if(level == 1)
    {
        return _assetManager.getTexture("blocks\\textures\\mining1.png");
    }
    else if(level == 2)
    {
        return _assetManager.getTexture("blocks\\textures\\mining2.png");
    }
    return _assetManager.getTexture("blocks\\textures\\mining3.png");
}
