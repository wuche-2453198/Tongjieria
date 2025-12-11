#pragma once

/**
 * @file EnttTest.h
 * @brief 验证EnTT库安装和基本功能
 * 
 * 编译此文件即可验证EnTT是否正确安装
 */

#include <entt/entt.hpp>
#include "cocos2d.h"

namespace EnttTest {

// 测试用组件
struct TestPosition {
    float x = 0.0f;
    float y = 0.0f;
    
    TestPosition() = default;
    TestPosition(float _x, float _y) : x(_x), y(_y) {}
};

struct TestVelocity {
    float dx = 0.0f;
    float dy = 0.0f;
    
    TestVelocity() = default;
    TestVelocity(float _dx, float _dy) : dx(_dx), dy(_dy) {}
};

/**
 * @brief 运行EnTT基础测试
 * @return true表示所有测试通过
 */
inline bool runBasicTest() {
    CCLOG("========== EnTT Basic Test ==========");
    
    // 创建registry
    entt::registry registry;
    CCLOG("[OK] Registry created");
    
    // 创建实体
    auto entity1 = registry.create();
    auto entity2 = registry.create();
    auto entity3 = registry.create();
    CCLOG("[OK] Created 3 entities: %u, %u, %u", 
          entt::to_integral(entity1),
          entt::to_integral(entity2),
          entt::to_integral(entity3));
    
    // 添加组件
    registry.emplace<TestPosition>(entity1, 100.0f, 200.0f);
    registry.emplace<TestVelocity>(entity1, 5.0f, 10.0f);
    
    registry.emplace<TestPosition>(entity2, 300.0f, 400.0f);
    // entity2 没有速度组件
    
    registry.emplace<TestPosition>(entity3, 500.0f, 600.0f);
    registry.emplace<TestVelocity>(entity3, -3.0f, -7.0f);
    
    CCLOG("[OK] Components added");
    
    // 测试组件获取
    if (auto* pos = registry.try_get<TestPosition>(entity1)) {
        CCLOG("[OK] Entity1 position: (%.1f, %.1f)", pos->x, pos->y);
    } else {
        CCLOG("[ERROR] Failed to get position for entity1");
        return false;
    }
    
    // 测试view迭代
    int count = 0;
    auto view = registry.view<TestPosition, TestVelocity>();
    view.each([&count](auto entity, TestPosition& pos, TestVelocity& vel) {
        count++;
        pos.x += vel.dx;
        pos.y += vel.dy;
        CCLOG("[OK] Updated entity %u: pos=(%.1f, %.1f) vel=(%.1f, %.1f)",
              entt::to_integral(entity), pos.x, pos.y, vel.dx, vel.dy);
    });
    
    if (count != 2) {
        CCLOG("[ERROR] Expected 2 entities with both components, got %d", count);
        return false;
    }
    CCLOG("[OK] View iteration found %d entities", count);
    
    // 测试组件删除
    registry.remove<TestVelocity>(entity1);
    if (registry.any_of<TestVelocity>(entity1)) {
        CCLOG("[ERROR] Failed to remove velocity component");
        return false;
    }
    CCLOG("[OK] Component removal works");
    
    // 测试实体销毁
    registry.destroy(entity2);
    if (registry.valid(entity2)) {
        CCLOG("[ERROR] Entity2 should be destroyed");
        return false;
    }
    CCLOG("[OK] Entity destruction works");
    
    // 最终验证：确认entity1和entity3仍然有效
    if (!registry.valid(entity1) || !registry.valid(entity3)) {
        CCLOG("[ERROR] Valid entities should still exist");
        return false;
    }
    CCLOG("[OK] Remaining entities are valid");
    
    CCLOG("========== All EnTT Tests Passed! ==========");
    return true;
}

/**
 * @brief 性能测试：创建和迭代大量实体
 */
inline void runPerformanceTest(int entityCount = 10000) {
    CCLOG("========== EnTT Performance Test ==========");
    CCLOG("Creating %d entities...", entityCount);
    
    entt::registry registry;
    
    // 创建实体
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < entityCount; i++) {
        auto entity = registry.create();
        registry.emplace<TestPosition>(entity, 
                                       static_cast<float>(i % 100), 
                                       static_cast<float>(i / 100));
        registry.emplace<TestVelocity>(entity, 1.0f, 1.0f);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    CCLOG("[OK] Created %d entities in %lld μs (%.2f μs per entity)", 
          entityCount, duration.count(), 
          static_cast<double>(duration.count()) / entityCount);
    
    // 迭代测试
    start = std::chrono::high_resolution_clock::now();
    auto view = registry.view<TestPosition, TestVelocity>();
    int updateCount = 0;
    view.each([&](auto entity, TestPosition& pos, TestVelocity& vel) {
        pos.x += vel.dx;
        pos.y += vel.dy;
        updateCount++;
    });
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    CCLOG("[OK] Updated %d entities in %lld μs (%.2f μs per entity)", 
          updateCount, duration.count(),
          static_cast<double>(duration.count()) / updateCount);
    
    CCLOG("========== Performance Test Complete ==========");
}

} // namespace EnttTest
