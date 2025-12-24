#ifndef __ENTT_TEST_H__
#define __ENTT_TEST_H__

#include "entt/entt.hpp"
#include "cocos2d.h"
#include <chrono>
#include <string>

/**
 * @brief EnTT 集成测试类
 * 用于验证 EnTT 库是否正确配置和工作
 */
class EnttTest {
public:
    // 简单的测试组件
    struct Position {
        float x, y;
        Position(float x = 0, float y = 0) : x(x), y(y) {}
    };

    struct Velocity {
        float dx, dy;
        Velocity(float dx = 0, float dy = 0) : dx(dx), dy(dy) {}
    };

    struct Name {
        std::string value;
        Name(const std::string& n = "") : value(n) {}
    };

    /**
     * @brief 运行基础测试
     * @return 测试是否通过
     */
    static bool runBasicTest() {
        CCLOG("=== EnTT Basic Test Start ===");

        try {
            // 创建 registry
            entt::registry registry;

            // 测试1: 创建实体
            auto entity1 = registry.create();
            auto entity2 = registry.create();
            auto entity3 = registry.create();
            CCLOG("✓ Test 1 Passed: Created 3 entities");

            // 测试2: 添加组件
            registry.emplace<Position>(entity1, 100.0f, 200.0f);
            registry.emplace<Velocity>(entity1, 5.0f, -3.0f);
            registry.emplace<Name>(entity1, "Player");

            registry.emplace<Position>(entity2, 300.0f, 400.0f);
            registry.emplace<Name>(entity2, "Enemy");

            registry.emplace<Position>(entity3, 500.0f, 600.0f);
            registry.emplace<Velocity>(entity3, -2.0f, 4.0f);

            CCLOG("✓ Test 2 Passed: Added components to entities");

            // 测试3: 查询组件
            auto* pos = registry.try_get<Position>(entity1);
            if (pos && pos->x == 100.0f && pos->y == 200.0f) {
                CCLOG("✓ Test 3 Passed: Component query works (x=%.1f, y=%.1f)", pos->x, pos->y);
            } else {
                CCLOG("✗ Test 3 Failed: Component query failed");
                return false;
            }

            // 测试4: View 查询 (单组件)
            int posCount = 0;
            auto posView = registry.view<Position>();
            for (auto entity : posView) {
                posCount++;
            }
            if (posCount == 3) {
                CCLOG("✓ Test 4 Passed: View query found %d entities with Position", posCount);
            } else {
                CCLOG("✗ Test 4 Failed: Expected 3, got %d", posCount);
                return false;
            }

            // 测试5: View 查询 (多组件)
            int bothCount = 0;
            auto multiView = registry.view<Position, Velocity>();
            for (auto entity : multiView) {
                bothCount++;
                auto [pos, vel] = multiView.get<Position, Velocity>(entity);
                CCLOG("  - Entity with Position(%.1f, %.1f) and Velocity(%.1f, %.1f)",
                      pos.x, pos.y, vel.dx, vel.dy);
            }
            if (bothCount == 2) {
                CCLOG("✓ Test 5 Passed: Multi-component view found %d entities", bothCount);
            } else {
                CCLOG("✗ Test 5 Failed: Expected 2, got %d", bothCount);
                return false;
            }

            // 测试6: 使用 each 遍历
            CCLOG("Test 6: Using each() to iterate:");
            registry.view<Position, Name>().each([](auto entity, Position& pos, Name& name) {
                CCLOG("  - %s at position (%.1f, %.1f)", name.value.c_str(), pos.x, pos.y);
            });
            CCLOG("✓ Test 6 Passed: each() iteration works");

            // 测试7: 移除组件
            registry.remove<Velocity>(entity1);
            if (!registry.all_of<Velocity>(entity1)) {
                CCLOG("✓ Test 7 Passed: Component removal works");
            } else {
                CCLOG("✗ Test 7 Failed: Component removal failed");
                return false;
            }

            // 测试8: 销毁实体
            registry.destroy(entity2);
            if (!registry.valid(entity2)) {
                CCLOG("✓ Test 8 Passed: Entity destruction works");
            } else {
                CCLOG("✗ Test 8 Failed: Entity destruction failed");
                return false;
            }

            // 测试9: 统计信息
            // EnTT 的 registry 通过 storage<entt::entity>() 访问实体存储
            size_t aliveCount = registry.storage<entt::entity>().size();
            CCLOG("✓ Test 9 Passed: %zu entities alive", aliveCount);

            CCLOG("=== EnTT Basic Test Complete: ALL TESTS PASSED ===");
            return true;

        } catch (const std::exception& e) {
            CCLOG("✗ EnTT Test Failed with exception: %s", e.what());
            return false;
        }
    }

    /**
     * @brief 运行性能测试
     */
    static void runPerformanceTest() {
        CCLOG("=== EnTT Performance Test Start ===");

        entt::registry registry;

        // 创建10000个实体
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < 10000; i++) {
            auto entity = registry.create();
            registry.emplace<Position>(entity, (float)i, (float)i);
            if (i % 2 == 0) {
                registry.emplace<Velocity>(entity, 1.0f, 1.0f);
            }
            if (i % 3 == 0) {
                registry.emplace<Name>(entity, "Entity_" + std::to_string(i));
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        CCLOG("Created 10000 entities in %lld ms", duration);

        // 查询性能测试
        start = std::chrono::high_resolution_clock::now();
        int count = 0;
        auto view = registry.view<Position, Velocity>();
        for (auto entity : view) {
            auto [pos, vel] = view.get<Position, Velocity>(entity);
            pos.x += vel.dx;
            pos.y += vel.dy;
            count++;
        }
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        CCLOG("Updated %d entities in %lld microseconds", count, duration);

        CCLOG("=== EnTT Performance Test Complete ===");
    }
};

#endif // __ENTT_TEST_H__
