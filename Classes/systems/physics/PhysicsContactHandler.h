#ifndef __ECS_PHYSICS_CONTACT_HANDLER_H__
#define __ECS_PHYSICS_CONTACT_HANDLER_H__

#include "cocos2d.h"
#include "components/AllComponents.h"
#include "components/render/SpriteComponent.h"
#include <entt/entt.hpp>
#include <cstdint>

namespace ecs {

/**
 * @brief 物理碰撞处理器 - 提供通用的碰撞处理逻辑
 * 
 * 功能：
 * - 地面检测（基于接触法线）
 * - 墙壁碰撞预处理（防止滑墙）
 * - 动态/静态物体判断
 */
class PhysicsContactHandler {
public:
    // 法线阈值常量
    static constexpr float GROUND_NORMAL_THRESHOLD = -0.3f;  // 地面法线Y阈值
    static constexpr float GROUND_NORMAL_X_MAX = 0.6f;
    static constexpr float WALL_NORMAL_X_THRESHOLD = 0.7f;   // 墙壁法线X阈值
    static constexpr float WALL_NORMAL_Y_THRESHOLD = 0.3f;   // 墙壁法线Y阈值

    /**
     * @brief 碰撞信息结构
     */
    struct ContactInfo {
        cocos2d::PhysicsBody* dynamicBody = nullptr;
        cocos2d::PhysicsBody* staticBody = nullptr;
        cocos2d::Node* dynamicNode = nullptr;
        cocos2d::Node* staticNode = nullptr;
        cocos2d::Vec2 normal;           // 指向动态物体的法线
        std::uintptr_t contactKey = 0;
        bool isValid = false;           // 是否有效（存在动态-静态对）
        bool isGroundContact = false;   // 是否为地面接触
        bool isWallContact = false;     // 是否为墙壁接触
    };

    static std::uintptr_t makeContactKey(cocos2d::PhysicsContact& contact) {
        auto* shapeA = contact.getShapeA();
        auto* shapeB = contact.getShapeB();
        std::uintptr_t a = reinterpret_cast<std::uintptr_t>(shapeA);
        std::uintptr_t b = reinterpret_cast<std::uintptr_t>(shapeB);
        if (a > b) {
            std::uintptr_t tmp = a;
            a = b;
            b = tmp;
        }
        return a ^ (b << 1);
    }

    /**
     * @brief 解析碰撞信息
     * @param contact 物理碰撞对象
     * @return 碰撞信息结构
     */
    static ContactInfo parseContact(cocos2d::PhysicsContact& contact) {
        ContactInfo info;
        
        auto bodyA = contact.getShapeA()->getBody();
        auto bodyB = contact.getShapeB()->getBody();
        
        bool dynamicIsA = false;
        if (bodyA->isDynamic() && !bodyB->isDynamic()) {
            info.dynamicBody = bodyA;
            info.staticBody = bodyB;
            dynamicIsA = true;
        } else if (bodyB->isDynamic() && !bodyA->isDynamic()) {
            info.dynamicBody = bodyB;
            info.staticBody = bodyA;
            dynamicIsA = false;
        } else {
            info.isValid = false;
            return info;
        }
        
        info.dynamicNode = info.dynamicBody->getNode();
        info.staticNode = info.staticBody->getNode();

        info.contactKey = makeContactKey(contact);
        
        if (!info.dynamicNode) {
            info.isValid = false;
            return info;
        }
        
        // 获取并调整法线方向（指向动态物体）
        info.normal = contact.getContactData()->normal;
        if (!dynamicIsA) info.normal = -info.normal;
        
        // 判断接触类型
        info.isGroundContact = (info.normal.y < GROUND_NORMAL_THRESHOLD) &&
                               (std::abs(info.normal.x) < GROUND_NORMAL_X_MAX);
        info.isWallContact = (std::abs(info.normal.x) > WALL_NORMAL_X_THRESHOLD && 
                              std::abs(info.normal.y) < WALL_NORMAL_Y_THRESHOLD);
        info.isValid = true;
        
        return info;
    }

    /**
     * @brief 处理地面接触开始 - 更新GroundDetectorComponent
     * @param registry EnTT注册表
     * @param info 碰撞信息
     */
    static void handleGroundContactBegin(entt::registry& registry, const ContactInfo& info) {
        if (!info.isValid || !info.isGroundContact) return;
        
        EntityId entityId = NodeEntityMap::getInstance().findEntity(info.dynamicNode);
        if (entityId == INVALID_ENTITY) return;
        
        auto entity = static_cast<entt::entity>(entityId);
        if (!registry.valid(entity)) return;
        
        auto* ground = registry.try_get<GroundDetectorComponent>(entity);
        if (ground) {
            if (ground->groundContactKeys.insert(info.contactKey).second) {
                ground->isOnGround = true;
                ground->groundContactCount++;
            }
        }
    }

    /**
     * @brief 处理地面接触分离 - 更新GroundDetectorComponent
     * @param registry EnTT注册表
     * @param info 碰撞信息
     */
    static void handleGroundContactSeparate(entt::registry& registry, const ContactInfo& info) {
        if (!info.isValid) return;
        
        EntityId entityId = NodeEntityMap::getInstance().findEntity(info.dynamicNode);
        if (entityId == INVALID_ENTITY) return;
        
        auto entity = static_cast<entt::entity>(entityId);
        if (!registry.valid(entity)) return;
        
        auto* ground = registry.try_get<GroundDetectorComponent>(entity);
        if (ground) {
            auto it = ground->groundContactKeys.find(info.contactKey);
            if (it != ground->groundContactKeys.end()) {
                ground->groundContactKeys.erase(it);
                ground->groundContactCount--;
                if (ground->groundContactCount <= 0) {
                    ground->isOnGround = false;
                    ground->groundContactCount = 0;
                    ground->groundContactKeys.clear();
                }
            }
        }
    }

    /**
     * @brief 处理墙壁碰撞预求解 - 防止滑墙
     * @param info 碰撞信息
     * @param solve 预求解对象
     */
    static void handleWallPreSolve(const ContactInfo& info, cocos2d::PhysicsContactPreSolve& solve) {
        if (!info.isValid || !info.isWallContact) return;
        
        // 设置弹性和摩擦为0，防止滑上去
        solve.setRestitution(0.0f);
        solve.setFriction(0.0f);
    }

    /**
     * @brief 创建标准的物理碰撞监听器
     * @param registry EnTT注册表引用
     * @param customBeginHandler 自定义的碰撞开始处理（可选，返回true继续处理）
     * @param customSeparateHandler 自定义的碰撞分离处理（可选）
     * @return 配置好的碰撞监听器
     */
    static cocos2d::EventListenerPhysicsContact* createContactListener(
        entt::registry& registry,
        std::function<bool(cocos2d::PhysicsContact&, const ContactInfo&)> customBeginHandler = nullptr,
        std::function<void(cocos2d::PhysicsContact&, const ContactInfo&)> customSeparateHandler = nullptr
    ) {
        auto listener = cocos2d::EventListenerPhysicsContact::create();
        
        // 碰撞开始
        listener->onContactBegin = [&registry, customBeginHandler](cocos2d::PhysicsContact& contact) {
            auto info = parseContact(contact);
            
            // 先调用自定义处理器
            if (customBeginHandler) {
                if (!customBeginHandler(contact, info)) {
                    return true;  // 自定义处理器已处理，跳过默认处理
                }
            }
            {
                auto* bodyA = contact.getShapeA() ? contact.getShapeA()->getBody() : nullptr;
                auto* bodyB = contact.getShapeB() ? contact.getShapeB()->getBody() : nullptr;
                cocos2d::Node* nodeA = bodyA ? bodyA->getNode() : nullptr;
                cocos2d::Node* nodeB = bodyB ? bodyB->getNode() : nullptr;

                EntityId idA = nodeA ? NodeEntityMap::getInstance().findEntity(nodeA) : INVALID_ENTITY;
                EntityId idB = nodeB ? NodeEntityMap::getInstance().findEntity(nodeB) : INVALID_ENTITY;

                if (idA != INVALID_ENTITY && idB != INVALID_ENTITY) {
                    auto entA = static_cast<entt::entity>(idA);
                    auto entB = static_cast<entt::entity>(idB);
                    if (registry.valid(entA) && registry.valid(entB)) {
                        auto* combatA = registry.try_get<CombatComponent>(entA);
                        auto* playerB = registry.try_get<PlayerTag>(entB);
                        auto* healthB = registry.try_get<HealthComponent>(entB);
                        if (combatA && playerB && healthB) {
                            if (combatA->attackTimer <= 0.0f && healthB->invincibleTimer <= 0.0f) {
                                healthB->takeDamage(combatA->attackDamage);
                                healthB->invincibleTimer = healthB->invincibleTime;
                                combatA->attackTimer = combatA->attackCooldown;
                            }
                        }
                        auto* combatB = registry.try_get<CombatComponent>(entB);
                        auto* playerA = registry.try_get<PlayerTag>(entA);
                        auto* healthA = registry.try_get<HealthComponent>(entA);
                        if (combatB && playerA && healthA) {
                            if (combatB->attackTimer <= 0.0f && healthA->invincibleTimer <= 0.0f) {
                                healthA->takeDamage(combatB->attackDamage);
                                healthA->invincibleTimer = healthA->invincibleTime;
                                combatB->attackTimer = combatB->attackCooldown;
                            }
                        }
                    }
                }
            }
            
            // 默认地面检测处理
            handleGroundContactBegin(registry, info);
            return true;
        };
        
        // 碰撞分离
        listener->onContactSeparate = [&registry, customSeparateHandler](cocos2d::PhysicsContact& contact) {
            auto info = parseContact(contact);
            
            // 先调用自定义处理器
            if (customSeparateHandler) {
                customSeparateHandler(contact, info);
            }
            
            // 默认地面检测处理
            handleGroundContactSeparate(registry, info);
        };
        
        // 预求解 - 防止滑墙
        listener->onContactPreSolve = [](cocos2d::PhysicsContact& contact, 
                                         cocos2d::PhysicsContactPreSolve& solve) {
            auto info = parseContact(contact);
            handleWallPreSolve(info, solve);
            return true;
        };
        
        return listener;
    }
};

} // namespace ecs

#endif // __ECS_PHYSICS_CONTACT_HANDLER_H__
