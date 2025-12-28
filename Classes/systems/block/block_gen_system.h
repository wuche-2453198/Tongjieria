#pragma once
#include "entt/entt.hpp"
#include "components/block/block_component.h"
#include "noise/noise.h"
#include "block_system_manager.h"

class Vec2i;

static inline const float fixedX = 1.0f;
static inline const float fixedY = 1.0f;
static inline const float fixedZ = 1.0f;

/**
* @brief 方块生成器的生成标签
*/
enum ReplaceType
{
	FORCE,	// 强制替换，不管该位置是否被占用
	OCCUPY, // 占用该位置，如果该位置为空
	SKIP,     // 跳过该位置，不进行任何操作
};

enum Bio
{
	SNOW,
	PLAINS,
    DESERT,
};

/**
* @brief 生物群系生成器
* 
* 负责生成地形高度和温度
*/
class BiosProperties
{
public:
	BiosProperties(int seed);
	~BiosProperties() = default;
	int getHeight(int x) const;
	double getTemp(int x) const;
	Bio getBio(int x) const;
protected:
	int _seed;
    noise::module::Perlin _heightPerlin;
	noise::module::Perlin _heightPerlinMix;
	
	noise::module::ScaleBias _heightScaleBias;

	noise::module::Perlin _tempPerlin;
	noise::module::ScaleBias _tempScaleBias;
};

/**
* @brief 结构生成器
* 
* 负责生成结构的位置
*/
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

/**
* @brief 全局属性
* 
* 包含种子和生物群系生成器、结构生成器。
* 提供全局的生成参数。
*/
class GlobalProperties
{
public:
	GlobalProperties(int seed);
	~GlobalProperties() = default;
	const BiosProperties& getBioGenerator() const;
	const StructureProperties& getStructProperties() const;
	int seed() const { return _seed; }
protected:
	int _seed;	///< 生成器的种子

	std::unique_ptr<BiosProperties> _bioProperties;			///< 生物群系生成器
	std::unique_ptr<StructureProperties> _structProperties; ///< 结构生成器
};

/**
* @brief 方块生成器
* 
* 负责生成方块，这是方块生成的“粗”阶段。会生成一些简单的方块，如泥土、石头等。
*/
class BlockGenerator
{
public:
	BlockGenerator(const GlobalProperties& globalProperties) : _globalProperties(globalProperties) {};
	virtual ~BlockGenerator() = default;

	/**
	* @brief 在指定位置生成方块
	* 
	* 
	*/
	virtual std::pair<ReplaceType, BlockState> genAt(const Vec2i& blockPos) const = 0;
protected:
	const GlobalProperties& _globalProperties;
};

/**
* @brief 地形生成器
* 
* 生成简单的地形，包含地形起伏和泥土等
*/
class TerrainGenerator : public BlockGenerator
{
public:
	TerrainGenerator(const GlobalProperties& globalProperties);
	virtual std::pair<ReplaceType, BlockState> genAt(const Vec2i& blockPos) const override;
protected:

	std::vector<entt::id_type> _dirtTypes;

	mutable noise::module::Perlin _perlin;
	mutable noise::module::ScaleBias _scaleBias;
};

/**
* @brief 基岩生成器
* 
* 在世界底部生成基岩
*/
class BedrockGenerator : public BlockGenerator
{
public:
	BedrockGenerator(const GlobalProperties& globalProperties);
	virtual std::pair<ReplaceType, BlockState> genAt(const Vec2i& blockPos) const override;
protected:
	noise::module::Perlin _perlin;
	noise::module::ScaleBias _scaleBias;
};

/**
* @brief 洞穴生成器
* 
* 通过掏空地形生成洞穴
*/
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

/**
* @brief 矿物生成器
* 
* 生成矿石
*/
class OreGenerator : public BlockGenerator
{
public:
	/**
	* @brief 构造函数
	* 
	* @param globalProperties 全局属性
	* @param oreId 矿物ID
	* @param freq 生成频率，控制生成的细碎程度
	* @param density 密度，控制生成的密集程度
	* @param min 最小高度
	* @param max 最大高度
	* @param heighest 概率最高高度
	*/
	OreGenerator(const GlobalProperties& globalProperties, entt::id_type oreId,
		double freq, double density, double min, double max, double heighest);
	virtual std::pair<ReplaceType, BlockState> genAt(const Vec2i& blockPos) const override;
protected:

	static double lerp(double a, double b, double t)
	{
		return a + (b - a) * t; // 线性插值
	}

	mutable noise::module::Perlin _perlin;
	mutable noise::module::ScaleBias _scaleBias;

	entt::id_type _oreId;

	double _density;
	double _min;
	double _max;
	double _heighest;
};

/**
* @brief 方块装饰器
* 
* 负责装饰方块，这是方块生成的“细”阶段。会生成一些复杂的方块，如草、树等。
*/
class BlockDecorator
{
public:
	BlockDecorator(const GlobalProperties& globalProperties);
	virtual ~BlockDecorator() = default;
	virtual void decorateAt(ChunkBlocks& block, const Vec2i& chunkPos) const = 0;
protected:
	const GlobalProperties& _globalProperties;
};

/**
* @brief 草装饰器
*/
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

class WallGenerator : public BlockGenerator
{
public:
	WallGenerator(const GlobalProperties& globalProperties, entt::id_type wallID, int seedOffset);
	virtual std::pair<ReplaceType, BlockState> genAt(const Vec2i& blockPos) const override;
protected:
	entt::id_type wallID;
    noise::module::Perlin _perlin;
	noise::module::ScaleBias _scaleBias;
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

	BlockState genBlockAt(const Vec2i& blockPos, const std::vector<std::unique_ptr<BlockGenerator>>& generators);
	
	int _seed;
	std::unique_ptr<GlobalProperties> _globalProperties;

	std::vector<std::unique_ptr<BlockGenerator>> _blockGenerators;
	std::vector<std::unique_ptr<BlockDecorator>> _blockDecorators;

	std::vector<std::unique_ptr<BlockGenerator>> _wallGenerators;
};