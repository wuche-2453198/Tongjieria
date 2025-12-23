#ifndef __ECS_COMPONENT_SPRITERESOURCEDESCRIPTOR_H__
#define __ECS_COMPONENT_SPRITERESOURCEDESCRIPTOR_H__

#include "cocos2d.h"
#include <string>
#include <vector>

namespace ecs {

/**
 * @brief 精灵资源描述符 - 用于加载和缓存精灵（新解耦架构）
 * 
 * 用于向SpriteManager注册精灵资源
 * 不是实体组件，而是配置数据
 */
struct SpriteResourceDescriptor {
    std::string resourceId;            // 资源唯一ID
    std::string spritePath;            // 精灵文件路径
    std::vector<std::string> framePaths; // 动画帧路径（多帧动画）
    cocos2d::Vec2 anchorPoint = cocos2d::Vec2(0.5f, 0.5f);
    
    SpriteResourceDescriptor() = default;
    SpriteResourceDescriptor(const std::string& id, const std::string& path)
        : resourceId(id), spritePath(path) {}
};

} // namespace ecs

#endif // __ECS_COMPONENT_SPRITERESOURCEDESCRIPTOR_H__
