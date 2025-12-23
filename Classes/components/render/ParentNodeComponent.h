#ifndef __ECS_COMPONENT_PARENTNODECOMPONENT_H__
#define __ECS_COMPONENT_PARENTNODECOMPONENT_H__

#include "cocos2d.h"

namespace ecs {

/**
 * @brief 父节点引用组件 - 记录应挂载到哪个节点（新解耦架构）
 * 
 * 用途：
 * - 告诉RenderSystem应该将Sprite挂载到哪个Cocos2d节点
 * - 通常在创建实体时由Scene设置
 */
struct ParentNodeComponent {
    cocos2d::Node* parentNode = nullptr; // 父节点指针（由Scene管理）
    bool attachedToParent = false;       // 是否已挂载
    
    ParentNodeComponent() = default;
    ParentNodeComponent(cocos2d::Node* parent) : parentNode(parent) {}
};

} // namespace ecs

#endif // __ECS_COMPONENT_PARENTNODECOMPONENT_H__
