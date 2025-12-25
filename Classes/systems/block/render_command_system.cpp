#include "components/block/chunk_render.h"
#include "components/block/block_component.h"
#include "render_command_system.h"

#include "debug_system.h"

CommandSystem::CommandSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : ISystem(registry, dispatcher) {}
CommandSystem::~CommandSystem() = default;

void CommandSystem::draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags) {
    auto view = _registry.view<Position, CustomcommandPack>();

    // 将包中的所有渲染命令转发给cocos系统
    view.each([&](const Position& pos, CustomcommandPack& pack)
        {
            for (auto command : pack.commands)
            {
                if (!command->isActive()) continue;
                if (command->isUseTransform())
                {
                    // 根据位置进行矩阵变换
                    cocos2d::Mat4 trans;
                    cocos2d::Mat4::createTranslation({ pos.getPostion().x, pos.getPostion().y, 0 }, &trans);
                    
                    command->draw(renderer, transform * trans, flags);
                }
                else
                {
                    command->draw(renderer, transform, flags);
                }
            }
        });
}

void CommandSystem::visit(cocos2d::Renderer* renderer, const cocos2d::Mat4& parentTransform, uint32_t parentFlags)
{
    Node::visit(renderer, parentTransform, parentFlags);
}