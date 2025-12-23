#include "RenderSystem.h"
#include "SpriteManager.h"
#include "../AllComponents.h"

namespace ecs {

// ==================== RenderSystem 实现 ====================

void RenderSystem::update(float delta) {
    // 1. 创建新Sprite
    createSprites();
    
    // 2. 同步Transform到Sprite位置
    syncTransformToSprite();
    
    // 3. 同步渲染属性
    syncRenderProperties();
    
    // 4. 清理已销毁的Sprite（由observer处理，这里可选）
    // cleanupDestroyedSprites();
}

void RenderSystem::createSprites() {
    // 查找有RenderComponent和ParentNodeComponent，但没有SpriteStateComponent的实体
    auto view = _registry->view<RenderComponent, ParentNodeComponent>(entt::exclude<SpriteStateComponent>);
    
    view.each([this](auto entity, RenderComponent& render, ParentNodeComponent& parent) {
        if (!parent.parentNode) {
            CCLOG("RenderSystem: Entity %u has no parent node, skipping sprite creation",
                  entt::to_integral(entity));
            return;
        }
        
        // 创建Sprite
        auto* sprite = SpriteManager::getInstance().createSpriteAndAttach(
            render.spriteResourceId,
            parent.parentNode,
            render.zOrder
        );
        
        if (!sprite) {
            CCLOG("RenderSystem: Failed to create sprite for entity %u (resource=%s)",
                  entt::to_integral(entity),
                  render.spriteResourceId.c_str());
            return;
        }
        
        // 添加SpriteStateComponent
        auto& state = _registry->emplace<SpriteStateComponent>(entity);
        state.spriteCreated = true;
        state.spriteHandle = sprite;
        
        parent.attachedToParent = true;
        
        // 获取Transform组件设置初始位置
        auto* transform = _registry->try_get<TransformComponent>(entity);
        if (transform) {
            sprite->setPosition(transform->position);
        }
        
        // 初始化渲染属性
        sprite->setScale(render.scale);
        sprite->setFlippedX(render.flipX);
        sprite->setFlippedY(render.flipY);
        sprite->setVisible(render.visible);
        sprite->setColor(render.color);
        sprite->setOpacity(render.opacity);
        sprite->setRotation(render.rotation);
        
        // 如果有PhysicsBodyComponent，创建并附加物理体
        auto* physicsComp = _registry->try_get<PhysicsBodyComponent>(entity);
        if (physicsComp) {
            cocos2d::PhysicsMaterial material(physicsComp->density, 
                                             physicsComp->restitution, 
                                             physicsComp->friction);
            
            cocos2d::PhysicsBody* body = nullptr;
            if (physicsComp->shape == PhysicsBodyComponent::BodyShape::Circle) {
                body = cocos2d::PhysicsBody::createCircle(physicsComp->radius, material, physicsComp->offset);
            } else {
                body = cocos2d::PhysicsBody::createBox(
                    cocos2d::Size(physicsComp->width, physicsComp->height), 
                    material, 
                    physicsComp->offset);
            }
            
            if (body) {
                body->setDynamic(physicsComp->dynamic);
                body->setMass(physicsComp->density);  // density字段实际存储的是质量
                body->setRotationEnable(physicsComp->rotationEnabled);
                body->setGravityEnable(physicsComp->gravityEnabled);
                body->setVelocityLimit(physicsComp->velocityLimit);
                body->setLinearDamping(physicsComp->linearDamping);
                body->setAngularDamping(physicsComp->angularDamping);
                body->setCategoryBitmask(physicsComp->categoryBitmask);
                body->setContactTestBitmask(physicsComp->contactTestBitmask);
                body->setCollisionBitmask(physicsComp->collisionBitmask);
                body->setGroup(physicsComp->group);
                
                sprite->setPhysicsBody(body);
                
                // 检查是否有初始速度组件，并应用速度
                auto* initialVel = _registry->try_get<InitialVelocityComponent>(entity);
                if (initialVel && !initialVel->applied) {
                    body->setVelocity(initialVel->velocity);
                    initialVel->applied = true;
                    CCLOG("RenderSystem: Applied initial velocity (%.1f, %.1f) to entity %u",
                          initialVel->velocity.x, initialVel->velocity.y, entt::to_integral(entity));
                }
                
                CCLOG("RenderSystem: Created physics body for entity %u",
                      entt::to_integral(entity));
            }
        }
        
        // 注册到NodeEntityMap供碰撞检测使用
        NodeEntityMap::getInstance().registerNode(sprite, entt::to_integral(entity));
        
        CCLOG("RenderSystem: Created sprite for entity %u (resource=%s)",
              entt::to_integral(entity),
              render.spriteResourceId.c_str());
    });
}

void RenderSystem::syncTransformToSprite() {
    auto view = _registry->view<TransformComponent, SpriteStateComponent>();
    
    view.each([this](auto entity, TransformComponent& transform, SpriteStateComponent& state) {
        if (!state.spriteCreated || !state.spriteHandle) return;
        
        auto* sprite = static_cast<cocos2d::Sprite*>(state.spriteHandle);
        
        // 如果有物理体，从物理体读取位置（物理引擎驱动）
        auto* body = sprite->getPhysicsBody();
        if (body) {
            transform.position = sprite->getPosition();
        } else {
            // 否则从Transform同步到Sprite
            sprite->setPosition(transform.position);
        }
    });
}

void RenderSystem::syncRenderProperties() {
    auto view = _registry->view<RenderComponent, SpriteStateComponent>();
    
    view.each([](auto entity, RenderComponent& render, SpriteStateComponent& state) {
        if (!state.spriteCreated || !state.spriteHandle) return;
        
        // 如果禁用同步，跳过（允许Cocos2d Action控制）
        if (!render.enableSync) return;
        
        auto* sprite = static_cast<cocos2d::Sprite*>(state.spriteHandle);
        
        // 同步渲染属性
        sprite->setScale(render.scale);
        sprite->setFlippedX(render.flipX);
        sprite->setFlippedY(render.flipY);
        sprite->setVisible(render.visible);
        sprite->setColor(render.color);
        sprite->setOpacity(render.opacity);
        sprite->setRotation(render.rotation);  // 同步旋转角度
        sprite->setLocalZOrder(render.zOrder);
    });
}

void RenderSystem::cleanupDestroyedSprites() {
    // 这个方法可以作为备用清理机制
    // 主要清理由SpriteDestructionObserver处理
}

// ==================== AnimationSystem 实现 ====================

void AnimationSystem::update(float delta) {
    // 1. 更新状态驱动的动画切换
    updateStateDrivenAnimation();
    
    // 2. 更新帧动画播放
    updateFrameAnimation(delta);
}

void AnimationSystem::updateStateDrivenAnimation() {
    // 处理同时拥有AnimationComponent和AnimationStateComponent的实体
    auto view = _registry->view<AnimationComponent, AnimationStateComponent>();
    
    view.each([this](auto entity, AnimationComponent& anim, AnimationStateComponent& animState) {
        if (!animState.enableStateDriven) {
            return; // 未启用状态驱动
        }
        
        // 检查状态是否发生变化
        if (animState.hasStateChanged()) {
            const AnimationStateData* stateData = animState.getCurrentAnimationData();
            if (stateData) {
                // 应用新的动画状态
                applyAnimationStateData(anim, *stateData);
                animState.currentPriority = stateData->priority;
                
                CCLOG("AnimationSystem: Switched animation state to '%s' for entity %u",
                      animState.currentState.c_str(), entt::to_integral(entity));
            }
            
            // 更新previousState以避免重复切换
            animState.previousState = animState.currentState;
        }
    });
}

void AnimationSystem::applyAnimationStateData(AnimationComponent& anim, const AnimationStateData& stateData) {
    // 更新动画组件的所有相关属性
    anim.animationSetId = stateData.animationSetId;
    anim.frameSequence = stateData.frameSequence;
    anim.frameTime = stateData.frameTime;
    anim.loop = stateData.loop;
    
    // 重置动画播放状态
    anim.reset();
    anim.isPlaying = true;
}

void AnimationSystem::updateFrameAnimation(float delta) {
    auto view = _registry->view<AnimationComponent, SpriteStateComponent>();
    
    view.each([delta](auto entity, AnimationComponent& anim, SpriteStateComponent& state) {
        if (!anim.isPlaying || anim.frameSequence.empty() || !state.spriteHandle) {
            return;
        }
        
        auto* sprite = static_cast<cocos2d::Sprite*>(state.spriteHandle);
        
        // 更新计时器
        anim.frameTimer += delta;
        
        if (anim.frameTimer >= anim.frameTime) {
            anim.frameTimer -= anim.frameTime;
            
            // 切换到下一帧
            anim.currentFrameIndex++;
            
            // 检查是否到达末尾
            if (anim.currentFrameIndex >= (int)anim.frameSequence.size()) {
                if (anim.loop) {
                    anim.currentFrameIndex = 0;
                } else {
                    anim.currentFrameIndex = (int)anim.frameSequence.size() - 1;
                    anim.isPlaying = false;
                    return;
                }
            }
            
            // 获取动画帧
            const auto& frames = SpriteManager::getInstance().getAnimationFrames(anim.animationSetId);
            if (!frames.empty()) {
                int frameNumber = anim.frameSequence[anim.currentFrameIndex];
                int frameIdx = frameNumber - 1; // 1-indexed转0-indexed
                
                if (frameIdx >= 0 && frameIdx < (int)frames.size()) {
                    sprite->setSpriteFrame(frames.at(frameIdx));
                }
            }
        }
    });
}

// ==================== SpriteDestructionObserver 实现 ====================

void SpriteDestructionObserver::registerToRegistry(entt::registry& registry) {
    // 注册组件销毁回调
    registry.on_destroy<SpriteStateComponent>().connect<&SpriteDestructionObserver::onSpriteStateDestroy>();
    
    CCLOG("SpriteDestructionObserver: Registered to registry");
}

void SpriteDestructionObserver::onSpriteStateDestroy(entt::registry& registry, entt::entity entity) {
    // 获取SpriteStateComponent
    auto* state = registry.try_get<SpriteStateComponent>(entity);
    if (state && state->spriteHandle) {
        auto* sprite = static_cast<cocos2d::Sprite*>(state->spriteHandle);
        SpriteManager::getInstance().releaseSprite(sprite);
        
        CCLOG("SpriteDestructionObserver: Released sprite for entity %u",
              entt::to_integral(entity));
    }
}

} // namespace ecs
