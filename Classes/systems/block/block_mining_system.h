#pragma once
#include "block_system_manager.h"
#include "entt/entt.hpp"

namespace cocos2d
{
    class Texture2D;
}

class AssetManager;

class BlockMiningSystem : public ISystem
{
public:
    BlockMiningSystem(entt::registry& registry, entt::dispatcher& dispatcher);
    ~BlockMiningSystem();
    void update(float delta);
private:
    void mining(float delta);
    void render();
    int getProgressLevel(float progress);
    cocos2d::Texture2D* getProgressTexture(int level);
    AssetManager& _assetManager;
};