#pragma once
#include "entt/entt.hpp"
#include "components/block/block_component.h"
#include "block_system_manager.h"

class Vec2i;

/**
* @brief 生成区块的方块。调度加载系统和生成系统。
*
* 使用噪声库生成的噪声（主要是柏林噪声）进行逐方块的生成。
* 性能消耗中等，不建议在每帧调用。
*/
class BlockGenSystem : public ISystem
{
public:
	BlockGenSystem(entt::registry& registry, entt::dispatcher& dispatcher);
	~BlockGenSystem();
	void update(float delta);
private:
	BlockState genBlockAt(const Vec2i& blockPos);
	BlockState genWallAt(const Vec2i& blockPos);
};