#include "entt/entt.hpp"
#include "components/block/block_event.h"
#include "block_system_manager.h"
#include "cocos2d.h"
#pragma once

class MouseEvent;
class DebugSystem : public ISystem
{
public:
    DebugSystem(entt::registry& registry, entt::dispatcher& dispatcher);
    ~DebugSystem();

    void onMouseEvent(const MouseEvent& event);
    void update(float delta);

    void addADrawNode();
    void addAPhysicsSprites();
    
    static inline std::vector<entt::entity> testEntites;
    static inline std::vector<cocos2d::DrawNode*> drawNodes;
    static inline std::vector<cocos2d::Label*> labels;
    static inline std::vector<entt::entity> physicsEntity;
    static inline std::vector<cocos2d::Sprite*> physicsSprites;
};