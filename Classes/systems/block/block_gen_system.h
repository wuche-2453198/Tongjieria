#pragma once
#include "entt/entt.hpp"
#include "components/block/block_component.h"
#include "noise/noise.h"
#include "block_system_manager.h"

class Vec2i;

static inline const float fixedX = 1.0f;
static inline const float fixedY = 1.0f;
static inline const float fixedZ = 1.0f;

enum ReplaceType
{
	FORCE,	// 强制替换，不管该位置是否被占用
	OCCUPY, // 占用该位置，如果该位置为空
	SKIP,     // 跳过该位置，不进行任何操作
	FORCE_IF_AIR
};

class BiosProperties
{
public:
	BiosProperties(int seed);
	~BiosProperties() = default;
	int getHeight(int x) const;
	double getTemp(int x) const;
protected:
	int _seed;
    noise::module::Perlin _heightPerlin;
	noise::module::Curve _heightCurve;
	noise::module::ScaleBias _heightScaleBias;
	noise::module::Perlin _tempPerlin;
	noise::module::ScaleBias _tempScaleBias;
};

class StructureProperties
{
public:
	StructureProperties(int seed, const BiosProperties& biosProperties);
	~StructureProperties() = default;
    Vec2i getStructurePos(int x) const;
	std::vector<Vec2i> getAllStructurePosInChunk(int chunkX) const;
protected:
	int _seed;
	const BiosProperties& _biosProperties;
};

class GlobalProperties
{
public:
	GlobalProperties(int seed);
	~GlobalProperties() = default;
	const BiosProperties& getBioGenerator() const;
	const StructureProperties& getStructProperties() const;
	int seed() const { return _seed; }
protected:
	int _seed;

	std::unique_ptr<BiosProperties> _bioProperties;
	std::unique_ptr<StructureProperties> _structProperties;
};

class BlockGenerator
{
public:
	BlockGenerator(const GlobalProperties& globalProperties) : _globalProperties(globalProperties) {};
	virtual ~BlockGenerator() = default;
	virtual std::pair<ReplaceType, BlockState> genAt(const Vec2i& blockPos) const = 0;

protected:
	const GlobalProperties& _globalProperties;
};

class TerrainGenerator : public BlockGenerator
{
public:
	TerrainGenerator(const GlobalProperties& globalProperties);
	virtual std::pair<ReplaceType, BlockState> genAt(const Vec2i& blockPos) const override;
protected:
	mutable noise::module::Perlin _perlin;
	mutable noise::module::ScaleBias _scaleBias;
};

class BedrockGenerator : public BlockGenerator
{
public:
	BedrockGenerator(const GlobalProperties& globalProperties);
	virtual std::pair<ReplaceType, BlockState> genAt(const Vec2i& blockPos) const override;
protected:
	noise::module::Perlin _perlin;
	noise::module::ScaleBias _scaleBias;
};

class CaveGenerator : public BlockGenerator
{
public:
	CaveGenerator(const GlobalProperties& globalProperties);
	virtual std::pair<ReplaceType, BlockState> genAt(const Vec2i& blockPos) const override;
protected:
	noise::module::Perlin _perlinOrigin;
	noise::module::Turbulence _turbulance;
	noise::module::ScaleBias _scaleBias;
};

class OreGenerator : public BlockGenerator
{
public:
	OreGenerator(const GlobalProperties& globalProperties, entt::id_type oreId,
		double freq, double density, double min, double max, double heighest);
	virtual std::pair<ReplaceType, BlockState> genAt(const Vec2i& blockPos) const override;
protected:
	static double lerp(double a, double b, double t)
	{
		return a + (b - a) * t;
	}

	mutable noise::module::Perlin _perlin;
	mutable noise::module::ScaleBias _scaleBias;

	entt::id_type _oreId;

	double _density;
	double _min;
	double _max;
	double _heighest;
};

class BlockDecorator
{
public:
	BlockDecorator(const GlobalProperties& globalProperties);
	virtual ~BlockDecorator() = default;
	virtual void decorateAt(ChunkBlocks& block, const Vec2i& chunkPos) const = 0;
protected:
	const GlobalProperties& _globalProperties;
};

class GrassDecorator : public BlockDecorator
{
public:
	GrassDecorator(const GlobalProperties& globalProperties, double density);
	virtual void decorateAt(ChunkBlocks& block, const Vec2i& chunkPos) const override;
protected:
	double _density;
	std::vector<BlockState> _grassIds;
	noise::module::Perlin _perlin;
};

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

	static inline const std::string AIR = "air";

	void initGenerator();

	BlockState genBlockAt(const Vec2i& blockPos);
	BlockState genWallAt(const Vec2i& blockPos);
	
	int _seed;
	std::unique_ptr<GlobalProperties> _globalProperties;
	std::vector<std::unique_ptr<BlockGenerator>> _blockGenLayers;
	std::vector<std::unique_ptr<BlockDecorator>> _blockDecorators;
};