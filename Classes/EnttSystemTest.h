#pragma once

#include <entt/entt.hpp>
#include "ecs/SystemsEntt.h"
#include "ecs/Components.h"
#include "cocos2d.h"

/**
 * @file EnttSystemTest.h
 * @brief EnTT System迁移测试
 * 
 * 对比测试：验证EnTT版本的System与原版功能一致
 */

namespace EnttSystemTest {

/**
 * @brief 测试HealthSystem迁移
 */
inline bool testHealthSystem() {
    CCLOG("========== Testing HealthSystem Migration ==========");
    
    // 创建Registry
    entt::registry registry;
    
    // 创建测试实体
    auto entity1 = registry.create();
    auto entity2 = registry.create();
    auto entity3 = registry.create();
    
    // 添加HealthComponent
    registry.emplace<ecs::HealthComponent>(entity1, 100.0f);
    registry.emplace<ecs::HealthComponent>(entity2, 50.0f);
    registry.emplace<ecs::HealthComponent>(entity3, 75.0f);
    
    // 设置无敌时间
    auto& health1 = registry.get<ecs::HealthComponent>(entity1);
    health1.invincibleTimer = 2.0f;
    
    auto& health2 = registry.get<ecs::HealthComponent>(entity2);
    health2.invincibleTimer = 1.5f;
    
    auto& health3 = registry.get<ecs::HealthComponent>(entity3);
    health3.invincibleTimer = 0.0f;
    
    CCLOG("[Setup] Created 3 entities with health:");
    CCLOG("  Entity1: HP=%.1f, Invincible=%.1f", health1.currentHealth, health1.invincibleTimer);
    CCLOG("  Entity2: HP=%.1f, Invincible=%.1f", health2.currentHealth, health2.invincibleTimer);
    CCLOG("  Entity3: HP=%.1f, Invincible=%.1f", health3.currentHealth, health3.invincibleTimer);
    
    // 创建HealthSystem
    ecs::HealthSystemEntt healthSystem;
    healthSystem.setRegistry(&registry);
    
    // 模拟第一帧更新（delta = 1.0秒）
    CCLOG("[Frame 1] Updating with delta=1.0s...");
    healthSystem.update(1.0f);
    
    // 验证结果
    auto& health1_after1 = registry.get<ecs::HealthComponent>(entity1);
    auto& health2_after1 = registry.get<ecs::HealthComponent>(entity2);
    auto& health3_after1 = registry.get<ecs::HealthComponent>(entity3);
    
    CCLOG("[After Frame 1]:");
    CCLOG("  Entity1: Invincible=%.1f (expected 1.0)", health1_after1.invincibleTimer);
    CCLOG("  Entity2: Invincible=%.1f (expected 0.5)", health2_after1.invincibleTimer);
    CCLOG("  Entity3: Invincible=%.1f (expected 0.0)", health3_after1.invincibleTimer);
    
    if (std::abs(health1_after1.invincibleTimer - 1.0f) > 0.01f ||
        std::abs(health2_after1.invincibleTimer - 0.5f) > 0.01f ||
        std::abs(health3_after1.invincibleTimer - 0.0f) > 0.01f) {
        CCLOG("[ERROR] Invincible timer calculation incorrect!");
        return false;
    }
    
    // 模拟第二帧更新（delta = 0.6秒）
    CCLOG("[Frame 2] Updating with delta=0.6s...");
    healthSystem.update(0.6f);
    
    auto& health1_after2 = registry.get<ecs::HealthComponent>(entity1);
    auto& health2_after2 = registry.get<ecs::HealthComponent>(entity2);
    
    CCLOG("[After Frame 2]:");
    CCLOG("  Entity1: Invincible=%.1f (expected 0.4)", health1_after2.invincibleTimer);
    CCLOG("  Entity2: Invincible=%.1f (expected 0.0)", health2_after2.invincibleTimer);
    
    if (std::abs(health1_after2.invincibleTimer - 0.4f) > 0.01f ||
        health2_after2.invincibleTimer < 0.0f) {
        CCLOG("[ERROR] Second frame calculation incorrect!");
        return false;
    }
    
    CCLOG("[OK] HealthSystem migration test PASSED!");
    CCLOG("========== Test Complete ==========");
    return true;
}

/**
 * @brief 测试CombatSystem迁移
 */
inline bool testCombatSystem() {
    CCLOG("========== Testing CombatSystem Migration ==========");
    
    entt::registry registry;
    
    // 创建测试实体
    auto warrior = registry.create();
    auto archer = registry.create();
    
    // 添加CombatComponent
    registry.emplace<ecs::CombatComponent>(warrior, 25.0f, 50.0f, 1.0f);
    registry.emplace<ecs::CombatComponent>(archer, 15.0f, 30.0f, 0.8f);
    
    // 设置攻击冷却
    auto& warCombat = registry.get<ecs::CombatComponent>(warrior);
    warCombat.attackTimer = 1.0f;
    
    auto& archCombat = registry.get<ecs::CombatComponent>(archer);
    archCombat.attackTimer = 0.5f;
    
    CCLOG("[Setup] Warrior attackTimer=%.1f, Archer attackTimer=%.1f", 
          warCombat.attackTimer, archCombat.attackTimer);
    
    // 创建CombatSystem
    ecs::CombatSystemEntt combatSystem;
    combatSystem.setRegistry(&registry);
    
    // 更新
    combatSystem.update(0.3f);
    
    auto& warCombat_after = registry.get<ecs::CombatComponent>(warrior);
    auto& archCombat_after = registry.get<ecs::CombatComponent>(archer);
    
    CCLOG("[After Update] Warrior attackTimer=%.1f (expected 0.7), Archer attackTimer=%.1f (expected 0.2)",
          warCombat_after.attackTimer, archCombat_after.attackTimer);
    
    if (std::abs(warCombat_after.attackTimer - 0.7f) > 0.01f ||
        std::abs(archCombat_after.attackTimer - 0.2f) > 0.01f) {
        CCLOG("[ERROR] Combat timer calculation incorrect!");
        return false;
    }
    
    CCLOG("[OK] CombatSystem migration test PASSED!");
    CCLOG("========== Test Complete ==========");
    return true;
}

/**
 * @brief 测试LifetimeSystem迁移
 */
inline bool testLifetimeSystem() {
    CCLOG("========== Testing LifetimeSystem Migration ==========");
    
    entt::registry registry;
    
    // 创建带生命周期的实体
    auto shortLived = registry.create();
    auto longLived = registry.create();
    auto immortal = registry.create();
    
    registry.emplace<ecs::LifetimeComponent>(shortLived, 0.5f);
    registry.emplace<ecs::LifetimeComponent>(longLived, 2.0f);
    // immortal没有LifetimeComponent
    
    CCLOG("[Setup] Created 3 entities (2 with lifetime, 1 immortal)");
    
    // 创建LifetimeSystem
    ecs::LifetimeSystemEntt lifetimeSystem;
    lifetimeSystem.setRegistry(&registry);
    
    // 第一次更新（0.3秒）
    lifetimeSystem.update(0.3f);
    
    bool shortAlive = registry.valid(shortLived);
    bool longAlive = registry.valid(longLived);
    bool immortalAlive = registry.valid(immortal);
    
    CCLOG("[After 0.3s] Short=%d (expected 1), Long=%d (expected 1), Immortal=%d (expected 1)",
          shortAlive, longAlive, immortalAlive);
    
    if (!shortAlive || !longAlive || !immortalAlive) {
        CCLOG("[ERROR] Entities destroyed too early!");
        return false;
    }
    
    // 第二次更新（0.3秒，总共0.6秒）
    lifetimeSystem.update(0.3f);
    
    shortAlive = registry.valid(shortLived);
    longAlive = registry.valid(longLived);
    
    CCLOG("[After 0.6s] Short=%d (expected 0), Long=%d (expected 1)",
          shortAlive, longAlive);
    
    if (shortAlive || !longAlive) {
        CCLOG("[ERROR] Short-lived entity should be destroyed!");
        return false;
    }
    
    CCLOG("[OK] LifetimeSystem migration test PASSED!");
    CCLOG("========== Test Complete ==========");
    return true;
}

/**
 * @brief 测试SystemManager
 */
inline bool testSystemManager() {
    CCLOG("========== Testing SystemManager ==========");
    
    entt::registry registry;
    ecs::SystemManagerEntt manager;
    manager.setRegistry(&registry);
    
    // 添加多个System
    manager.addSystem<ecs::HealthSystemEntt>();
    manager.addSystem<ecs::CombatSystemEntt>();
    manager.addSystem<ecs::LifetimeSystemEntt>();
    
    CCLOG("[Setup] Added 3 systems to manager");
    
    if (manager.getSystemCount() != 3) {
        CCLOG("[ERROR] System count incorrect! Expected 3, got %zu", manager.getSystemCount());
        return false;
    }
    
    // 创建测试实体
    auto entity = registry.create();
    registry.emplace<ecs::HealthComponent>(entity, 100.0f);
    registry.emplace<ecs::CombatComponent>(entity, 25.0f, 50.0f, 1.0f);
    
    auto& health = registry.get<ecs::HealthComponent>(entity);
    auto& combat = registry.get<ecs::CombatComponent>(entity);
    health.invincibleTimer = 1.0f;
    combat.attackTimer = 0.5f;
    
    // 统一更新所有System
    manager.update(0.2f);
    
    if (std::abs(health.invincibleTimer - 0.8f) > 0.01f ||
        std::abs(combat.attackTimer - 0.3f) > 0.01f) {
        CCLOG("[ERROR] Manager update failed!");
        return false;
    }
    
    CCLOG("[OK] SystemManager test PASSED!");
    CCLOG("========== Test Complete ==========");
    return true;
}

/**
 * @brief 运行所有测试
 */
inline bool runAllTests() {
    CCLOG("========================================");
    CCLOG("  EnTT System Migration Test Suite");
    CCLOG("========================================");
    
    bool allPassed = true;
    
    allPassed &= testHealthSystem();
    CCLOG("");
    
    allPassed &= testCombatSystem();
    CCLOG("");
    
    allPassed &= testLifetimeSystem();
    CCLOG("");
    
    allPassed &= testSystemManager();
    CCLOG("");
    
    CCLOG("========================================");
    if (allPassed) {
        CCLOG("✓ ALL TESTS PASSED!");
    } else {
        CCLOG("✗ SOME TESTS FAILED!");
    }
    CCLOG("========================================");
    
    return allPassed;
}

} // namespace EnttSystemTest
