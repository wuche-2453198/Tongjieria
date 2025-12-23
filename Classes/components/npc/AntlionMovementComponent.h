#ifndef __ECS_COMPONENT_ANTLIONMOVEMENTCOMPONENT_H__
#define __ECS_COMPONENT_ANTLIONMOVEMENTCOMPONENT_H__

#include "cocos2d.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace ecs {

/**
 * @brief 蚁狮移动组件
 * 
 * 蚁狮特性：
 * - 静止不动，头部伸出地面
 * - 头部会朝向45度角范围内的玩家
 * - 每5秒向玩家发射沙球射弹
 * - 只能向上45度角范围内射击
 */
struct AntlionMovementComponent {
    // AI状态
    enum AIState {
        IDLE,           // 待机状态
        TRACKING,       // 追踪玩家（头部朝向）
        SHOOTING        // 发射射弹
    };

    // 基础属性
    AIState aiState = IDLE;
    float aiTimer = 0.0f;
    
    // 射击配置
    float shootInterval = 3.33f;        // 射击间隔（3.33秒）
    float shootCooldown = 0.0f;         // 射击冷却计时
    float lastShootTime = 0.0f;         // 上次射击时间
    
    // 检测和瞄准
    float detectionRange = 500.0f;      // 检测范围（增加到500以便更容易发现玩家）
    float shootAngle = 45.0f;           // 射击角度范围（左右各45度）
    cocos2d::Vec2 targetPosition;       // 目标位置
    bool hasTarget = false;             // 是否有目标
    
    // 头部朝向
    float headRotation = 0.0f;          // 头部旋转角度
    float rotationSpeed = 2.0f;         // 头部转向速度（弧度/秒）
    
    // 射弹属性
    float projectileSpeed = 400.0f;     // 射弹速度
    float projectileGravity = 980.0f;   // 射弹重力
    
    // 动画配置
    float idleAnimSpeed = 1.0f;         // 待机动画速度
    float shootAnimSpeed = 2.0f;        // 射击动画速度
    
    AntlionMovementComponent() {
        aiState = IDLE;
        aiTimer = 0.0f;
        shootCooldown = 0.0f;
        lastShootTime = 0.0f;
        targetPosition = cocos2d::Vec2::ZERO;
        hasTarget = false;
        headRotation = 0.0f;
    }
    
    /**
     * @brief 检查角度是否在射击范围内
     * @param angle 角度（弧度）
     * @return 是否在射击范围内
     */
    bool isAngleInShootRange(float angle) const {
        // 将角度转换为度数
        float angleDegrees = angle * 180.0f / M_PI;
        
        // 标准化角度到[0, 360]
        while (angleDegrees < 0) angleDegrees += 360.0f;
        while (angleDegrees >= 360.0f) angleDegrees -= 360.0f;
        
        // 进一步放宽射击范围：允许-30到210度（覆盖水平线上下）
        // 这样玩家在同水平线或稍低位置时也能射击
        // 标准化到合理范围：330-360度 和 0-210度
        return (angleDegrees >= 330.0f || angleDegrees <= 210.0f);
    }
    
    /**
     * @brief 计算射击角度
     * @param fromPos 蚁狮位置
     * @param toPos 目标位置
     * @return 射击角度（弧度）
     */
    float calculateShootAngle(const cocos2d::Vec2& fromPos, const cocos2d::Vec2& toPos) const {
        cocos2d::Vec2 direction = toPos - fromPos;
        return atan2(direction.y, direction.x);
    }
    
    /**
     * @brief 计算抛物线射击角度
     * @param fromPos 发射位置
     * @param toPos 目标位置
     * @param velocity 发射速度
     * @param gravity 重力加速度
     * @return 最佳射击角度（弧度），如果无解返回-1
     */
    float calculateBallisticAngle(const cocos2d::Vec2& fromPos, const cocos2d::Vec2& toPos, 
                                  float velocity, float gravity) const {
        cocos2d::Vec2 displacement = toPos - fromPos;
        float x = displacement.x;
        float y = displacement.y;
        
        // 抛物线公式求解
        float v2 = velocity * velocity;
        float discriminant = v2 * v2 - gravity * (gravity * x * x + 2 * y * v2);
        
        if (discriminant < 0) {
            // 无解，目标太远
            return -1.0f;
        }
        
        // 选择较小的角度（更直接的轨迹）
        float angle1 = atan((v2 - sqrt(discriminant)) / (gravity * x));
        float angle2 = atan((v2 + sqrt(discriminant)) / (gravity * x));
        
        // 选择在射击范围内的角度
        if (isAngleInShootRange(angle1)) return angle1;
        if (isAngleInShootRange(angle2)) return angle2;
        
        return -1.0f; // 都不在射击范围内
    }
};

} // namespace ecs

#endif // __ECS_COMPONENT_ANTLIONMOVEMENTCOMPONENT_H__
