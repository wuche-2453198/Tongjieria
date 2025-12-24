#include "block_system_manager.h"
#include "cocos2d.h"

class BlockCommand;
class BlockState;

/**
* @brief 将所有自定义渲染指令提交给Renderer。
*
* 任何有**位置**和**渲染指令**的实体都会触发渲染指令的提交。
* 该系统本质是桥接器，将实体自定义的渲染指令包发送到cocos中。不参与直接的渲染工作。
*
* @see CustomCommandPack
*/
class CommandSystem : public ISystem, public cocos2d::Node
{
public:
    CommandSystem(entt::registry& registry, entt::dispatcher& dispatcher);
    ~CommandSystem();
    void draw(cocos2d::Renderer* renderer,
        const cocos2d::Mat4& transform, uint32_t flags);

    void visit(cocos2d::Renderer* renderer,
        const cocos2d::Mat4& parentTransform, uint32_t parentFlags);
private:
};