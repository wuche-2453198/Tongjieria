#ifndef __PLAYER_CRAFTING_SYSTEM_H__
#define __PLAYER_CRAFTING_SYSTEM_H__

#include "cocos2d.h"
#include "entt/entt.hpp"
#include "components/player/PlayerComponents.h"
#include "systems/item/StationDetector.h"

/**
 * @file PlayerCraftingSystem.h
 * @brief 玩家合成系统
 *
 * 职责:
 * - 检测玩家附近的工作台
 * - 更新 StationDetector 的可用工作台列表
 * - 处理合成界面的打开/关闭
 * - 响应合成快捷键
 *
 * 优先级: 15
 */
class PlayerCraftingSystem {
public:
    /**
     * @brief 更新合成系统
     * @param registry EnTT注册表
     * @param dt 帧时间
     */
    static void update(entt::registry& registry, float dt);

    /**
     * @brief 设置工作台检测范围(像素)
     * @param range 检测范围
     */
    static void setDetectionRange(float range);

    /**
     * @brief 获取工作台检测范围
     */
    static float getDetectionRange();

    /**
     * @brief 打开合成界面
     */
    static void openCraftingUI();

    /**
     * @brief 关闭合成界面
     */
    static void closeCraftingUI();

    /**
     * @brief 切换合成界面显示状态
     */
    static void toggleCraftingUI();

    /**
     * @brief 检查合成界面是否打开
     */
    static bool isCraftingUIOpen();

private:
    PlayerCraftingSystem() = delete;

    /**
     * @brief 检测玩家附近的工作台
     * @param registry EnTT注册表
     * @param playerPos 玩家位置
     * @return 检测到的工作台类型列表
     */
    static std::vector<StationType> detectNearbyStations(entt::registry& registry, const cocos2d::Vec2& playerPos);

    /**
     * @brief 更新StationDetector的工作台列表
     * @param nearbyStations 附近的工作台列表
     */
    static void updateStationDetector(const std::vector<StationType>& nearbyStations);

    // 静态成员变量
    static float s_detectionRange;        // 工作台检测范围(默认200像素)
    static bool s_craftingUIOpen;         // 合成界面是否打开
    static std::vector<StationType> s_lastDetectedStations;  // 上一帧检测到的工作台
};

#endif // __PLAYER_CRAFTING_SYSTEM_H__
