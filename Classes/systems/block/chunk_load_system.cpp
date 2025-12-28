#include "core/block_world.h"
#include "components/block/block_component.h"
#include "systems/block_layer/block_layer.h"
#include "chunk_load_system.h"

#include "debug_system.h"

#define CHUNK_VIEW_ENABEL 0;

ChunkLoadSystem::ChunkLoadSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : ISystem(registry, dispatcher), _blockWorld(_registry.ctx().get<BlockWorld>())
{

}
ChunkLoadSystem::~ChunkLoadSystem() {}

void ChunkLoadSystem::update(float delta)
{
    AddNewChunk();
    updatePriority();

    
#if CHUNK_VIEW_ENABEL
    auto& mappings = _blockWorld.getLayer(LayerType::BLOCK).getChunkMappings();

    auto drawNode = DebugSystem::getDrawNode(_registry, "chunk");
    drawNode->clear();
    auto view = _registry.view<Position, ChunkHead>();

    view.each([&](const Position& pos, const ChunkHead& head)
        {
            Vec2i chunkPos = BlockLayer::worldPosToChunkPos(pos);
            cocos2d::Vec2 lowLeft = chunkPos * BLOCK_SIZE * CHUNK_SIZE;
            cocos2d::Vec2 highRight = (chunkPos + Vec2i(1, 1)) * BLOCK_SIZE * CHUNK_SIZE;
            drawNode->drawRect(lowLeft, highRight, cocos2d::Color4F::GREEN);
        });
#endif 
}

void ChunkLoadSystem::AddNewChunk()
{
    auto view = _registry.view<Position, LoadingTicket>();
    view.each([&](const Position& worldPos, const LoadingTicket& ticket)
        {
            Vec2i upperLeft = BlockLayer::worldPosToChunkPos(worldPos) +
                Vec2i(-1, 1) * (ticket.radius - 1);
            Vec2i lowerRight = BlockLayer::worldPosToChunkPos(worldPos) +
                Vec2i(1, -1) * (ticket.radius - 1);
            for (int y = upperLeft.y; y >= lowerRight.y; y--)
            {
                for (int x = upperLeft.x; x <= lowerRight.x; x++)
                {
                    AddChunk(LayerType::BLOCK, Vec2i(x, y));
                    AddChunk(LayerType::WALL, Vec2i(x, y));
                }
            }
        });
}

void ChunkLoadSystem::AddChunk(LayerType layerType, const Vec2i& chunkPos)
{
    auto& layer = _blockWorld.getLayer(layerType);
    if (!layer.hasChunkExist(chunkPos))
    {
        // 如果添加的位置过高或过低，或区块已经存在，直接返回
        if (!isValiedChunkPos(chunkPos) || layer.hasChunkExist(chunkPos))
        {
            return;
        }
        layer.addChunk(chunkPos);
    }
}

void ChunkLoadSystem::updatePriority()
{
    auto ticketView = _registry.view<Position, LoadingTicket>();
    auto chunkView = _registry.view<Position, ChunkHead>();
    chunkView.each([&](Position& pos, ChunkHead& head)
        {
            head.setPriority(-1000);
        });

    ticketView.each([&](Position& ticketPos, LoadingTicket& ticket)
        {
            Vec2i loadingCenter = BlockLayer::worldPosToChunkPos(ticketPos);
            chunkView.each([&](Position& chunkWorldPos, ChunkHead& chunkHead)
                {
                    Vec2i chunkPos = BlockLayer::worldPosToChunkPos(chunkWorldPos);
                    int newPriority = ticket.radius - inLayer(loadingCenter, chunkPos);
                    chunkHead.setPriority(std::max(chunkHead.getPriority(), newPriority));
                });
        });
}

bool ChunkLoadSystem::isValiedChunkPos(const Vec2i& chunkPos)
{
    return LOWWER_LIMIT <= chunkPos.y && chunkPos.y <= UPPER_LIMIT;
}

int ChunkLoadSystem::inLayer(const Vec2i& center, const Vec2i& chunkPos)
{
    // 横纵距离的绝对值
    return std::max(labs(center.x - chunkPos.x), labs(center.y - chunkPos.y));
}