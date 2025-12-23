#ifndef __ECS_COMPONENT_KINGSLIMECOMPONENT_H__
#define __ECS_COMPONENT_KINGSLIMECOMPONENT_H__

#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>

namespace ecs {

/**
 * @brief 史莱姆王Boss组件 - 管理Boss特有的复杂行为
 */
struct KingSlimeComponent
{
  // AI状态枚举（用于动画系统）
  enum AIState {
    IDLE,
    JUMPING,
    TELEPORTING_OUT,
    TELEPORTING_IN
  };
  AIState aiState = IDLE;
  
  // 基础状态
  bool isActive = true;
  float baseScale = 1.0f;
  float currentScale = 1.0f;
  float minScale = 0.4f;
  
  // 跳跃模式
  int jumpCount = 0;
  int smallJumpsPerCycle = 3;
  bool isDoingBigJump = false;
  float bigJumpMultiplier = 3.0f;
  
  // 从JSON读取的基础跳跃参数
  float baseHorizontalImpulse = 8000.0f;
  float baseVerticalImpulse = 26000.0f;
  
  // 传送系统
  float teleportTimer = 0.0f;
  float teleportInterval = 15.0f;
  float teleportRange = 300.0f;
  bool canTeleport = true;
  bool teleportOnlyOnGround = true;
  bool isTeleporting = false;
  
  // 史莱姆生成系统
  int totalSlimesToSpawn = 80;
  int slimesSpawned = 0;
  float lastHealthPercent = 1.0f;
  float spawnHealthInterval = 0.012f;
  std::vector<std::string> spawnTypes = {
    "GreenSlime", "BlueSlime", "YellowSlime", 
    "RedSlime", "PurpleSlime", "SpikedSlime"
  };
  int maxSpawnPerInterval = 2;
  
  // 跳跃冷却倍率（血量越低跳得越快）
  float jumpCooldownMultiplier = 1.0f;
  
  KingSlimeComponent() = default;
  
  /**
   * @brief 根据当前血量百分比连续更新缩放（线性插值）
   */
  void updateStage(float healthPercent) {
    currentScale = 0.4f + 0.6f * healthPercent;
    currentScale = std::max(0.4f, std::min(1.0f, currentScale));
  }
  
  /**
   * @brief 获取当前阶段的冷却倍率
   */
  float getJumpCooldownMultiplier(float healthPercent) const {
    return 0.3f + 0.7f * healthPercent;
  }
  
  /**
   * @brief 获取当前阶段的跳跃力度倍率
   */
  float getJumpForceMultiplier(float healthPercent) const {
    return 1.4f - 0.4f * healthPercent;
  }
  
  /**
   * @brief 检查是否需要生成史莱姆
   */
  bool shouldSpawnSlime(float currentHealthPercent) {
    if (slimesSpawned >= totalSlimesToSpawn) return false;
    
    float healthDrop = lastHealthPercent - currentHealthPercent;
    if (healthDrop >= spawnHealthInterval) {
      lastHealthPercent = currentHealthPercent;
      return true;
    }
    return false;
  }
  
  /**
   * @brief 获取随机史莱姆类型
   */
  std::string getRandomSlimeType() const {
    if (spawnTypes.empty()) return "GreenSlime";
    int index = rand() % spawnTypes.size();
    return spawnTypes[index];
  }
  
  /**
   * @brief 重置跳跃周期
   */
  void resetJumpCycle() {
    jumpCount = 0;
    isDoingBigJump = false;
  }
  
  /**
   * @brief 执行下一次跳跃
   * @return true表示小跳，false表示大跳
   */
  bool executeNextJump() {
    if (jumpCount < smallJumpsPerCycle) {
      jumpCount++;
      isDoingBigJump = false;
      return true;
    } else {
      jumpCount = 0;
      isDoingBigJump = true;
      return false;
    }
  }
};

} // namespace ecs

#endif // __ECS_COMPONENT_KINGSLIMECOMPONENT_H__
