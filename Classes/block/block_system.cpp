#include "block_system.h"
#include "block_layer.h"
#include "block_physics_layer.h"
#include "block_component.h"
#include "block_event.h"
#include "block_behavior.h"
#include "chunk_render.h"
#include "cocos/physics3d/CCPhysics3D.h"
#include "server/assets_manager.h"
#include "server/input_manager.h"
#include "utils/tools.h"
#include "level/world.h"

ISystem::ISystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : _registry(registry), _dispatcher(dispatcher) {}

ISystem::~ISystem() = default;

void ISystem::update(float delta) {}

BlockSystemManager::BlockSystemManager(entt::registry& registry, entt::dispatcher& dispatcher)
    : _registry(registry), _dispatcher(dispatcher)
{
    _debugSystem = std::make_unique<DebugSystem>(_registry, dispatcher);

    // 初始化各个子系统
    _chunkLoadSystem = std::make_unique<ChunkLoadSystem>(_registry, dispatcher);
    _blockLoadSystem = std::make_unique<BlockLoadSystem>(_registry, dispatcher);
    _chunkUnloadSystem = std::make_unique<ChunkUnloadSystem>(_registry, dispatcher);
    _blockPhysicsSystem = std::make_unique<BlockPhysicsSystem>(_registry, dispatcher);
    _chunkRenderCommandSystem = std::make_unique<ChunkRenderCommandSystem>(_registry, dispatcher);
    _blockInteractSystem = std::make_unique<BlockInteractSystem>(_registry, dispatcher);
}

BlockSystemManager::~BlockSystemManager() {}

void BlockSystemManager::update(float delta) 
{
    // 按顺序更新各个子系统
    _chunkLoadSystem->update(delta);
    _blockLoadSystem->update(delta);
    _chunkUnloadSystem->update(delta);
    _blockPhysicsSystem->update(delta);
    _chunkRenderCommandSystem->update(delta);
    _debugSystem->update(delta);
}

LoadingTicket::LoadingTicket() {}

ChunkLoadSystem::ChunkLoadSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : ISystem(registry, dispatcher) ,
    _blockLayer(_registry.ctx().get<BlockLayer>())
{

}
ChunkLoadSystem::~ChunkLoadSystem() {}

void ChunkLoadSystem::update(float delta) 
{
    AddNewChunk();
    updatePriority();
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
    _blockLayer.addChunkID(chunkPos, entity);
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

ChunkUnloadSystem::ChunkUnloadSystem(entt::registry& registry, entt::dispatcher& dispatcher) 
    : ISystem(registry, dispatcher) {}
ChunkUnloadSystem::~ChunkUnloadSystem() = default;

void ChunkUnloadSystem::update(float delta)
{
    auto view = _registry.view<Position, ChunkHead>();
    std::vector<entt::entity> unloadChunks;
    view.each([&](entt::entity entity, Position& pos, ChunkHead& chunk) 
        {
            // 卸载所有低优先级的区块
            if (chunk.getPriority() < ChunkHead::UNLOADING_PRIORITY)
            {
                _registry.ctx().get<BlockLayer>().
                    removeChunkID(BlockLayer::worldPosToChunkPos(pos.getPostion()));
                unloadChunks.push_back(entity);
            }
        });
    // 卸载区块实体
    for (auto entity : unloadChunks)
    {
        _registry.destroy(entity);
    }
}

BlockLoadSystem::BlockLoadSystem(entt::registry& registry, entt::dispatcher& dispatcher) 
    : ISystem(registry,dispatcher) {}
BlockLoadSystem::~BlockLoadSystem() = default;

void BlockLoadSystem::update(float delta) 
{
    // 获取未生成方块的区块
    auto view = _registry.view<Position, ChunkHead>(entt::exclude<ChunkBlocks>);
    view.each([&](entt::entity entity, Position& pos, ChunkHead& head) 
        {
            auto& blocks = _registry.emplace<ChunkBlocks>(entity);
            
            // 为区块中的每个方块生成方块ID
            for (int y = 0; y < CHUNK_SIZE; y++)
            {
                for (int x = 0; x < CHUNK_SIZE; x++)
                {
                    blocks.setBlockAt({ x,y }, WorldGenAt(pos.getPostion() + Vec2i(x, y)));
                }
            }
        });
}

ChunkBlocks BlockLoadSystem::loadChunk()
{
    return ChunkBlocks();
}

ChunkBlocks BlockLoadSystem::generateChunk()
{
    return ChunkBlocks();
}

entt::id_type BlockLoadSystem::WorldGenAt(const Vec2i& pos)
{
    // 一个非常简单的世界生成函数
    if (pos.y > 4)
    {
        return entt::hashed_string("air");
    }
    else if(pos.y>0)
    {
        return entt::hashed_string("dirt");
    }
    else
    {
        return entt::hashed_string("stone");
    }
}

BlockInteractSystem::BlockInteractSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : ISystem(registry, dispatcher)
{
    _behaviorRegistry = std::make_unique<BlockBehaviorRegistry>();

    _dispatcher.sink<BlockDestroyEvent>().connect<&BlockInteractSystem::onBlockDestroyed>(this);
    _dispatcher.sink<BlockPlacedEvent>().connect<&BlockInteractSystem::onBlockPlaced>(this);
    _dispatcher.sink<BlockMinedEvent>().connect<&BlockInteractSystem::onBlockMined>(this);
    _dispatcher.sink<BlockInteractEvent>().connect<&BlockInteractSystem::onBlockInteracted>(this);
}

BlockInteractSystem::~BlockInteractSystem()
{
}

void BlockInteractSystem::onBlockPlaced(const BlockPlacedEvent& event)
{
    // 调用行为
    auto behavior = _behaviorRegistry->getBehavior(event.id);
    if (behavior) behavior->onBlockPlaced(event);

    auto& assetManager = _registry.ctx().get<AssetManager>();
    auto& blockLayer = _registry.ctx().get<BlockLayer>();

    BlockState newState(event.blockPos, event.id);
    blockLayer.setBlockAtBlockPos(event.blockPos, newState);

    auto chunkID = blockLayer.getChunkID(BlockLayer::blockPosToChunkPos(event.blockPos));
    addDirtyTag(chunkID, BlockLayer::blockPosToChunkLocalPos(event.blockPos));
}

void BlockInteractSystem::onBlockDestroyed(const BlockDestroyEvent& event)
{
    // 调用行为
    auto behavior = _behaviorRegistry->getBehavior(event.id);
    if (behavior) behavior->onBlockDestroyed(event);

    auto& assetManager = _registry.ctx().get<AssetManager>();
    auto& blockLayer = _registry.ctx().get<BlockLayer>();

    BlockState newState(event.blockPos, entt::hashed_string("air"));
    blockLayer.setBlockAtBlockPos(event.blockPos, newState);

    auto chunkID = blockLayer.getChunkID(BlockLayer::blockPosToChunkPos(event.blockPos));
    addDirtyTag(chunkID, BlockLayer::blockPosToChunkLocalPos(event.blockPos));
}

void BlockInteractSystem::onBlockMined(const BlockMinedEvent& event)
{
    _behaviorRegistry->getBehavior(event.id)->onBlockMined(event);
}

void BlockInteractSystem::onBlockNeighborChanged()
{
}

void BlockInteractSystem::onBlockInteracted(const BlockInteractEvent& event)
{
    _behaviorRegistry->getBehavior(event.id)->onBlockInteracted(event);
}

void BlockInteractSystem::onRandomTick()
{
}

void BlockInteractSystem::addDirtyTag(entt::entity chunk, const Vec2i& localPos)
{
    auto& tag = _registry.get_or_emplace<DirtyChunkTag>(chunk);
    tag.addDirtyBlock(localPos);
}

BlockUpdateSystem::BlockUpdateSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : ISystem(registry, dispatcher) {}
BlockUpdateSystem::~BlockUpdateSystem() = default;

BlockPhysicsSystem::BlockPhysicsSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : ISystem(registry, dispatcher), 
    _assetManager(registry.ctx().get<AssetManager>()),
    _blockLayer(registry.ctx().get<BlockLayer>()),
    _physicsLayer(registry.ctx().get<BlockPhysicsLayer>())
{
    _physicsNode = cocos2d::Node::create();
    _physicsNode->setPosition(cocos2d::Vec2(0.5f,0.5f)*BLOCK_SIZE);
    _registry.ctx().get<WorldScene>()->addChild(_physicsNode);
    _body = cocos2d::PhysicsBody::create();
    _body->setDynamic(false);
    _physicsNode->addComponent(_body);
}
BlockPhysicsSystem::~BlockPhysicsSystem() = default;

void BlockPhysicsSystem::update(float delta)
{
    // 1. 确认什么在域内
    // 2. 确认什么要去除
    
    // 3. 更新脏方块

    std::vector<Vec2i> toAdd = getAllAddIn();
    addAll(toAdd);
    std::vector<Vec2i> toRemove = getAllRemoveOut();
    removeAll(toRemove);
    
    updateDirtyBlock();
}

std::vector<Vec2i> BlockPhysicsSystem::getAllAddIn()
{
    auto view = _registry.view<Position, PhysicsTicket>();
    std::vector<Vec2i> toAdd;
    view.each([&](Position& worldPos, PhysicsTicket& ticket)
        {
            // debug
            Vec2i blockUpperLeft = getUpperLeft(worldPos, ticket);
            Vec2i blockLowerRight = getLowerRight(worldPos, ticket);

            for (int y = blockUpperLeft.y; y >= blockLowerRight.y; y--)
            {
                for (int x = blockUpperLeft.x; x <= blockLowerRight.x; x++)
                {
                    Vec2i blockPos = Vec2i(x, y);

                    if (hasCollision(blockPos) && !_physicsLayer.hasPhysicsShapeTag(blockPos))
                    {
                        toAdd.push_back(blockPos);
                    }
                }
            }
        });
    return toAdd;
}

std::vector<Vec2i> BlockPhysicsSystem::getAllRemoveOut()
{
    auto view = _registry.view<Position, PhysicsTicket>();
    std::vector<Vec2i> toRemove;

    for (auto& activePhysicsShape : _physicsLayer.getPhysicsBodyTags())
    {
        bool shouldRemoved = true;
        Vec2i blockPos = activePhysicsShape.first;
        view.each([&](Position& worldPos, PhysicsTicket& ticket)
            {
                Vec2i blockUpperLeft = getUpperLeft(worldPos, ticket);
                Vec2i blockLowerRight = getLowerRight(worldPos, ticket);
                if (isInside(blockPos, blockUpperLeft, blockLowerRight))
                {
                    shouldRemoved = false;
                    return;
                }
            });
        if (shouldRemoved)
        {
            toRemove.push_back(blockPos);
        }
    }
    return toRemove;
}

void BlockPhysicsSystem::addAll(std::vector<Vec2i> allAdded)
{
    for (auto& added : allAdded)
    {
        if (_physicsLayer.hasPhysicsShapeTag(added)) continue;
        _body->addShape(createBoxAtBlockPos(added));
        _physicsLayer.addPhysicsShapeTag(added, Vec2i::vec2ihash(added));
    }
}

void BlockPhysicsSystem::removeAll(std::vector<Vec2i> allRemoved)
{
    for (auto& removed : allRemoved)
    {
        _physicsLayer.removePhysicsShapeTag(removed);
        _body->removeShape(Vec2i::vec2ihash(removed));
    }
}

void BlockPhysicsSystem::updateDirtyBlock()
{
    auto view = _registry.view<DirtyChunkTag>();
    view.each([&](entt::entity chunkID, DirtyChunkTag& tag) 
        {
            const auto chunkPos = BlockLayer::worldPosToChunkPos(_registry.get<Position>(chunkID));
            
            for (auto& dirtyBlock : tag.dirtyBlocks)
            {
                const Vec2i blockPos = chunkPos * CHUNK_SIZE + dirtyBlock.localPos;
                auto shape = createBoxAtBlockPos(blockPos);

                if (_physicsLayer.hasPhysicsShapeTag(blockPos))
                {
                    _body->removeShape(Vec2i::vec2ihash(blockPos));
                    _physicsLayer.removePhysicsShapeTag(blockPos);
                }
                if (shape)
                {
                    _body->addShape(shape);
                    _physicsLayer.addPhysicsShapeTag(blockPos, Vec2i::vec2ihash(blockPos));
                }

                dirtyBlock.collisionDirty = false;
            }
            if (tag.isAllClean())
            {
                _registry.remove<DirtyChunkTag>(chunkID);
            }
        });
}

bool BlockPhysicsSystem::isInside(const Vec2i& blockPos, const Vec2i& blockUpperLeft, const Vec2i& blockLowerRight)
{
    return blockPos.x >= blockUpperLeft.x &&
        blockPos.x <= blockLowerRight.x &&
        blockPos.y <= blockUpperLeft.y &&
        blockPos.y >= blockLowerRight.y;
}

bool BlockPhysicsSystem::hasCollision(const Vec2i& blockPos)
{
    auto blockState = _blockLayer.getBlockAtBlockPos(blockPos);

    bool collision = false;
    if (blockState.id.has_value())
    {
        auto config = _assetManager.getBlockConfig(blockState.id.value());
        collision = tools::get_bool_or(*config, "collision", false);
    }
    return collision;
}

Vec2i BlockPhysicsSystem::getUpperLeft(const Position& worldPos, const PhysicsTicket& ticket)
{
    Vec2i upperleft =
        worldPos.getPostion() + ticket.offset + Vec2i(-ticket.size.x, ticket.size.y) / 2;
    return _blockLayer.worldPosToBlockPos(upperleft);
}

Vec2i BlockPhysicsSystem::getLowerRight(const Position& worldPos, const PhysicsTicket& ticket)
{
    Vec2i lowerright =
        worldPos.getPostion() + ticket.offset + Vec2i(ticket.size.x, -ticket.size.y) / 2;
    return _blockLayer.worldPosToBlockPos(lowerright);
}

cocos2d::PhysicsShapeBox* BlockPhysicsSystem::createBoxAtBlockPos(const Vec2i& blockPos)
{
    if (!hasCollision(blockPos))
    {
        return nullptr;
    }
    auto box = cocos2d::PhysicsShapeBox::create(
        { BLOCK_SIZE, BLOCK_SIZE },
        { 0.1, 1, 1 },
        { blockPos * BLOCK_SIZE });

    CCLOG("create box at %d %d", blockPos.x, blockPos.y);
    box->setTag(Vec2i::vec2ihash(blockPos));
    return box;
}

ChunkRenderCommandSystem::ChunkRenderCommandSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : ISystem(registry, dispatcher) {};
ChunkRenderCommandSystem::~ChunkRenderCommandSystem() = default;

void ChunkRenderCommandSystem::update(float delta) 
{
    updateChunkCommand();
    updateDirtyBlock();
}

BlockCommand* ChunkRenderCommandSystem::generateCommand(const ChunkBlocks& blocks, const Vec2i& localPos)
{
    auto& assetManager = _registry.ctx().get<AssetManager>();
    auto blockID = blocks.getBlockAt(localPos);
    auto config = assetManager.getBlockConfig(blockID);
    std::string texture = tools::get_str_or(*config, "texture", "a_block.bmp");
    // 创建一个方块渲染指令
    return new BlockCommand(
        localPos,
        assetManager.getTexture(texture),
        nullptr
    );
}

void ChunkRenderCommandSystem::updateChunkCommand()
{
    // 获取已经生成但没有渲染指令的区块
    auto view = _registry.view<Position, ChunkBlocks>(entt::exclude<CustomcommandPack>);
    view.each([&](entt::entity entity, Position& pos, ChunkBlocks& blocks)
        {
            auto& commands = _registry.emplace<CustomcommandPack>(entity).commands;

            // 为区块中的每个方块创建渲染指令：
            // command[ x + y * CHUNK_SIZE ]
            for (int y = 0; y < CHUNK_SIZE; y++)
            {
                for (int x = 0; x < CHUNK_SIZE; x++)
                {
                    auto blockCommand = generateCommand(blocks, { x,y });
                    commands.push_back(blockCommand);
                }
            }
        });
}

void ChunkRenderCommandSystem::updateDirtyBlock()
{
    auto view = _registry.view<ChunkBlocks, CustomcommandPack, DirtyChunkTag>();
    view.each([&](entt::entity entity, ChunkBlocks& blocks, CustomcommandPack& pack, DirtyChunkTag tag)
        {
            for (auto& dirtyBlock : tag.dirtyBlocks)
            {
                Vec2i& localPos = dirtyBlock.localPos;
                int cmdPos = localPos.x + localPos.y * CHUNK_SIZE;
                auto blockCommand = generateCommand(blocks, localPos);
                delete pack.commands[cmdPos];
                pack.commands[cmdPos] = blockCommand;
                CCLOG("update a dirty block at %d %d", localPos.x, localPos.y);

                dirtyBlock.renderDirty = false;
            }
            if (tag.isAllClean())
            {
                _registry.remove<DirtyChunkTag>(entity);
            }
        });
}

CommandSystem::CommandSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : ISystem(registry, dispatcher) {}
CommandSystem::~CommandSystem() = default;

void CommandSystem::draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags) {
    auto view = _registry.view<Position, CustomcommandPack>();

    // 将包中的所有渲染命令转发给cocos系统
    view.each([&](Position& pos, CustomcommandPack& pack) 
    {
        for (auto command : pack.commands) 
        {
            // 根据位置进行矩阵变换
            cocos2d::Mat4 trans;
            cocos2d::Mat4::createTranslation({ pos.getPostion().x, pos.getPostion().y,0 }, &trans);
            command->draw(renderer, transform * trans, flags);
        }
    });
}

void CommandSystem::visit(cocos2d::Renderer* renderer, const cocos2d::Mat4& parentTransform, uint32_t parentFlags) 
{
    Node::visit(renderer, parentTransform, parentFlags);
}

DebugSystem::DebugSystem(entt::registry& registry, entt::dispatcher& dispatcher) 
    : ISystem(registry, dispatcher)
{
    auto& world = _registry.ctx().get<WorldScene>();
    auto& assetManager = _registry.ctx().get<AssetManager>();
    auto physicsWorld = world->getPhysicsWorld();
    physicsWorld->setDebugDrawMask(cocos2d::PhysicsWorld::DEBUGDRAW_ALL);
    world->setPhysics3DDebugCamera(cocos2d::Camera::getDefaultCamera());
    _dispatcher.sink<MouseEvent>().connect<&DebugSystem::onMouseEvent>(this);
    
    testEntites.push_back(_registry.create());
    _registry.emplace<Position>(testEntites[0], cocos2d::Vec2::ZERO);
    //_registry.emplace<LoadingTicket>(testEntites[0], testEntites[0], 1, false);

    for (int i = 0; i < 2; i++)
    {
        addAPhysicsSprites();
        physicsSprites[i]->setPosition(400 + i * 200, 100);
        _registry.emplace<LoadingTicket>(physicsEntity[i], physicsEntity[i], 2, false);
    }
}

DebugSystem::~DebugSystem() = default;

void DebugSystem::onMouseEvent(const MouseEvent& event)
{
    auto& blockLayer = _registry.ctx().get<BlockLayer>();
    auto& blockWorld = _registry.ctx().get<BlockWorld>();
    auto blockPos = blockLayer.worldPosToBlockPos(event.worldPos);
    auto blockState = blockLayer.getBlockAtBlockPos(blockPos);

    cocos2d::Color4F color;
    
    if (event.button == cocos2d::EventMouse::MouseButton::BUTTON_RIGHT)
    {
        Vec2i blockPos =
            BlockLayer::worldPosToBlockPos(tools::MouseDebugTool::getWorldPosition());
        blockWorld.TryPlace(blockPos, entt::hashed_string("dirt"), 0, testEntites[0]);
    }
    else if(event.button == cocos2d::EventMouse::MouseButton::BUTTON_LEFT)
    {
        Vec2i blockPos =
            BlockLayer::worldPosToBlockPos(tools::MouseDebugTool::getWorldPosition());
        blockWorld.tryDestroy(blockPos, testEntites[0]);
    }
}

void DebugSystem::update(float delta)
{
    for (int i = 0; i < physicsEntity.size(); i++)
    {
        entt::entity entity = physicsEntity[i];
        auto sprite = physicsSprites[i];

        Position& pos = _registry.get<Position>(entity);
        pos = sprite->getPosition();
    }

    auto camera = cocos2d::Camera::getDefaultCamera();
    if (camera)
    {
        _registry.get<Position>(testEntites[0]) = camera->getPosition();
    }
}

void DebugSystem::addADrawNode()
{
    auto& world = _registry.ctx().get<WorldScene>();
    auto drawNode = cocos2d::DrawNode::create();
    drawNode->setPosition(cocos2d::Vec2::ZERO);
    world->addChild(drawNode);
    drawNodes.push_back(drawNode);
}

void DebugSystem::addAPhysicsSprites()
{
    auto& world = _registry.ctx().get<WorldScene>();
    auto& assetManager = _registry.ctx().get<AssetManager>();
    auto texture = assetManager.getTexture("blocks\\textures\\red_block.png");

    auto sprites = cocos2d::Sprite::createWithTexture(texture);
    sprites->addComponent(cocos2d::PhysicsBody::createBox({ BLOCK_SIZE,BLOCK_SIZE }));
    world->addChild(sprites);

    entt::entity entity = _registry.create();
    _registry.emplace<Position>(entity, cocos2d::Vec2::ZERO);
    _registry.emplace<PhysicsTicket>(entity, cocos2d::Vec2(30, 30), cocos2d::Vec2::ZERO);

    physicsEntity.push_back(entity);
    physicsSprites.push_back(sprites);
}
