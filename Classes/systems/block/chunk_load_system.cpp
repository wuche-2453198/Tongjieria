#include "components/block/block_component.h"
#include "systems/block_layer/block_layer.h"
#include "chunk_load_system.h"

#include "debug_system.h"

#define CHUNK_VIEW_ENABEL 0

ChunkLoadSystem::ChunkLoadSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : ISystem(registry, dispatcher),
    _blockLayer(_registry.ctx().get<BlockLayer>())
{

}
ChunkLoadSystem::~ChunkLoadSystem() {}

void ChunkLoadSystem::update(float delta)
{
    AddNewChunk();
    updatePriority();

    auto& mappings = _blockLayer.getChunkMappings();

    DebugSystem::drawNodes[2]->clear();
    auto view = _registry.view<Position, ChunkHead>();
    
#if CHUNK_VIEW_ENABEL == 1
    view.each([&](const Position& pos, const ChunkHead& head)
        {
            Vec2i chunkPos = BlockLayer::worldPosToChunkPos(pos);
            cocos2d::Vec2 lowLeft = chunkPos * BLOCK_SIZE * CHUNK_SIZE;
            cocos2d::Vec2 highRight = (chunkPos + Vec2i(1, 1)) * BLOCK_SIZE * CHUNK_SIZE;
            DebugSystem::drawNodes[2]->drawRect(lowLeft, highRight, cocos2d::Color4F::GREEN);
        });
#endif // 
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
                    Vec2i chunkPos = { x,y };
                    if (!_blockLayer.hasChunkExist(chunkPos))
                    {
                        addChunk(chunkPos);
                    }
                }
            }
        });
}

void ChunkLoadSystem::addChunk(const Vec2i& chunkPos)
{
    // 如果添加的位置过高或过低，或区块已经存在，直接返回
    if (!isValiedChunkPos(chunkPos) || _blockLayer.hasChunkExist(chunkPos)) return;

    // 创建实体
    entt::entity entity = _registry.create();

    // 添加位置和区块头
    _registry.emplace<Position>(entity, chunkPos * BLOCK_SIZE * CHUNK_SIZE);
    _registry.emplace<ChunkHead>(entity);

    // 更新区块索引
    _blockLayer.addChunk(chunkPos, entity);
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