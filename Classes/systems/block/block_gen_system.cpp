#include "block_gen_system.h"
#include "components/block/block_component.h"
#include "noise/noise.h"

#define BLOCK_GEN_LOG 0

BiosProperties::BiosProperties(int seed) : _seed(seed)
{
    _heightPerlin.SetSeed(seed);
    _heightPerlin.SetFrequency(0.003);
    _heightPerlin.SetLacunarity(0.5);
    _heightPerlin.SetOctaveCount(2);
    _heightPerlin.SetPersistence(0.3);

    /*
    _heightCurve.SetSourceModule(0, _heightPerlin);
    _heightCurve.AddControlPoint(-1, -1);
    _heightCurve.AddControlPoint(-0.5, -0.8);
    _heightCurve.AddControlPoint(0.0, -0.6);
    _heightCurve.AddControlPoint(0.5, 0.1);
    _heightCurve.AddControlPoint(1, 1);
    */

    _heightScaleBias.SetSourceModule(0, _heightPerlin);
    _heightScaleBias.SetBias(20 * CHUNK_SIZE);
    _heightScaleBias.SetScale(10 * CHUNK_SIZE);

    _tempPerlin.SetSeed(seed + 1);
    _tempPerlin.SetFrequency(0.0001);
    _tempPerlin.SetLacunarity(0.5);
    _tempPerlin.SetOctaveCount(1);
    _tempPerlin.SetPersistence(0.5);

    _tempScaleBias.SetSourceModule(0, _heightPerlin);
    _tempScaleBias.SetBias(-20);
    _tempScaleBias.SetScale(40);
}

int BiosProperties::getHeight(int x) const
{
    return _heightScaleBias.GetValue(x, 0, fixedZ);
}

double BiosProperties::getTemp(int x) const
{
    return _tempScaleBias.GetValue(x, 0, fixedZ);
}

StructureProperties::StructureProperties(int seed, const BiosProperties& biosProperties)
    : _seed(seed), _biosProperties(biosProperties) {}

Vec2i StructureProperties::getStructurePos(int x) const
{
    return Vec2i();
}

std::vector<Vec2i> StructureProperties::getAllStructurePosInChunk(int chunkX) const
{
    return std::vector<Vec2i>();
}

GlobalProperties::GlobalProperties(int seed)
{
    _seed = seed;
    _bioProperties = std::make_unique<BiosProperties>(seed);
    _structProperties = std::make_unique<StructureProperties>(seed, *_bioProperties);
}

const BiosProperties& GlobalProperties::getBioGenerator() const
{
    return *_bioProperties;
}

const StructureProperties& GlobalProperties::getStructProperties() const
{
    return *_structProperties;
}

TerrainGenerator::TerrainGenerator(const GlobalProperties& globalProperties) : BlockGenerator(globalProperties)
{
    _perlin.SetSeed(_globalProperties.seed());
    _perlin.SetFrequency(0.05);
    _perlin.SetLacunarity(0.5);
    _perlin.SetOctaveCount(3);
    _perlin.SetPersistence(0.5);
    
    _scaleBias.SetSourceModule(0, _perlin);
    _scaleBias.SetBias(10 * CHUNK_SIZE);
    _scaleBias.SetScale(10);
}

std::pair<ReplaceType, BlockState> TerrainGenerator::genAt(const Vec2i& blockPos) const
{
    auto& bio = _globalProperties.getBioGenerator();

    int height = bio.getHeight(blockPos.x);

    if (blockPos.y > height)
    {
        return { FORCE, BlockState::AIR};
    }
    else if (blockPos.y >= height - 8 + 3 * _perlin.GetValue(blockPos.x, 0, fixedZ))
    {
        return { OCCUPY, BlockState("dirt", 0) };
    }
    else
    {
        return { SKIP, BlockState("stone", 0) };
    }
}


BedrockGenerator::BedrockGenerator(const GlobalProperties& globalProperties)
    : BlockGenerator(globalProperties)
{
    _perlin.SetSeed(_globalProperties.seed());
    _perlin.SetFrequency(0.5);
    _perlin.SetLacunarity(0.5);
    _perlin.SetOctaveCount(1);
    _perlin.SetPersistence(0.5);

    _scaleBias.SetSourceModule(0, _perlin);
    _scaleBias.SetBias(1);
    _scaleBias.SetScale(2);
}

std::pair<ReplaceType, BlockState> BedrockGenerator::genAt(const Vec2i& blockPos) const
{
    int bedrockheight = _scaleBias.GetValue(blockPos.x, 0, fixedZ);
    if (blockPos.y > bedrockheight)
    {
        return { SKIP, BlockState::AIR };
    }
    else
    {
        return { FORCE, BlockState("bedrock", 0) };
    }
}

CaveGenerator::CaveGenerator(const GlobalProperties& globalProperties) : BlockGenerator(globalProperties)
{
    _perlinOrigin.SetSeed(_globalProperties.seed());
    _perlinOrigin.SetFrequency(0.07);
    _perlinOrigin.SetLacunarity(2);
    _perlinOrigin.SetOctaveCount(3);
    _perlinOrigin.SetPersistence(0.3);

    _turbulance.SetSourceModule(0, _perlinOrigin);
    _turbulance.SetSeed(_globalProperties.seed());
    _turbulance.SetFrequency(0.1);
    _turbulance.SetPower(4);
    _turbulance.SetRoughness(2);

    _scaleBias.SetSourceModule(0, _turbulance);
    _scaleBias.SetBias(-5);
    _scaleBias.SetScale(10);
}

std::pair<ReplaceType, BlockState> CaveGenerator::genAt(const Vec2i& blockPos) const
{
    if (_perlinOrigin.GetValue(blockPos.x, blockPos.y, 0) > 0.3)
    {
        return { FORCE, BlockState::AIR };
    }
    else
    {
        return { OCCUPY, BlockState("stone", 0) };
    }
}


OreGenerator::OreGenerator(const GlobalProperties& globalProperties, entt::id_type oreId,
    double freq, double density, double min, double max, double heighest)
    : BlockGenerator(globalProperties)
{
    _oreId = oreId;
    _density = density;
    _min = min;
    _max = max;
    _heighest = heighest;

    _perlin.SetSeed(_globalProperties.seed());
    _perlin.SetFrequency(freq);
    _perlin.SetLacunarity(2);
    _perlin.SetOctaveCount(1);
    _perlin.SetPersistence(0.5);
    
    _scaleBias.SetSourceModule(0, _perlin);
    _scaleBias.SetBias(0);
}

std::pair<ReplaceType, BlockState> OreGenerator::genAt(const Vec2i& blockPos) const
{
    if (blockPos.y >= _max) return { SKIP, BlockState::AIR };
    else if(blockPos.y<= _min) return { SKIP, BlockState::AIR};
    
    if (blockPos.y >= _heighest)
    {
        _scaleBias.SetScale(lerp(0, 1, (_max - blockPos.y) / (_max - _heighest)));
    }
    else
    {
        _scaleBias.SetScale(lerp(0, 1, (blockPos.y - _min) / (_heighest - _min)));
    }

    if (_scaleBias.GetValue(blockPos.x, blockPos.y, fixedZ) > _density)
    {
        return { OCCUPY, BlockState(_oreId, 0) };
    }
    else
    {
        return { SKIP, BlockState::AIR };
    }
}

BlockDecorator::BlockDecorator(const GlobalProperties& globalProperties) 
    : _globalProperties(globalProperties) {}

GrassDecorator::GrassDecorator(const GlobalProperties& globalProperties, double density)
    : BlockDecorator(globalProperties)
{
    _density = density;
    _perlin.SetSeed(_globalProperties.seed());
    _perlin.SetFrequency(0.3);
    _perlin.SetLacunarity(2);
    _perlin.SetOctaveCount(1);

    _grassIds.push_back({ entt::hashed_string("weed"), 0});
    _grassIds.push_back({ entt::hashed_string("weed"), 1});
    _grassIds.push_back({ entt::hashed_string("weed"), 2});

    _grassIds.push_back({ entt::hashed_string("flower"), 0});
    _grassIds.push_back({ entt::hashed_string("flower"), 1});
    _grassIds.push_back({ entt::hashed_string("flower"), 2});
}

void GrassDecorator::decorateAt(ChunkBlocks& blocks, const Vec2i& chunkPos) const
{
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        int height = _globalProperties.getBioGenerator().getHeight(chunkPos.x * CHUNK_SIZE + x);
        int localHeight = height - chunkPos.y * CHUNK_SIZE;
        if (localHeight < 0 || localHeight >= CHUNK_SIZE)
        {
            continue;
        }
        Vec2i localPos = { x,  localHeight };
        if (blocks.getBlockAt(localPos).id == entt::hashed_string("dirt"))
        {
            blocks.setBlockAt(localPos, BlockState("grass_block", 0));

            if (localHeight + 1 < CHUNK_SIZE)
            {
                if (_perlin.GetValue(x + chunkPos.x * CHUNK_SIZE, fixedZ, fixedZ) > _density)
                {
                    blocks.setBlockAt(localPos + Vec2i(0, 1), BlockState(_grassIds[rand() % _grassIds.size()]));
                }
            }
        }
        
    }
}

BlockGenSystem::BlockGenSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : ISystem(registry, dispatcher)
{
    initGenerator();
}

BlockGenSystem::~BlockGenSystem() {}

void BlockGenSystem::update(float delta)
{
    // ��ȡδ���ɷ��������
    auto view = _registry.view<Position, ChunkHead, NeedGen>();
    view.each([&](entt::entity entity, Position& pos, ChunkHead& head)
        {
#if BLOCK_GEN_LOG
            CCLOG("Gen chunk at %d %d", pos.getPostion().x/BLOCK_SIZE, pos.getPostion().y/BLOCK_SIZE);
#endif
            auto& blocks = _registry.emplace<ChunkBlocks>(entity);
            // ���ɷ���
            for (int y = 0; y < CHUNK_SIZE; y++)
            {
                for (int x = 0; x < CHUNK_SIZE; x++)
                {
                    auto blockPos = BlockLayer::worldPosToBlockPos(pos) + Vec2i(x,y);
                    if (head.getLayerType() == LayerType::BLOCK)
                    {
                        blocks.setBlockAt({ x,y }, genBlockAt(blockPos));
                    }
                    else
                    {
                        blocks.setBlockAt({ x,y }, genWallAt(blockPos));
                    }
                }
            }

            for (auto& decorator: _blockDecorators)
            {
                decorator->decorateAt(blocks, BlockLayer::worldPosToChunkPos(pos));
            }

            _registry.remove<NeedGen>(entity);
        });
}

void BlockGenSystem::initGenerator()
{
    _seed = rand();
    _globalProperties = std::make_unique<GlobalProperties>(_seed);
    
    _blockGenLayers.push_back(std::make_unique<TerrainGenerator>(*_globalProperties));
    _blockGenLayers.push_back(std::make_unique<OreGenerator>(*_globalProperties,
        entt::hashed_string("copper_ore"),
        0.15, 0.5, 60, 350, 200));
    _blockGenLayers.push_back(std::make_unique<OreGenerator>(*_globalProperties,
        entt::hashed_string("silver_ore"),
        0.2, 0.7, 20, 300, 120));
    _blockGenLayers.push_back(std::make_unique<OreGenerator>(*_globalProperties,
        entt::hashed_string("lead_ore"),
        0.25, 0.7, 20, 250, 50));
    _blockGenLayers.push_back(std::make_unique<OreGenerator>(*_globalProperties,
        entt::hashed_string("platinum_ore"),
        0.35, 0.8, 0, 150, 30));
    _blockGenLayers.push_back(std::make_unique<CaveGenerator>(*_globalProperties));
    _blockGenLayers.push_back(std::make_unique<BedrockGenerator>(*_globalProperties));

    _blockDecorators.push_back(std::make_unique<GrassDecorator>(*_globalProperties, 0.3));
}

BlockState BlockGenSystem::genBlockAt(const Vec2i& blockPos)
{
    BlockState result = BlockState::AIR;
    bool isOccupied = false;
    for (int layerIndex  = 0; layerIndex < _blockGenLayers.size(); layerIndex++)
    {
        auto& layer = _blockGenLayers[layerIndex];
        auto genResult = layer->genAt(blockPos);
        
        if (genResult.first == FORCE)
        {
            isOccupied = true;
            result = genResult.second;
        }
        else if (genResult.first == OCCUPY && isOccupied == false)
        {
            isOccupied = true;
            result = genResult.second;
        }
        else if (genResult.first == SKIP && isOccupied == false)
        {
            result = genResult.second;
        }
    }
    return result;
}

BlockState BlockGenSystem::genWallAt(const Vec2i& blockPos)
{

    return BlockState::AIR;
}
